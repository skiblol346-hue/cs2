#include "legit.h"

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include "imgui.h"
#include <Windows.h>

#define DEG2RAD(x) ((x) * 0.01745329251f)
#define RAD2DEG(x) ((x) * 57.2957795131f)

static int ImGuiKeyToVK(int imKey)
{
    if (imKey >= ImGuiKey_A && imKey <= ImGuiKey_Z)
        return 'A' + (imKey - ImGuiKey_A);
    if (imKey >= ImGuiKey_0 && imKey <= ImGuiKey_9)
        return '0' + (imKey - ImGuiKey_0);
    if (imKey >= ImGuiKey_F1 && imKey <= ImGuiKey_F12)
        return VK_F1 + (imKey - ImGuiKey_F1);
    switch (imKey)
    {
    case ImGuiKey_MouseLeft:   return VK_LBUTTON;
    case ImGuiKey_MouseRight:  return VK_RBUTTON;
    case ImGuiKey_MouseMiddle: return VK_MBUTTON;
    case ImGuiKey_MouseX1:     return VK_XBUTTON1;
    case ImGuiKey_MouseX2:     return VK_XBUTTON2;
    case ImGuiKey_LeftShift:   return VK_LSHIFT;
    case ImGuiKey_RightShift:  return VK_RSHIFT;
    case ImGuiKey_LeftAlt:     return VK_LMENU;
    case ImGuiKey_RightAlt:    return VK_RMENU;
    case ImGuiKey_LeftCtrl:    return VK_LCONTROL;
    case ImGuiKey_RightCtrl:   return VK_RCONTROL;
    case ImGuiKey_Space:       return VK_SPACE;
    case ImGuiKey_Tab:         return VK_TAB;
    case ImGuiKey_CapsLock:    return VK_CAPITAL;
    case ImGuiKey_Delete:      return VK_DELETE;
    default:                   return 0;
    }
}

