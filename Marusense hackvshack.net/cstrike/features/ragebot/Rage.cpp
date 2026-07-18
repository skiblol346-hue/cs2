#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include <random>
#include <thread>
#include <chrono>
#include <numbers>
#include <cstring>

#include "../../features/legitbot/legit.h"
#include "../../sdk/entity.h"
#include "../../sdk/interfaces/cgameentitysystem.h"
#include "../../sdk/interfaces/cgametracemanager.h"
#include "../../sdk/datatypes/usercmd.h"
#include "../../core/interfaces.h"
#include "../../utilities/inputsystem.h"
#include "../../sdk/interfaces/ccsgoinput.h"
#include "../../core/variables.h"
#include "../../core/sdk.h"
#include "../../gui/pages/rage/rage.hpp"
#include "Rage.h"

#define DEG2RAD(x) ((x) * 0.01745329251f)
#define RAD2DEG(x) ((x) * 57.2957795131f)


namespace F::RAGEBOT
{
    static C_CSPlayerPawn* g_pLockedTarget = nullptr;
    static int g_nActiveConfig = 0;

    inline Vector_t GetBonePos(C_CSPlayerPawn* pawn, BoneID boneID)
    {
        if (!pawn) return {};

        auto skeleton = reinterpret_cast<CSkeletonInstance*>(pawn->GetGameSceneNode());
        if (!skeleton) return {};

        auto modelState = &skeleton->GetModelState();
        if (!modelState) return {};

        uintptr_t boneArray = *reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(modelState) + 0x80);
        if (!boneArray) return {};