static bool IsKeyHeld(int imKey)
{
    int vk = ImGuiKeyToVK(imKey);
    if (vk == 0) return false;
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

static bool IsKeyJustPressed(int imKey)
{
    int vk = ImGuiKeyToVK(imKey);
    if (vk == 0) return false;
    static bool prevState[256] = {};
    bool down = (GetAsyncKeyState(vk) & 0x8000) != 0;
    bool pressed = down && !prevState[vk];
    prevState[vk] = down;
    return pressed;
}

#include <random>
#include <thread>
#include <chrono>
#include "../../features/ragebot/Rage.h"
#include "../../sdk/entity.h"
#include "../../sdk/interfaces/cgameentitysystem.h"

#include "../../sdk/interfaces/cgametracemanager.h"

#include "../../sdk/datatypes/usercmd.h"
#include <sstream>
#include "../../core/interfaces.h"
#include "../../utilities/inputsystem.h"
#include "../../sdk/interfaces/ccsgoinput.h"

#include "../../core/variables.h"
#include "../../core/sdk.h"
#include "../../gui/pages/legit/legit.hpp"

#include <numbers>

#include <cstring>
inline static unsigned int(__fastcall* fnGetHashSeed)(void* a1, QAngle_t* angle, std::uint32_t nPlayerTickCount);
inline static void(__fastcall* fnCalculateSpreadAngle)(int16_t, int, int, std::uint32_t, float, float, float, float*, float*);


void F::LEGITBOT::LEGIT::InitSpreadFunctions()
{

    fnGetHashSeed = (unsigned int(__fastcall*)(void*, QAngle_t*, std::uint32_t))MEM::FindPattern(CLIENT_DLL, "48 89 5C 24 ? 57 48 81 EC ? ? ? ? F3 0F 10 0A");


    fnCalculateSpreadAngle = (void(__fastcall*)(int16_t, int, int, std::uint32_t, float, float, float, float*, float*))MEM::FindPattern(CLIENT_DLL, "48 8B C4 48 89 58 08 48 89 68 18 48 89 70 20 57 41 54 41 55 41 56 41 57 48 81");
}


inline Vector_t GetBonePos(C_CSPlayerPawn* pawn, BoneID boneID)
{
    if (!pawn)
        return {};

    auto skeleton = reinterpret_cast<CSkeletonInstance*>(pawn->GetGameSceneNode());
    if (!skeleton)
        return {};


    auto modelState = &skeleton->GetModelState();
    if (!modelState)
        return {};


    uintptr_t boneArray = *reinterpret_cast<uintptr_t*>(
        reinterpret_cast<uintptr_t>(modelState) + 0x80
        );

    if (!boneArray)
        return {};

    return *reinterpret_cast<Vector_t*>(boneArray + static_cast<int>(boneID) * 0x20);
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
bool SegmentIntersectsCapsule(
    const Vector_t& start,
    const Vector_t& end,
    const Vector_t& capsuleStart,
    const Vector_t& capsuleEnd,
    float radius)
{
    Vector_t segDir = end - start;
    Vector_t capDir = capsuleEnd - capsuleStart;

    float segLen = std::sqrt(segDir.DotProduct(segDir));
    float capLen = std::sqrt(capDir.DotProduct(capDir));

    if (segLen < 0.001f)
    {
        // segment = point
        Vector_t closest = start;

        Vector_t capToPoint = closest - capsuleStart;
        float proj = capToPoint.DotProduct(capDir);

        if (capLen > 0.001f)
        {
            proj /= capLen;
            proj = std::clamp(proj, 0.0f, capLen);
        }
        else
        {
            proj = 0.0f;
        }

        Vector_t capPoint = capsuleStart + (capDir / (capLen + 0.0001f)) * proj;

        return capPoint.DistToSqr(closest) <= radius * radius;
    }

    segDir /= segLen;

    Vector_t capDirNorm = capDir;
    if (capLen > 0.001f)
        capDirNorm /= capLen;

    // closest points between segment and capsule axis (approximated)
    Vector_t w = start - capsuleStart;

    float a = segDir.DotProduct(segDir); // =1
    float b = segDir.DotProduct(capDirNorm);
    float c = capDirNorm.DotProduct(capDirNorm);
    float d = segDir.DotProduct(w);
    float e = capDirNorm.DotProduct(w);

    float D = a * c - b * b;

    float sc = 0.0f;
    float tc = 0.0f;

    if (D < 0.0001f)
    {
        sc = 0.0f;
        tc = (capLen > 0.001f) ? e : 0.0f;
    }
    else
    {
        sc = (b * e - c * d) / D;
        tc = (a * e - b * d) / D;
    }

    sc = std::clamp(sc, 0.0f, segLen);
    tc = std::clamp(tc, 0.0f, capLen);

    Vector_t p1 = start + segDir * sc;
    Vector_t p2 = capsuleStart + capDirNorm * tc;

    return p1.DistToSqr(p2) <= radius * radius;
}
bool F::LEGITBOT::LEGIT::IsBulletHitChance(
    C_CSPlayerPawn* pLocalPawn,
    C_CSPlayerPawn* pTargetPawn,
    const Vector_t& vecTargetPos,
    CBaseUserCmdPB* pUserCmd,
    CCSPlayerController* pLocalController,
    float iHitChance)
{
    if (!pLocalPawn || !pTargetPawn || !pUserCmd || !pLocalController)
        return false;

    if (iHitChance <= 0.f)
        return true;

    auto pWeapon = pLocalPawn->GetActiveWeapon();
    if (!pWeapon)
        return false;

    auto pWeaponData = pWeapon->GetWeaponVData();
    if (!pWeaponData)
        return false;

    Vector_t vecEyePos = pLocalPawn->GetEyePosition();

    float flDistance =
        vecEyePos.DistTo(vecTargetPos);

    if (flDistance <= 0.0f)
        return false;

    if (flDistance > pWeaponData->GetRange())
        return false;

    float flInaccuracy = pWeapon->get_inaccuracy();
    float flSpread = pWeaponData->GetSpread();
    float flRecoilIndex = pWeapon->GetRecoilIndex();

    if (flInaccuracy <= 0.0001f &&
        flSpread <= 0.0001f)
        return true;

    QAngle_t angAim =
        pUserCmd->pViewAngles->angValue;

    Vector_t vForward, vRight, vUp;

    AngleVectors(
        angAim,
        &vForward,
        &vRight,
        &vUp);

    constexpr int iTotalSeeds = 256;

    int iNeededHits =
        static_cast<int>(
            (iTotalSeeds * iHitChance) / 100.f);

    if (iNeededHits <= 0)
        return true;

    int iHits = 0;

    int nWeaponID =
        pWeapon->GetAttributeManager()
        .GetItem()
        .GetItemDefinitionIndex();

    Vector_t capsuleStart = vecTargetPos;
    Vector_t capsuleEnd = vecTargetPos;

    capsuleEnd.z += 12.0f;

    constexpr float flCapsuleRadius = 5.0f;

    for (int iSeed = 1; iSeed <= iTotalSeeds; iSeed++)
    {
        float flSpreadX = 0.0f;
        float flSpreadY = 0.0f;

        fnCalculateSpreadAngle(
            nWeaponID,
            1,
            pLocalPawn->IsScoped(),
            iSeed,
            flInaccuracy,
            flSpread,
            flRecoilIndex,
            &flSpreadX,
            &flSpreadY);

        Vector_t vSpreadDir =
            vForward +
            (vRight * flSpreadX) +
            (vUp * flSpreadY);

        vSpreadDir.Normalize();

        Vector_t vEnd =
            vecEyePos +
            (vSpreadDir * flDistance);

        if (SegmentIntersectsCapsule(
            vecEyePos,
            vEnd,
            capsuleStart,
            capsuleEnd,
            flCapsuleRadius))
        {
            ++iHits;

            if (iHits >= iNeededHits)
                return true;
        }

        int iRemainingSeeds =
            iTotalSeeds - iSeed;

        if (iHits + iRemainingSeeds <
            iNeededHits)
        {
            return false;
        }
    }

    return iHits >= iNeededHits;
}
void BunnyHop(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, C_CSPlayerPawn* pLocalPawn)
{
    if (!C_GET(bool, Vars.bBhop)) return;


    constexpr std::uint64_t kJumpMask = IN_JUMP;
    const std::uint64_t cmdButtons = pCmd->nButtons.nValue;
    const std::uint64_t baseButtons = (pBaseCmd && pBaseCmd->pInButtonState) ? pBaseCmd->pInButtonState->nValue : 0ULL;
    if (((cmdButtons | baseButtons) & kJumpMask) == 0ULL)
        return;


    const int moveType = pLocalPawn->GetMoveType();
    if (moveType == MOVETYPE_NOCLIP || moveType == MOVETYPE_LADDER)
        return;

    const std::uint32_t flags = pLocalPawn->GetFlags();
    if ((flags & FL_ONGROUND) != 0U)
        return;

    pCmd->nButtons.nValue &= ~kJumpMask;
    pCmd->nButtons.nValueChanged &= ~kJumpMask;
    pCmd->nButtons.nValueScroll &= ~kJumpMask;

    if (pBaseCmd && pBaseCmd->pInButtonState)
    {
        pBaseCmd->pInButtonState->nValue &= ~kJumpMask;
        pBaseCmd->pInButtonState->nValueChanged &= ~kJumpMask;
        pBaseCmd->pInButtonState->nValueScroll &= ~kJumpMask;
        pBaseCmd->pInButtonState->SetBits(
            BUTTON_STATE_PB_BITS_BUTTONSTATE1 |
            BUTTON_STATE_PB_BITS_BUTTONSTATE2 |
            BUTTON_STATE_PB_BITS_BUTTONSTATE3
        );
    }

    if (pBaseCmd && pBaseCmd->subtickMovesField.pRep)
    {
        for (int i = 0; i < pBaseCmd->subtickMovesField.nCurrentSize; ++i)
        {
            CSubtickMoveStep* step = pBaseCmd->subtickMovesField.pRep->tElements[i];
            if (!step || (step->nButton & kJumpMask) == 0ULL)
                continue;

            step->nButton &= ~kJumpMask;
            step->bPressed = false;
            step->SetBits(MOVESTEP_BITS_BUTTON | MOVESTEP_BITS_PRESSED);
        }
    }

    if (pBaseCmd)
        pBaseCmd->SetBits(BASE_BITS_BUTTONPB);
}
void F::LEGITBOT::LEGIT::OnMove(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn)
{


    if (!pLocalController->IsPawnAlive())
        return;

   
       



    BunnyHop(pCmd, pBaseCmd, pLocalPawn);
   TriggerBot(pCmd, pBaseCmd, pLocalPawn, pLocalController);

    if (!C_GET(bool, Vars.bLegitAimbot) && C_GET(bool, Vars.bRagebot))
        F::RAGEBOT::OnMove(pCmd, pBaseCmd, pLocalController, pLocalPawn);

}







bool F::LEGITBOT::LEGIT::IsSpreadValid(
    C_CSPlayerPawn* pLocalPawn,
    C_CSPlayerPawn* pTargetPawn,
    const Vector_t& vecTargetPos,
    CBaseUserCmdPB* pUserCmd,
    CCSPlayerController* pLocalController)
{
    if (!pLocalPawn || !pTargetPawn || !pUserCmd || !pLocalController)
        return false;

    auto pWeapon = pLocalPawn->GetActiveWeapon();
    if (!pWeapon)
        return false;

    auto pWeaponData = pWeapon->GetWeaponVData();
    if (!pWeaponData)
        return false;

    Vector_t vecStart = pLocalPawn->GetEyePosition();
    QAngle_t angView = pUserCmd->pViewAngles->angValue;

    float flInaccuracy = pWeapon->get_inaccuracy();
    float flSpread = pWeaponData->GetSpread();
    float flRecoilIndex = pWeapon->GetRecoilIndex();

    int nWeaponID =
        pWeapon->GetAttributeManager()
        .GetItem()
        .GetItemDefinitionIndex();

    int nNumBullets = pWeaponData->GetNumBullets();

    uint32_t seed =
        fnGetHashSeed(
            nullptr,
            &angView,
            pLocalController->GetTickBase());

    float spread_x = 0.0f;
    float spread_y = 0.0f;

    fnCalculateSpreadAngle(
        nWeaponID,
        nNumBullets,
        1,
        seed + 1,
        flInaccuracy,
        flSpread,
        flRecoilIndex,
        &spread_x,
        &spread_y);

    Vector_t forward, right, up;
    AngleVectors(
        angView,
        &forward,
        &right,
        &up);

    Vector_t spreadDir =
        forward +
        (right * spread_x) +
        (up * spread_y);

    spreadDir.Normalize();

    float flDistance =
        vecStart.DistTo(vecTargetPos);

    if (flDistance <= 0.0f)
        return false;

    if (flDistance > pWeaponData->GetRange())
        return false;

    Vector_t spreadEnd =
        vecStart + (spreadDir * flDistance);

    // Виртуальная капсула вокруг точки
    Vector_t capsuleStart = vecTargetPos;
    Vector_t capsuleEnd = vecTargetPos;

    // Высота капсулы
    capsuleEnd.z += 12.0f;

    // Радиус капсулы
    constexpr float flCapsuleRadius = 5.0f;

    return SegmentIntersectsCapsule(
        vecStart,
        spreadEnd,
        capsuleStart,
        capsuleEnd,
        flCapsuleRadius);
}

static bool CanShoot(C_CSWeaponBase* pWeapon, CCSPlayerController* pLocal)
{
    return pLocal && pWeapon && pWeapon->GetClip1() > 0 &&
        pWeapon->GetNextPrimaryAttackTick() <= pLocal->GetTickBase();
}


void F::LEGITBOT::LEGIT::TriggerBot(
    CUserCmd* pCmd,
    CBaseUserCmdPB* pUserCmd,
    C_CSPlayerPawn* pLocalPawn,
    CCSPlayerController* pLocalController)
{
    if (!pLocalPawn || !pLocalController || !pUserCmd)
        return;

    if (!pLocalController->IsPawnAlive())
        return;

    // Получаем активное оружие
    auto pWeapon = pLocalPawn->GetActiveWeapon();
    if (!pWeapon || !CanShoot(pWeapon, pLocalController))
        return;

    const int nDefIndex = pWeapon->GetAttributeManager().GetItem().GetItemDefinitionIndex();
    const int w = GetLegitWeaponCategory(nDefIndex);
    if (w < 0 || !vars::ltTrigger[w])
        return;

    {
        int keyMode = vars::ltTriggerKeyMode[w];
        int triggerKey = vars::ltTriggerKey[w];

        bool bActive = false;
        if (vars::ltTriggerAlways[w] || keyMode == 2)
            bActive = true;
        else if (triggerKey != 0)
        {
            if (keyMode == 1)
            {
                bActive = IsKeyHeld(triggerKey);
            }
            else
            {
                static bool toggled[6] = {};
                if (IsKeyJustPressed(triggerKey))
                    toggled[w] = !toggled[w];
                bActive = toggled[w];
            }
        }

        if (!bActive)
            return;
    }

    const bool triggerAutoWall = vars::ltTriggerAutoWall[w];

    Vector_t vecEyePos = pLocalPawn->GetEyePosition();
    QAngle_t angView = pUserCmd->pViewAngles->angValue;
    Vector_t vForward, vRight, vUp;
    AngleVectors(angView, &vForward, &vRight, &vUp);
    Vector_t vecAimEnd = vecEyePos + (vForward * 10000.0f);

    const int triggerMinDmg = C_GET(int, Vars.nTriggerMinDamage);
    const int triggerHc     = C_GET(int, Vars.nTriggerHitchance);

    Target_t bestTarget = {};
    float bestDamage = 0.f;
    bool bTargetFound = false;

    for (int i = 1; i <= 64; i++)
    {
        auto pEntity = I::GameResourceService->pGameEntitySystem->Get(i);
        if (!pEntity) continue;

        CSchemaClassInfo* pClassInfo = nullptr;
        pEntity->GetSchemaClassInfo(&pClassInfo);
        if (!pClassInfo || FNV1A::Hash(pClassInfo->Name) != FNV1A::HashConst("CCSPlayerController"))
            continue;

        auto pPlayer = reinterpret_cast<CCSPlayerController*>(pEntity);
        if (!pPlayer || pPlayer == pLocalController) continue;

        auto pPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(pPlayer->GetPawnHandle());
        if (!pPawn || !pPlayer->IsPawnAlive() || !pLocalPawn->IsOtherEnemy(pPawn))
            continue;

        for (int hitbox = HITBOX_HEAD; hitbox <= HITBOX_PELVIS; hitbox++)
        {
            Vector_t hitPos = GetBonePos(pPawn, static_cast<BoneID>(hitbox));
            if (hitPos.IsZero())
                continue;

            int damage = 0;
            if (!F::RAGEBOT::CheckPoint(vecEyePos, hitPos, pLocalPawn, pPawn, damage, triggerAutoWall, 0))
                continue;

            if (triggerMinDmg > 0 && damage < triggerMinDmg)
                continue;

            if (!IsSpreadValid(pLocalPawn, pPawn, hitPos, pUserCmd, pLocalController))
                continue;

            if (triggerHc > 0 && !IsBulletHitChance(pLocalPawn, pPawn, hitPos, pUserCmd, pLocalController, static_cast<float>(triggerHc)))
                continue;

            if (damage > bestDamage)
            {
                bestDamage = static_cast<float>(damage);
                bestTarget.pPawn = pPawn;
                bestTarget.vecHitPos = hitPos;
                bestTarget.iDamage = damage;
                bTargetFound = true;
            }
        }
    }

    if (!bTargetFound)
        return;

    // Стреляем
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<> dist(1, 5);

    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
    std::this_thread::sleep_for(std::chrono::microseconds(dist(rng)));
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}