        return *reinterpret_cast<Vector_t*>(boneArray + static_cast<int>(boneID) * 0x20);
    }

    static int ResolveWeaponConfig(C_CSPlayerPawn* pLocalPawn)
    {
        auto pWeapon = pLocalPawn->GetActiveWeapon();
        if (!pWeapon)
            return 0;

        int nDefIndex = pWeapon->GetAttributeManager().GetItem().GetItemDefinitionIndex();
        int specific = GetRageWeaponCategory(nDefIndex);

        if (specific != 0 && vars::rageEnabled[specific])
            return specific;
        return 0;
    }

     bool CheckPoint(const Vector_t& vecStart, const Vector_t& vecPoint,
        C_CSPlayerPawn* pLocal, C_CSPlayerPawn* pTarget, int& iDamage,
        bool bAutoWall, int iMinDamage)
    {

        alignas(16) CGameTrace trace = {};
        EngineTrace::TraceLine(vecStart, vecPoint, &trace, reinterpret_cast<uintptr_t>(pLocal), 0x1C300B);

        bool bVisible = trace.fraction > 0.97f;

        if (!bAutoWall)
        {
            if (!bVisible) return false;
            iDamage = 100;
            return true;
        }

        if (bVisible)
        {
            iDamage = 100;
            return true;
        }


        auto pWeapon = SDK::LocalPawn->GetActiveWeapon();
        if (!pWeapon)
            return false;

        auto pWeaponData = pWeapon->GetWeaponVData();
        if (!pWeaponData)
            return false;

        FireBulletData_t fireData{};
        fireData.pTargetPawn = pTarget;
        fireData.vecStartPos = vecStart;
        fireData.vecEndPos = vecPoint;
        fireData.iPenetrationCount = 4;

        if (EngineTrace::FireBullet(fireData, reinterpret_cast<uintptr_t>(pLocal), pWeapon, pWeaponData))
        {
            iDamage = static_cast<int>(fireData.flDamage);
            return iDamage >= iMinDamage;
        }

        return false;
    }
    inline QAngle_t VectorAngles(const Vector_t& vecDirection)
    {
        QAngle_t angAngles;

        if (vecDirection.x == 0.0f && vecDirection.y == 0.0f)
        {
            angAngles.x = (vecDirection.z > 0.0f) ? 270.0f : 90.0f;
            angAngles.y = 0.0f;
        }
        else
        {
            angAngles.y = atan2f(vecDirection.y, vecDirection.x) * (180.0f / M_PI);
            if (angAngles.y < 0.0f)
                angAngles.y += 360.0f;

            float flHypot = sqrtf(vecDirection.x * vecDirection.x + vecDirection.y * vecDirection.y);
            angAngles.x = atan2f(-vecDirection.z, flHypot) * (180.0f / M_PI);
            if (angAngles.x < 0.0f)
                angAngles.x += 360.0f;
        }

        angAngles.z = 0.0f;

        return angAngles;
    }

    inline void AngleVectors(const QAngle_t& angles, Vector_t* forward, Vector_t* right = nullptr, Vector_t* up = nullptr)
    {
        float sp = sinf(angles.x * 0.017453292f);
        float cp = cosf(angles.x * 0.017453292f);

        float sy = sinf(angles.y * 0.017453292f);
        float cy = cosf(angles.y * 0.017453292f);

        float sr = sinf(angles.z * 0.017453292f);
        float cr = cosf(angles.z * 0.017453292f);

        if (forward)
        {
            forward->x = cp * cy;
            forward->y = cp * sy;
            forward->z = -sp;
        }

        if (right)
        {
            right->x = -1.f * sr * sp * cy + -1.f * cr * -sy;
            right->y = -1.f * sr * sp * sy + -1.f * cr * cy;
            right->z = -1.f * sr * cp;
        }

        if (up)
        {
            up->x = cr * sp * cy + -sr * -sy;
            up->y = cr * sp * sy + -sr * cy;
            up->z = cr * cp;
        }
    }

    static void FindBestTarget(CCSPlayerController* pLocalController,
        C_CSPlayerPawn* pLocalPawn, CBaseUserCmdPB* pUserCmd)
    {
        Vector_t vecEye = pLocalPawn->GetEyePosition();
        Internal::g_target = {};
        Target_t bestTarget = {};
        float flBestFov = static_cast<float>(vars::rageMaxFov[g_nActiveConfig]);
        int iMinDamage = vars::rageMindmg[g_nActiveConfig];

        for (int i = 1; i <= 126; i++)
        {
            auto* pEntity = I::GameResourceService->pGameEntitySystem->Get(i);
            if (!pEntity) continue;

            CSchemaClassInfo* pClassInfo = nullptr;
            pEntity->GetSchemaClassInfo(&pClassInfo);
            if (!pClassInfo || FNV1A::Hash(pClassInfo->Name) != FNV1A::HashConst("CCSPlayerController"))
                continue;

            auto* pPlayer = reinterpret_cast<CCSPlayerController*>(pEntity);
            if (!pPlayer || pPlayer == pLocalController) continue;

            auto* pPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(pPlayer->GetPawnHandle());
            if (!pPawn || !pPlayer->IsPawnAlive() || !pLocalPawn->IsOtherEnemy(pPawn))
                continue;


            if (g_pLockedTarget && g_pLockedTarget != pPawn)
            {
                Vector_t vecLockedPos = GetBonePos(g_pLockedTarget, BoneID::Head);
                if (!vecLockedPos.IsZero())
                {
                    QAngle_t angToLocked = (vecLockedPos - vecEye).ToAngles();
                    angToLocked.Normalize();
                    QAngle_t angView = pUserCmd->pViewAngles->angValue;
                    float flDeltaYaw = std::abs(angToLocked.y - angView.y);
                    if (flDeltaYaw > 180.f) flDeltaYaw = 360.f - flDeltaYaw;
                    if (flDeltaYaw < flBestFov)
                        continue;
                }
            }

            Vector_t hitPos = GetBonePos(pPawn, BoneID::Head);
            if (hitPos.IsZero()) continue;

            QAngle_t angToPoint = (hitPos - vecEye).ToAngles();
            angToPoint.Normalize();
            QAngle_t angView = pUserCmd->pViewAngles->angValue;

            float flDeltaYaw = std::abs(angToPoint.y - angView.y);
            if (flDeltaYaw > 180.f) flDeltaYaw = 360.f - flDeltaYaw;
            if (flDeltaYaw > flBestFov) continue;

            int iDamage = 0;
            if (!CheckPoint(vecEye, hitPos, pLocalPawn, pPawn, iDamage,
                vars::rageAutoWall[g_nActiveConfig], vars::rageMindmg[g_nActiveConfig])) continue;
            if (iDamage < iMinDamage) continue;

            if (flDeltaYaw < flBestFov)
            {
                flBestFov = flDeltaYaw;
                bestTarget.pController = pPlayer;
                bestTarget.pPawn = pPawn;
                bestTarget.vecHitPos = hitPos;
                bestTarget.iDamage = iDamage;
                bestTarget.flFov = flDeltaYaw;
                bestTarget.iHitbox = HITBOX_HEAD;
            }
        }

        if (bestTarget.pPawn)
        {
            Internal::g_target = bestTarget;
            g_pLockedTarget = bestTarget.pPawn;
        }
        else
        {
            g_pLockedTarget = nullptr;
        }
    }

    static bool CanShoot(C_CSWeaponBase* pWeapon, CCSPlayerController* pLocal)
    {
        return pLocal && pWeapon && pWeapon->GetClip1() > 0 &&
            pWeapon->GetNextPrimaryAttackTick() <= pLocal->GetTickBase();
    }


    static void AutoShoot(CBaseUserCmdPB* pUserCmd, C_CSPlayerPawn* pLocalPawn,
        CCSPlayerController* pLocalController)
    {
        if (!Internal::g_target.pPawn) return;

        auto pWeapon = pLocalPawn->GetActiveWeapon();
        if (!pWeapon || !CanShoot(pWeapon, pLocalController)) return;

        Vector_t vecEye = pLocalPawn->GetEyePosition();
        QAngle_t angAim = (Internal::g_target.vecHitPos - vecEye).ToAngles();
        angAim.Normalize();

        I::Input->SetViewAngle(Vector_t(angAim.x, angAim.y, angAim.z));
        pUserCmd->pViewAngles->angValue = angAim;
        pUserCmd->SetBits(BASE_BITS_VIEWANGLES);

        bool bSpreadValid = false;
        switch (C_GET(int, Vars.nRagebotSpreadMode))
        {
        case RAGE_SPREAD_SEED:
            bSpreadValid = F::LEGITBOT::LEGIT::IsSpreadValid(pLocalPawn, Internal::g_target.pPawn,
                Internal::g_target.vecHitPos, pUserCmd, pLocalController);
            break;
        case RAGE_SPREAD_HITCHANCE:
            bSpreadValid = F::LEGITBOT::LEGIT::IsBulletHitChance(pLocalPawn, Internal::g_target.pPawn,
                Internal::g_target.vecHitPos, pUserCmd, pLocalController,
                static_cast<float>(vars::rageHitchance[g_nActiveConfig]));
            break;
        }

        if (!bSpreadValid) return;


        static std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<> dist(1, 5);


        auto currentTime = std::chrono::steady_clock::now();
        static auto lastTickTime = currentTime;
        std::random_device RandomDevice;
        std::mt19937 RandomNumber(RandomDevice());
        std::uniform_int_distribution<> Range(1, 5);
        auto rand = std::chrono::microseconds(Range(RandomNumber));

        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
        std::this_thread::sleep_for(std::chrono::microseconds(Range(RandomNumber)));
        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

    }

    void Init()
    {
        Internal::g_target = {};
        Internal::g_bActive = false;
        g_pLockedTarget = nullptr;
        g_nActiveConfig = 0;
    }

    void Reset()
    {
        Internal::g_target = {};
        Internal::g_bActive = false;
        g_pLockedTarget = nullptr;
    }


    void OnMove(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd,
        CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn)
    {
        if (!pLocalController || !pLocalPawn || !pBaseCmd || !pLocalController->IsPawnAlive())
        {
            Reset();
            return;
        }

        g_nActiveConfig = ResolveWeaponConfig(pLocalPawn);

        if (!vars::rageEnabled[g_nActiveConfig])
        {
            Reset();
            return;
        }

        FindBestTarget(pLocalController, pLocalPawn, pBaseCmd);

        if (!Internal::g_target.pPawn)
        {
            Reset();
            return;
        }

        auto pWeapon = pLocalPawn->GetActiveWeapon();
        if (!pWeapon) return;


        if (vars::rageAutofire)
            AutoShoot(pBaseCmd, pLocalPawn, pLocalController);
    }
}