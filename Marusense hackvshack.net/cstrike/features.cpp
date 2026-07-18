#include "features.h"

#include <chrono>
#include <functional>
#include <sstream>
#include <random>
#include "core/variables.h"

#include "utilities/draw.h"

#pragma comment(lib, "winmm.lib")

#include "core/menu.h"
#include "gui/gui.hpp"
#include "gui/framework/shell/dx_blur.h"
#include "gui/framework/styling.hpp"
#include "gui/framework/shell/shell.hpp"
#include "gui/assets/fonts.hpp"


#include "features/visuals.h"

#include "features/legitbot/legitbot.h"


#include "core/interfaces.h"
#include "sdk/interfaces/iswapchaindx11.h"
#include "sdk/interfaces/ienginecvar.h"

#include "sdk/interfaces/cgameentitysystem.h"
#include "sdk/datatypes/usercmd.h"
#include "sdk/entity.h"
#include "sdk/interfaces/iglobalvars.h"
#include "core/sdk.h"
#include "utilities/math.h"
#define DEG2RAD(x) ((x) * 0.01745329251f)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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

bool F::Setup()
{
    if (!VISUALS::Setup())
    {
        L_PRINT(LOG_ERROR) << CS_XOR("failed to setup visuals");
        return false;
    }

    return true;
}

template <typename T>
T Clamp(T v, T min, T max)
{
    if (v < min) return min;
    if (v > max) return max;
    return v;
}

void F::Destroy()
{
    VISUALS::OnDestroy();
}

struct HitMarkerData {
    float alpha = 0.f;
    float time = 0.f;
};
static HitMarkerData g_hitMarker;
static int g_prevShotsFired = 0;

static struct {
    int targetIndex = -1;
    float acquireTime = 0.f;
    float initialDist = 0.f;
    float lastKillTime = -99.f;
    int prevTargetHealth = 0;
    std::mt19937 rng{ std::random_device{}() };
} s_legit;

static void LegitAimbot(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn)
{
    if (!C_GET(bool, Vars.bLegitAimbot))
        return;

    if (C_GET(bool, Vars.bLegitFlashCheck) && pLocalPawn->GetFlashDuration() > 0.25f)
        return;

    int keyMode = C_GET(int, Vars.nLegitAimbotKeyMode);
    unsigned int aimKey = C_GET(unsigned int, Vars.nLegitAimbotKey);

    bool bActive = false;
    if (keyMode == 2)
        bActive = true;
    else if (aimKey != 0)
    {
        if (keyMode == 1)
            bActive = IsKeyHeld(static_cast<int>(aimKey));
        else
        {
            static bool toggled = false;
            if (IsKeyJustPressed(static_cast<int>(aimKey)))
                toggled = !toggled;
            bActive = toggled;
        }
    }

    if (!bActive)
        return;

    C_CSWeaponBase* pWeapon = pLocalPawn->GetActiveWeapon();
    if (!pWeapon)
        return;

    CCSWeaponBaseVData* pWeaponData = pWeapon->GetWeaponVData();
    if (!pWeaponData)
        return;

    if (pWeaponData->IsMeleeWeapon())
        return;

    if (C_GET(bool, Vars.bLegitScopeOnly))
    {
        int weaponType = pWeaponData->GetWeaponType();
        if (weaponType == 6 && !pLocalPawn->IsScoped())
            return;
    }

    float now = I::GlobalVars ? I::GlobalVars->flCurrentTime : 0.f;

    int killDelayMs = C_GET(int, Vars.nLegitKillDelay);
    if (killDelayMs > 0 && (now - s_legit.lastKillTime) < killDelayMs / 1000.f)
        return;

    static int shotDelayClock = 0;
    int shotDelayMs = C_GET(int, Vars.nLegitShotDelay);
    if (shotDelayMs > 0 && pLocalPawn->GetShotsFired() == 0)
    {
        if (shotDelayClock > 0)
        {
            shotDelayClock--;
            return;
        }
    }

    float flFov = C_GET(float, Vars.flLegitFov);
    float flSmooth = C_GET(float, Vars.flLegitSmooth);
    float flJitter = C_GET(float, Vars.flLegitJitter);
    float flDeadzone = C_GET(float, Vars.flLegitDeadzone);
    int nHitbox = C_GET(int, Vars.nLegitHitbox);

    Vector_t vecEye = pLocalPawn->GetEyePosition();
    QAngle_t angView = pBaseCmd->pViewAngles ? pBaseCmd->pViewAngles->angValue : QAngle_t{};

    float bestFov = flFov;
    Vector_t bestTarget = {};
    int bestIndex = -1;
    int bestHealth = 0;

    const int nHighestIndex = I::GameResourceService->pGameEntitySystem->GetHighestEntityIndex();
    for (int i = 1; i <= nHighestIndex; i++)
    {
        C_BaseEntity* pEntity = I::GameResourceService->pGameEntitySystem->Get(i);
        if (!pEntity)
            continue;

        CSchemaClassInfo* pClassInfo = nullptr;
        pEntity->GetSchemaClassInfo(&pClassInfo);
        if (!pClassInfo)
            continue;

        if (FNV1A::Hash(pClassInfo->Name) != FNV1A::HashConst("CCSPlayerController"))
            continue;

        CCSPlayerController* pController = reinterpret_cast<CCSPlayerController*>(pEntity);
        if (!pController || !pController->IsPawnAlive())
            continue;

        C_CSPlayerPawn* pPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(pController->GetPlayerPawnHandle());
        if (!pPawn)
            continue;

        if (!pLocalPawn->IsOtherEnemy(pPawn))
            continue;

        int hp = pPawn->GetHealth();
        if (hp <= 0)
            continue;

        auto skeleton = reinterpret_cast<CSkeletonInstance*>(pPawn->GetGameSceneNode());
        if (!skeleton)
            continue;

        auto modelState = &skeleton->GetModelState();
        if (!modelState)
            continue;

        uintptr_t boneArray = *reinterpret_cast<uintptr_t*>(
            reinterpret_cast<uintptr_t>(modelState) + 0x80);
        if (!boneArray)
            continue;

        int boneIdx = 7;
        switch (nHitbox)
        {
        case 0: boneIdx = 7; break;
        case 1: boneIdx = 6; break;
        case 2: boneIdx = 4; break;
        case 3:
        {
            float nearestDist = FLT_MAX;
            int nearestBone = 7;
            int bonesToCheck[] = { 7, 6, 4, 1, 0 };
            for (int b : bonesToCheck)
            {
                Vector_t bonePos = *reinterpret_cast<Vector_t*>(boneArray + b * 0x20);
                if (bonePos.IsZero())
                    continue;
                Vector_t d = bonePos - vecEye;
                float dist = d.Length();
                if (dist < nearestDist)
                {
                    nearestDist = dist;
                    nearestBone = b;
                }
            }
            boneIdx = nearestBone;
            break;
        }
        }

        Vector_t targetPos = *reinterpret_cast<Vector_t*>(boneArray + boneIdx * 0x20);
        if (targetPos.IsZero())
            continue;

        Vector_t delta = targetPos - vecEye;
        float len = delta.Length();
        if (len < 0.001f)
            continue;

        float pitch = -asinf(delta.z / len) * (180.f / (float)M_PI);
        float yaw = atan2f(delta.y, delta.x) * (180.f / (float)M_PI);

        float dPitch = pitch - angView.x;
        float dYaw = yaw - angView.y;
        while (dYaw > 180.f) dYaw -= 360.f;
        while (dYaw < -180.f) dYaw += 360.f;

        float fov = sqrtf(dPitch * dPitch + dYaw * dYaw);

        if (fov < bestFov)
        {
            bestFov = fov;
            bestTarget = targetPos;
            bestIndex = i;
            bestHealth = hp;
        }
    }

    if (bestIndex == -1)
    {
        if (s_legit.targetIndex != -1 && s_legit.prevTargetHealth > 0)
            s_legit.lastKillTime = now;

        s_legit.targetIndex = -1;
        s_legit.prevTargetHealth = 0;
        shotDelayClock = (int)(shotDelayMs / 15.6f);
        return;
    }
    shotDelayClock = 0;

    if (bestIndex != s_legit.targetIndex)
    {
        if (s_legit.targetIndex != -1 && s_legit.prevTargetHealth > 0)
            s_legit.lastKillTime = now;

        s_legit.targetIndex = bestIndex;
        s_legit.acquireTime = now;

        Vector_t d = bestTarget - vecEye;
        float len = d.Length();
        float p = -asinf(d.z / len) * (180.f / (float)M_PI);
        float y = atan2f(d.y, d.x) * (180.f / (float)M_PI);
        float dp = p - angView.x;
        float dy = y - angView.y;
        while (dy > 180.f) dy -= 360.f;
        while (dy < -180.f) dy += 360.f;
        s_legit.initialDist = sqrtf(dp * dp + dy * dy);
        if (s_legit.initialDist < 0.5f)
            s_legit.initialDist = 0.5f;
    }
    s_legit.prevTargetHealth = bestHealth;

    Vector_t delta = bestTarget - vecEye;
    float len = delta.Length();
    if (len < 0.001f) return;

    float aimPitch = -asinf(delta.z / len) * (180.f / (float)M_PI);
    float aimYaw = atan2f(delta.y, delta.x) * (180.f / (float)M_PI);

    if (C_GET(bool, Vars.bLegitRCS) && pLocalPawn->GetShotsFired() > 1)
    {
        float rcsAmount = C_GET(float, Vars.flLegitRCSAmount) / 100.f;
        QAngle_t punch = pLocalPawn->GetAimPunchAngle();
        aimPitch -= punch.x * 2.f * rcsAmount;
        aimYaw -= punch.y * 2.f * rcsAmount;
    }

    float dPitch = aimPitch - angView.x;
    float dYaw = aimYaw - angView.y;
    while (dYaw > 180.f) dYaw -= 360.f;
    while (dYaw < -180.f) dYaw += 360.f;

    float angDist = sqrtf(dPitch * dPitch + dYaw * dYaw);

    if (angDist < flDeadzone)
        return;

    bool bHumanized = C_GET(bool, Vars.bLegitHumanized);
    float factor;

    if (bHumanized)
    {
        std::uniform_real_distribution<float> dist01(-1.f, 1.f);

        float smoothRand = 1.f + dist01(s_legit.rng) * 0.15f;
        factor = 1.f / (flSmooth * smoothRand);

        float elapsed = now - s_legit.acquireTime;
        float ramp = Clamp(elapsed / 0.07f, 0.12f, 1.f);
        factor *= ramp;

        float decelZone = flFov * 0.35f;
        if (angDist < decelZone && decelZone > 0.01f)
        {
            float t = angDist / decelZone;
            float decel = 0.25f + 0.75f * t;
            factor *= decel;
        }
    }
    else
    {
        factor = 1.f / flSmooth;
    }

    float newPitch = angView.x + dPitch * factor;
    float newYaw = angView.y + dYaw * factor;

    if (bHumanized && flJitter > 0.f)
    {
        std::uniform_real_distribution<float> dist01(-1.f, 1.f);
        newPitch += dist01(s_legit.rng) * flJitter;
        newYaw += dist01(s_legit.rng) * flJitter;
    }

    if (newPitch > 89.f) newPitch = 89.f;
    if (newPitch < -89.f) newPitch = -89.f;
    while (newYaw > 180.f) newYaw -= 360.f;
    while (newYaw < -180.f) newYaw += 360.f;

    if (pBaseCmd->pViewAngles)
    {
        pBaseCmd->pViewAngles->angValue.x = newPitch;
        pBaseCmd->pViewAngles->angValue.y = newYaw;
    }
}

static void LastSecondDefuse(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn)
{
    if (!C_GET(bool, Vars.bLastSecondDefuse))
        return;

    if (!I::GlobalVars)
        return;

    float flServerTime = I::GlobalVars->flCurrentTime;

    const int nHighestIndex = I::GameResourceService->pGameEntitySystem->GetHighestEntityIndex();
    for (int i = 1; i <= nHighestIndex; i++)
    {
        C_BaseEntity* pEntity = I::GameResourceService->pGameEntitySystem->Get(i);
        if (!pEntity)
            continue;

        CSchemaClassInfo* pClassInfo = nullptr;
        pEntity->GetSchemaClassInfo(&pClassInfo);
        if (!pClassInfo)
            continue;

        if (FNV1A::Hash(pClassInfo->Name) != FNV1A::HashConst("C_PlantedC4"))
            continue;

        C_PlantedC4* pC4 = reinterpret_cast<C_PlantedC4*>(pEntity);
        if (!pC4 || !pC4->IsBombTicking() || pC4->IsBombDefused())
            continue;

        float flTimeRemaining = pC4->GetTimeRemaining(flServerTime);
        if (flTimeRemaining <= 0.f)
            continue;

        bool bHasKit = pLocalController->IsPawnHasDefuser();
        float flDefuseTime = bHasKit ? 5.f : 10.f;
        float flDefuseStart = flTimeRemaining - flDefuseTime;

        if (pLocalPawn->IsDefusing())
        {
            if (flTimeRemaining < flDefuseTime - 0.1f)
            {
                // too late, stop defusing
            }
        }
        else
        {
            if (flDefuseStart > 0.f && flDefuseStart < 0.5f)
            {
                // start defuse via +use
                if (pBaseCmd)
                {
                    auto* pButtons = pBaseCmd->pInButtonState;
                    if (pButtons)
                    {
                        pButtons->nValue |= (1ULL << 5); // IN_USE
                    }
                }
            }
        }
        break;
    }
}

static void Clantag()
{
    if (!C_GET(bool, Vars.bClantag))
        return;

    CConVar* nameVar = I::Cvar->Find(FNV1A::HashConst("name"));
    if (!nameVar) return;

    nameVar->nFlags &= ~FCVAR_PROTECTED;
    nameVar->nFlags |= FCVAR_USERINFO;

    static float flLastUpdate = 0.f;
    float flTime = I::GlobalVars ? I::GlobalVars->flCurrentTime : 0.f;
    if (flTime - flLastUpdate < 0.4f)
        return;
    flLastUpdate = flTime;

    static bool toggle = false;
    nameVar->value.sz = toggle ? "ct_a" : "ct_b";
    toggle = !toggle;
}

static void RadarHack()
{
    if (!C_GET(bool, Vars.bRadarHack))
        return;

    const int nHighestIndex = I::GameResourceService->pGameEntitySystem->GetHighestEntityIndex();
    for (int i = 1; i <= nHighestIndex; i++)
    {
        C_BaseEntity* pEntity = I::GameResourceService->pGameEntitySystem->Get(i);
        if (!pEntity)
            continue;

        CSchemaClassInfo* pClassInfo = nullptr;
        pEntity->GetSchemaClassInfo(&pClassInfo);
        if (!pClassInfo)
            continue;

        if (FNV1A::Hash(pClassInfo->Name) != FNV1A::HashConst("CCSPlayerController"))
            continue;

        CCSPlayerController* pController = reinterpret_cast<CCSPlayerController*>(pEntity);
        if (!pController || !pController->IsPawnAlive())
            continue;

        C_CSPlayerPawn* pPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(pController->GetPlayerPawnHandle());
        if (!pPawn || !SDK::LocalPawn || !SDK::LocalPawn->IsOtherEnemy(pPawn))
            continue;

        pPawn->IsSpottedOnRadar() = true;
    }
}

void F::OnCreateMove(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn)
{
    if (C_GET(bool, Vars.bAntiFlash))
    {
        pLocalPawn->GetFlashDuration() = 0.f;
    }

    LegitAimbot(pCmd, pBaseCmd, pLocalController, pLocalPawn);
    LastSecondDefuse(pCmd, pBaseCmd, pLocalController, pLocalPawn);
    RadarHack();

    int curShots = pLocalPawn->GetShotsFired();
    if (curShots > g_prevShotsFired && curShots > 0)
    {
        if (C_GET(bool, Vars.bDamageIndicator) || C_GET(bool, Vars.bHitMarker))
        {
            Vector_t vecEye = pLocalPawn->GetEyePosition();
            QAngle_t viewAng = pBaseCmd->pViewAngles->angValue;

            int dmg = 0;
            C_CSWeaponBase* pWpn = pLocalPawn->GetActiveWeapon();
            if (pWpn)
            {
                CCSWeaponBaseVData* pData = pWpn->GetWeaponVData();
                if (pData) dmg = pData->GetDamage();
            }

            const int nHighest = I::GameResourceService->pGameEntitySystem->GetHighestEntityIndex();
            for (int i = 1; i <= nHighest; i++)
            {
                C_BaseEntity* pEnt = I::GameResourceService->pGameEntitySystem->Get(i);
                if (!pEnt) continue;

                CSchemaClassInfo* pCI = nullptr;
                pEnt->GetSchemaClassInfo(&pCI);
                if (!pCI || FNV1A::Hash(pCI->Name) != FNV1A::HashConst("CCSPlayerController"))
                    continue;

                CCSPlayerController* pCtrl = reinterpret_cast<CCSPlayerController*>(pEnt);
                if (!pCtrl || !pCtrl->IsPawnAlive()) continue;

                C_CSPlayerPawn* pPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(pCtrl->GetPlayerPawnHandle());
                if (!pPawn || !pLocalPawn->IsOtherEnemy(pPawn)) continue;

                auto skeleton = reinterpret_cast<CSkeletonInstance*>(pPawn->GetGameSceneNode());
                if (!skeleton) continue;
                auto modelState = &skeleton->GetModelState();
                if (!modelState) continue;
                uintptr_t boneArray = *reinterpret_cast<uintptr_t*>(
                    reinterpret_cast<uintptr_t>(modelState) + 0x80);
                if (!boneArray) continue;

                int bonesToCheck[] = { 7, 6, 4, 1, 0 };
                for (int b : bonesToCheck)
                {
                    Vector_t bonePos = *reinterpret_cast<Vector_t*>(boneArray + b * 0x20);
                    if (bonePos.IsZero()) continue;

                    Vector_t delta = bonePos - vecEye;
                    float len = delta.Length();
                    if (len < 0.001f) continue;

                    float pitch = -asinf(delta.z / len) * (180.f / (float)M_PI);
                    float yaw = atan2f(delta.y, delta.x) * (180.f / (float)M_PI);

                    float dP = pitch - viewAng.x;
                    float dY = yaw - viewAng.y;
                    while (dY > 180.f) dY -= 360.f;
                    while (dY < -180.f) dY += 360.f;

                    float fov = sqrtf(dP * dP + dY * dY);
                    if (fov < 2.5f)
                    {
                        g_hitMarker.alpha = 1.f;
                        if (C_GET(bool, Vars.bDamageIndicator))
                            F::OnDamageDealt(bonePos, dmg);
                        goto done_hit_check;
                    }
                }
            }
            done_hit_check:;
        }
    }
    g_prevShotsFired = curShots;
}

struct WidgetDrag_t { bool bDragging = false; ImVec2 offset = {}; };

static bool UpdateWidgetDrag(WidgetDrag_t& drag, float& rx, float& ry, float w, float titleH)
{
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 m = io.MousePos;
    bool hit = m.x >= rx && m.x <= rx + w && m.y >= ry && m.y <= ry + titleH;
    if (hit && io.MouseClicked[0] && !drag.bDragging)
    {
        drag.bDragging = true;
        drag.offset = ImVec2(m.x - rx, m.y - ry);
    }
    if (!io.MouseDown[0]) drag.bDragging = false;
    if (drag.bDragging)
    {
        rx = ImClamp(m.x - drag.offset.x, 0.f, io.DisplaySize.x - w);
        ry = ImClamp(m.y - drag.offset.y, 0.f, io.DisplaySize.y - 30.f);
    }
    return drag.bDragging;
}

static constexpr ImU32 kWidgetBg     = IM_COL32(18, 18, 22, 51);
static constexpr ImU32 kWidgetPanel  = IM_COL32(14, 14, 18, 51);
static constexpr ImU32 kWidgetBorder = IM_COL32(12, 12, 12, 255);
static constexpr ImU32 kWidgetText   = IM_COL32(205, 205, 205, 255);
static constexpr ImU32 kWidgetDim    = IM_COL32(128, 128, 128, 255);
static constexpr ImU32 kWidgetAccent = IM_COL32(150, 232, 0, 255);
static constexpr ImU32 kWidgetBar    = IM_COL32(30, 30, 30, 56);

static void DrawWidgetBox(ImDrawList* dl, float x, float y, float w, float h, const char* title)
{
    ImVec2 mn(x, y), mx(x + w, y + h);
    ksd::BlurRegion(mn, mx);
    dl->AddRectFilled(mn, mx, kWidgetBg, 4.f);
    dl->AddRectFilled(mn, mx, kWidgetPanel, 4.f);
    dl->AddRect(mn, mx, kWidgetBorder, 4.f);

    dl->AddText(fonts::VerdanaBold(), fonts::kUiSize,
        ImVec2(x + 10.f, y + 5.f), kWidgetText, title);

    dl->AddLine(ImVec2(x + 1.f, y + 22.f), ImVec2(x + w - 1.f, y + 22.f), kWidgetBorder);
}

static C_PlantedC4* FindPlantedC4()
{
    const int nHighestIndex = I::GameResourceService->pGameEntitySystem->GetHighestEntityIndex();
    for (int i = 1; i <= nHighestIndex; i++)
    {
        C_BaseEntity* pEntity = I::GameResourceService->pGameEntitySystem->Get(i);
        if (!pEntity) continue;

        CSchemaClassInfo* pClassInfo = nullptr;
        pEntity->GetSchemaClassInfo(&pClassInfo);
        if (!pClassInfo) continue;

        if (FNV1A::Hash(pClassInfo->Name) == FNV1A::HashConst("C_PlantedC4"))
            return reinterpret_cast<C_PlantedC4*>(pEntity);
    }
    return nullptr;
}

static float CalcBombDamage(float flDistance, float flArmor)
{
    float flDamage = 500.f * (expf(-flDistance * flDistance / (2.f * 1750.f * 1750.f / 9.f)));

    if (flArmor > 0.f)
    {
        float flArmorRatio = 0.5f;
        float flArmorBonus = 0.5f;
        float flNew = flDamage * flArmorRatio;
        float flArmDmg = (flDamage - flNew) * flArmorBonus;

        if (flArmDmg > flArmor)
        {
            flArmDmg = flArmor;
            flNew = flDamage - flArmDmg * 2.f;
        }
        flDamage = flNew;
    }

    return flDamage;
}

static void DrawBombTimer()
{
    if (!C_GET(bool, Vars.bBombTimer) || !I::GlobalVars || !SDK::LocalPawn)
        return;

    C_PlantedC4* pC4 = FindPlantedC4();
    if (!pC4 || !pC4->IsBombTicking() || pC4->IsBombDefused())
        return;

    float flServerTime = I::GlobalVars->flCurrentTime;
    float flTimeRemaining = pC4->GetTimeRemaining(flServerTime);
    if (flTimeRemaining <= 0.f)
        return;

    ImDrawList* dl = ImGui::GetForegroundDrawList();
    ImGuiIO& io = ImGui::GetIO();

    static float wX = -1.f, wY = 40.f;
    static WidgetDrag_t wDrag;
    const float W = 200.f;
    const float titleH = 24.f;
    const float rowH = 14.f;
    const float padX = 10.f;
    const float fSz = fonts::kUiSize;

    int site = pC4->GetBombSite();
    const char* siteName = site == 0 ? "A" : "B";
    float totalTime = pC4->GetTimerLength();
    float frac = totalTime > 0.f ? Clamp(flTimeRemaining / totalTime, 0.f, 1.f) : 0.f;

    CGameSceneNode* pBombNode = pC4->GetGameSceneNode();
    Vector_t bombPos = pBombNode ? pBombNode->GetAbsOrigin() : Vector_t{};
    Vector_t localPos = SDK::LocalPawn->GetGameSceneNode()
        ? SDK::LocalPawn->GetGameSceneNode()->GetAbsOrigin() : Vector_t{};
    float dist = (bombPos - localPos).Length();
    float armor = SDK::LocalController ? (float)SDK::LocalController->GetPawnArmor() : 0.f;
    float dmg = CalcBombDamage(dist, armor);
    int iDmg = (int)dmg;
    int hp = SDK::LocalPawn->GetHealth();

    static char dmgBuf[16];
    const char* dmgLabel;
    ImU32 dmgCol;
    if (iDmg >= hp)
    {
        dmgLabel = "FATAL";
        dmgCol = IM_COL32(255, 40, 40, 255);
    }
    else if (iDmg >= hp - 10)
    {
        snprintf(dmgBuf, sizeof(dmgBuf), "-%dhp", iDmg);
        dmgLabel = dmgBuf;
        dmgCol = IM_COL32(255, 100, 40, 255);
    }
    else
    {
        snprintf(dmgBuf, sizeof(dmgBuf), "-%dhp", iDmg);
        dmgLabel = dmgBuf;
        dmgCol = kWidgetAccent;
    }

    int rows = 3;
    bool defusing = pC4->IsBeingDefused();
    if (defusing) rows++;
    float bodyH = rows * rowH + 20.f;
    float H = titleH + bodyH;

    if (wX < 0.f) wX = (io.DisplaySize.x - W) * 0.5f;
    UpdateWidgetDrag(wDrag, wX, wY, W, titleH);
    DrawWidgetBox(dl, wX, wY, W, H, "Bomb");

    float cy = wY + titleH + 4.f;

    {
        dl->AddText(fonts::Verdana(), fSz, ImVec2(wX + padX, cy), kWidgetDim, "Site");
        dl->AddText(fonts::VerdanaBold(), fSz, ImVec2(wX + padX + 48.f, cy), kWidgetAccent, siteName);
        cy += rowH;
    }

    {
        char tbuf[16]; snprintf(tbuf, sizeof(tbuf), "%.1fs", flTimeRemaining);
        int r = (int)(255.f * (1.f - frac));
        int g = (int)(255.f * frac);
        dl->AddText(fonts::Verdana(), fSz, ImVec2(wX + padX, cy), kWidgetDim, "Time");
        dl->AddText(fonts::Verdana(), fSz, ImVec2(wX + padX + 48.f, cy), IM_COL32(r, g, 0, 255), tbuf);
        cy += rowH;
    }

    float barY = cy;
    float barW = W - padX * 2.f;
    float barH = 4.f;
    dl->AddRectFilled(ImVec2(wX + padX, barY), ImVec2(wX + padX + barW, barY + barH),
        kWidgetBar, 2.f);
    {
        int r = (int)(255.f * (1.f - frac));
        int g = (int)(255.f * frac);
        dl->AddRectFilled(ImVec2(wX + padX, barY), ImVec2(wX + padX + barW * frac, barY + barH),
            IM_COL32(r, g, 0, 255), 2.f);
    }
    cy += barH + 4.f;

    {
        dl->AddText(fonts::Verdana(), fSz, ImVec2(wX + padX, cy), kWidgetDim, "Damage");
        dl->AddText(fonts::Verdana(), fSz, ImVec2(wX + padX + 48.f, cy), dmgCol, dmgLabel);
        cy += rowH;
    }

    if (defusing)
    {
        float defuseRemaining = pC4->GetDefuseTimeRemaining(flServerTime);
        bool safe = defuseRemaining < flTimeRemaining;
        ImU32 defCol = safe ? kWidgetAccent : IM_COL32(255, 50, 50, 255);

        char buf[32];
        snprintf(buf, sizeof(buf), "%.1fs %s", defuseRemaining, safe ? "(safe)" : "(fail)");
        dl->AddText(fonts::Verdana(), fSz, ImVec2(wX + padX, cy), kWidgetDim, "Defuse");
        dl->AddText(fonts::Verdana(), fSz, ImVec2(wX + padX + 48.f, cy), defCol, buf);
    }
}

static void DrawBombESP()
{
    if (!C_GET(bool, Vars.bBombESP) || !I::GlobalVars || !SDK::LocalPawn)
        return;

    C_PlantedC4* pC4 = FindPlantedC4();
    if (!pC4 || !pC4->IsBombTicking() || pC4->IsBombDefused())
        return;

    CGameSceneNode* pNode = pC4->GetGameSceneNode();
    if (!pNode) return;

    Vector_t bombPos = pNode->GetAbsOrigin();
    if (bombPos.IsZero()) return;

    ImVec2 screen;
    if (!D::WorldToScreen(bombPos, &screen))
        return;

    ImDrawList* dl = ImGui::GetForegroundDrawList();
    float flServerTime = I::GlobalVars->flCurrentTime;
    float flTimeRemaining = pC4->GetTimeRemaining(flServerTime);
    if (flTimeRemaining <= 0.f) return;

    int site = pC4->GetBombSite();
    float totalTime = pC4->GetTimerLength();
    float frac = totalTime > 0.f ? Clamp(flTimeRemaining / totalTime, 0.f, 1.f) : 0.f;

    char label[32];
    snprintf(label, sizeof(label), "C4 [%s] %.1fs", site == 0 ? "A" : "B", flTimeRemaining);
    const float fSz = fonts::kUiSize;
    ImVec2 labelSz = fonts::CalcUiTextSize(label);

    float cx = screen.x;
    float cy = screen.y;

    dl->AddCircleFilled(ImVec2(cx, cy), 5.f, IM_COL32(255, 60, 60, 220));
    dl->AddCircle(ImVec2(cx, cy), 5.f, kWidgetBorder, 0, 1.5f);

    float textY = cy - 20.f;
    float bgPad = 3.f;
    ImVec2 bgMin(cx - labelSz.x * 0.5f - bgPad, textY - bgPad);
    ImVec2 bgMax(cx + labelSz.x * 0.5f + bgPad, textY + labelSz.y + bgPad);
    ksd::BlurRegion(bgMin, bgMax);
    dl->AddRectFilled(bgMin, bgMax, kWidgetBg, 3.f);
    dl->AddRectFilled(bgMin, bgMax, kWidgetPanel, 3.f);
    dl->AddRect(bgMin, bgMax, kWidgetBorder, 3.f);
    dl->AddText(fonts::Verdana(), fSz, ImVec2(cx - labelSz.x * 0.5f, textY), kWidgetText, label);

    float barW = 40.f, barH = 3.f;
    float barX = cx - barW * 0.5f;
    float barBY = cy + 9.f;
    dl->AddRectFilled(ImVec2(barX, barBY), ImVec2(barX + barW, barBY + barH),
        kWidgetBar, 2.f);
    int r = (int)(255.f * (1.f - frac));
    int g = (int)(255.f * frac);
    dl->AddRectFilled(ImVec2(barX, barBY), ImVec2(barX + barW * frac, barBY + barH),
        IM_COL32(r, g, 0, 255), 2.f);
}

static void DrawSpectatorList()
{
    if (!C_GET(bool, Vars.bSpectatorList) || !SDK::LocalPawn)
        return;

    const char* names[10];
    int count = 0;

    const int nHighestIndex = I::GameResourceService->pGameEntitySystem->GetHighestEntityIndex();
    for (int i = 1; i <= nHighestIndex && count < 10; i++)
    {
        C_BaseEntity* pEntity = I::GameResourceService->pGameEntitySystem->Get(i);
        if (!pEntity) continue;

        CSchemaClassInfo* pClassInfo = nullptr;
        pEntity->GetSchemaClassInfo(&pClassInfo);
        if (!pClassInfo) continue;

        if (FNV1A::Hash(pClassInfo->Name) != FNV1A::HashConst("CCSPlayerController"))
            continue;

        CCSPlayerController* pController = reinterpret_cast<CCSPlayerController*>(pEntity);
        if (!pController || pController->IsLocalPlayerController() || pController->IsPawnAlive())
            continue;

        const char* name = pController->GetPlayerName();
        if (name && name[0])
            names[count++] = name;
    }

    if (count == 0)
        return;

    ImDrawList* dl = ImGui::GetForegroundDrawList();
    ImGuiIO& io = ImGui::GetIO();

    static float sX = -1.f, sY = 80.f;
    static WidgetDrag_t sDrag;
    const float W = 170.f;
    const float titleH = 24.f;
    const float rowH = 14.f;
    const float padX = 10.f;
    const float fSz = fonts::kUiSize;

    float bodyH = count * rowH + 6.f;
    float H = titleH + bodyH;

    if (sX < 0.f) sX = io.DisplaySize.x - W - 12.f;
    UpdateWidgetDrag(sDrag, sX, sY, W, titleH);
    DrawWidgetBox(dl, sX, sY, W, H, "Spectators");

    float cy = sY + titleH + 3.f;
    for (int i = 0; i < count; i++)
    {
        dl->AddText(fonts::Verdana(), fSz, ImVec2(sX + padX, cy), kWidgetText, names[i]);
        cy += rowH;
    }
}

struct DamageEntry_t
{
    Vector_t pos;
    int damage;
    float spawnTime;
};
static DamageEntry_t g_damageEntries[16];
static volatile int g_damageCount = 0;

void F::OnDamageDealt(const Vector_t& pos, int damage)
{
    if (!I::GlobalVars)
        return;

    float now = I::GlobalVars->flCurrentTime;

    int writeIdx = 0;
    int count = g_damageCount;
    if (count > 16) count = 16;
    for (int i = 0; i < count; i++)
    {
        float elapsed = now - g_damageEntries[i].spawnTime;
        if (elapsed >= 0.f && elapsed <= 1.5f)
            g_damageEntries[writeIdx++] = g_damageEntries[i];
    }

    if (writeIdx < 16)
        g_damageEntries[writeIdx++] = { pos, damage, now };

    g_damageCount = writeIdx;
}

static void DrawDamageIndicator()
{
    if (!C_GET(bool, Vars.bDamageIndicator) || !I::GlobalVars)
        return;

    ImDrawList* dl = ImGui::GetForegroundDrawList();
    if (!dl)
        return;

    float now = I::GlobalVars->flCurrentTime;
    int count = g_damageCount;
    if (count > 16) count = 16;

    for (int i = 0; i < count; i++)
    {
        DamageEntry_t e = g_damageEntries[i];
        float elapsed = now - e.spawnTime;
        if (elapsed > 1.5f || elapsed < 0.f)
            continue;

        Vector_t renderPos = e.pos;
        renderPos.z += elapsed * 40.f;

        ImVec2 screen;
        if (!D::WorldToScreen(renderPos, &screen))
            continue;

        float alpha = 1.f - (elapsed / 1.5f);
        int a = (int)(alpha * 255.f);
        if (a <= 0)
            continue;

        char buf[8];
        if (e.damage > 0) snprintf(buf, sizeof(buf), "-%d", e.damage);
        else snprintf(buf, sizeof(buf), "HIT");
        ImVec2 sz = fonts::CalcUiTextSize(buf);
        dl->AddText(fonts::VerdanaBold(), fonts::kUiSize,
            ImVec2(screen.x - sz.x * 0.5f + 1.f, screen.y + 1.f), IM_COL32(0, 0, 0, a), buf);
        dl->AddText(fonts::VerdanaBold(), fonts::kUiSize,
            ImVec2(screen.x - sz.x * 0.5f, screen.y), IM_COL32(255, 60, 60, a), buf);
    }
}

static void DrawHitMarker()
{
    if (!C_GET(bool, Vars.bHitMarker))
        return;

    if (g_hitMarker.alpha <= 0.f)
        return;

    ImDrawList* dl = ImGui::GetForegroundDrawList();
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);

    int a = (int)(g_hitMarker.alpha * 255.f);
    ImU32 col = IM_COL32(255, 255, 255, a);

    float sz = 8.f;
    float gap = 3.f;

    dl->AddLine(ImVec2(center.x - sz, center.y - sz), ImVec2(center.x - gap, center.y - gap), col, 2.f);
    dl->AddLine(ImVec2(center.x + sz, center.y - sz), ImVec2(center.x + gap, center.y - gap), col, 2.f);
    dl->AddLine(ImVec2(center.x - sz, center.y + sz), ImVec2(center.x - gap, center.y + gap), col, 2.f);
    dl->AddLine(ImVec2(center.x + sz, center.y + sz), ImVec2(center.x + gap, center.y + gap), col, 2.f);

    g_hitMarker.alpha -= io.DeltaTime * 3.f;
    if (g_hitMarker.alpha < 0.f) g_hitMarker.alpha = 0.f;
}

static void DrawDroppedWeapons()
{
    if (!C_GET(bool, Vars.bDroppedWeapons) || !SDK::LocalPawn)
        return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    Color_t col = C_GET(Color_t, Vars.colDroppedWeapons);
    ImU32 textCol = IM_COL32(col.r, col.g, col.b, col.a);

    const int nHighestIndex = I::GameResourceService->pGameEntitySystem->GetHighestEntityIndex();
    for (int i = 1; i <= nHighestIndex; i++)
    {
        C_BaseEntity* pEntity = I::GameResourceService->pGameEntitySystem->Get(i);
        if (!pEntity)
            continue;

        CSchemaClassInfo* pClassInfo = nullptr;
        pEntity->GetSchemaClassInfo(&pClassInfo);
        if (!pClassInfo)
            continue;

        const char* name = pClassInfo->Name;
        if (!name)
            continue;

        FNV1A_t hash = FNV1A::Hash(name);
        bool isWeapon = (hash == FNV1A::HashConst("C_AK47") ||
                         hash == FNV1A::HashConst("C_WeaponAWP") ||
                         hash == FNV1A::HashConst("C_WeaponM4A1") ||
                         hash == FNV1A::HashConst("C_WeaponM4A1Silencer") ||
                         hash == FNV1A::HashConst("C_DEagle") ||
                         hash == FNV1A::HashConst("C_WeaponSSG08") ||
                         hash == FNV1A::HashConst("C_WeaponSCAR20") ||
                         hash == FNV1A::HashConst("C_WeaponG3SG1") ||
                         hash == FNV1A::HashConst("C_WeaponAug") ||
                         hash == FNV1A::HashConst("C_WeaponP90"));

        if (!isWeapon)
            continue;

        CGameSceneNode* pNode = pEntity->GetGameSceneNode();
        if (!pNode)
            continue;

        Vector_t origin = pNode->GetAbsOrigin();
        if (origin.IsZero())
            continue;

        CBaseHandle ownerHandle = pEntity->GetOwnerHandle();
        if (ownerHandle.IsValid())
            continue;

        ImVec2 screen;
        if (!D::WorldToScreen(origin, &screen))
            continue;

        const char* displayName = name + 2; // skip "C_"
        if (CRT::StringLength(displayName) > 6)
            displayName += 6; // skip "Weapon"

        dl->AddText(ImVec2(screen.x - ImGui::CalcTextSize(displayName).x * 0.5f, screen.y),
            textCol, displayName);
    }
}

void F::OnPresent()
{
    if (!D::bInitialized)
        return;

    D::NewFrame();

    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* dl = ImGui::GetForegroundDrawList();

    dl->PushClipRectFullScreen();
    ImGui::PushFont(FONT::pMenu[0]);

    g_swapChain = I::SwapChain->pDXGISwapChain;
    device = I::Device;
    ctx = I::DeviceContext;

    gui::Render();
    F::VISUALS::OnRender();

    DrawBombTimer();
    DrawBombESP();
    DrawSpectatorList();
    DrawHitMarker();
    DrawDamageIndicator();
    DrawDroppedWeapons();

    auto KeyToString = [](unsigned int vk) -> const char*
        {
            static char buf[32];
            switch (vk)
            {
            case 0:           return "none";
            case VK_LMENU:    return "l-alt";
            case VK_RMENU:    return "r-alt";
            case VK_LSHIFT:   return "l-shift";
            case VK_RSHIFT:   return "r-shift";
            case VK_LCONTROL: return "l-ctrl";
            case VK_RCONTROL: return "r-ctrl";
            case VK_SPACE:    return "space";
            case VK_XBUTTON1: return "mouse4";
            case VK_XBUTTON2: return "mouse5";
            case VK_LBUTTON:  return "mouse1";
            case VK_RBUTTON:  return "mouse2";
            case VK_MBUTTON:  return "mouse3";
            case VK_INSERT:   return "insert";
            case VK_DELETE:   return "delete";
            case VK_HOME:     return "home";
            case VK_END:      return "end";
            case VK_F1:  return "f1";  case VK_F2:  return "f2";
            case VK_F3:  return "f3";  case VK_F4:  return "f4";
            case VK_F5:  return "f5";  case VK_F6:  return "f6";
            case VK_F7:  return "f7";  case VK_F8:  return "f8";
            case VK_F9:  return "f9";  case VK_F10: return "f10";
            case VK_F11: return "f11"; case VK_F12: return "f12";
            default:
                GetKeyNameTextA((MapVirtualKeyA(vk, MAPVK_VK_TO_VSC) << 16), buf, sizeof(buf));
                for (char* p = buf; *p; ++p) *p = (char)tolower((unsigned char)*p);
                return buf[0] ? buf : "?";
            }
        };


    auto Seg = [&](float x, float y, const char* t, ImU32 c) -> float
        {
            dl->AddText(ImVec2(x, y), c, t);
            return x + ImGui::CalcTextSize(t).x;
        };


    auto DrawPill = [&](float x, float y, float w, float h)
        {
            dl->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), IM_COL32(28, 28, 30, 235), 3.f);
            dl->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), IM_COL32(55, 55, 58, 255), 3.f, 0, 1.f);
        };


    struct DragState_t { bool bDragging = false; ImVec2 offset = {}; };

    auto UpdateDrag = [&](DragState_t& drag, float& rx, float& ry, float w, float h, float titleH) -> bool
        {
            ImVec2 m = io.MousePos;
            bool hit = m.x >= rx && m.x <= rx + w && m.y >= ry && m.y <= ry + titleH;
            if (hit && io.MouseClicked[0] && !drag.bDragging)
            {
                drag.bDragging = true;
                drag.offset = ImVec2(m.x - rx, m.y - ry);
            }
            if (!io.MouseDown[0]) drag.bDragging = false;
            if (drag.bDragging)
            {
                rx = ImClamp(m.x - drag.offset.x, 0.f, io.DisplaySize.x - w);
                ry = ImClamp(m.y - drag.offset.y, 0.f, io.DisplaySize.y - h);
            }
            return drag.bDragging;
        };

    {
        static float       wmX = -1.f, wmY = 10.f;
        static DragState_t wmDrag;
        static DWORD t0 = GetTickCount();
        DWORD  sec = (GetTickCount() - t0) / 1000;
        char   tbuf[16];
        snprintf(tbuf, sizeof(tbuf), "%02u:%02u:%02u", sec / 3600, (sec % 3600) / 60, sec % 60);

        const char* szName = CS_XOR("marusense");
        const char* szSep  = CS_XOR("  |  ");
        const char* szTime = tbuf;

        ImFont* fBold = fonts::VerdanaBold();
        ImFont* fNorm = fonts::Verdana();
        const float fs = fonts::kUiSize;

        ImVec2 szNameSz = fBold->CalcTextSizeA(fs, FLT_MAX, 0.f, szName);
        ImVec2 szSepSz  = fNorm->CalcTextSizeA(fs, FLT_MAX, 0.f, szSep);
        ImVec2 szTimeSz = fNorm->CalcTextSizeA(fs, FLT_MAX, 0.f, szTime);

        float totalW = szNameSz.x + szSepSz.x + szTimeSz.x;
        float textH  = szNameSz.y;
        float padX = 12.f, padY = 7.f;
        float dotSpace = 14.f;
        float wmW = totalW + padX * 2.f + dotSpace;
        float wmH = textH + padY * 2.f + 3.f;

        if (wmX == -1.f) wmX = io.DisplaySize.x - wmW - 12.f;
        UpdateDrag(wmDrag, wmX, wmY, wmW, wmH, wmH);

        const float R = 4.f;
        ImVec2 wmMin(wmX, wmY);
        ImVec2 wmMax(wmX + wmW, wmY + wmH);

        ksd::BlurRegion(wmMin, wmMax);
        dl->AddRectFilled(wmMin, wmMax, style::glass(18, 18, 22, 0.20f), R);
        dl->AddRectFilled(wmMin, wmMax, style::glass(14, 14, 18, 0.20f), R);
        dl->AddRect(wmMin, wmMax, style::col::outline(), R, 0, 1.f);

        const float accentR = vars::colorAccent[0];
        const float accentG = vars::colorAccent[1];
        const float accentB = vars::colorAccent[2];
        const float flowSpeed = 1.70f;
        const float flowWaves = 0.80f;
        const float flowFloor = 0.40f;
        const float scroll = static_cast<float>(ImGui::GetTime()) * flowSpeed;
        const float barW = wmW - R * 2.f;
        const int   segs = 32;
        const float segW = barW / static_cast<float>(segs);

        for (int i = 0; i < segs; ++i)
        {
            auto sample = [&](int idx) -> ImU32
            {
                float u = static_cast<float>(idx) / static_cast<float>(segs);
                float wave = 0.5f + 0.5f * std::sin(6.2831853f * flowWaves * (u - scroll));
                float level = flowFloor + (1.f - flowFloor) * wave;
                return IM_COL32(
                    static_cast<int>(accentR * 255.f * level),
                    static_cast<int>(accentG * 255.f * level),
                    static_cast<int>(accentB * 255.f * level), 255);
            };
            ImVec2 sMin(wmX + R + segW * static_cast<float>(i), wmY);
            ImVec2 sMax(wmX + R + segW * static_cast<float>(i + 1), wmY + 3.f);
            dl->AddRectFilledMultiColor(sMin, sMax, sample(i), sample(i + 1), sample(i + 1), sample(i));
        }

        float cy = wmY + padY + 3.f;
        float cx = wmX + padX + dotSpace;

        dl->AddCircleFilled(
            ImVec2(wmX + padX + 3.f, cy + textH * 0.5f),
            3.f, style::accentU32());

        const ImU32 cText = IM_COL32(205, 205, 205, 255);
        const ImU32 cDim  = IM_COL32(128, 128, 128, 255);

        dl->AddText(fBold, fs, ImVec2(cx, cy), cText, szName);
        cx += szNameSz.x;
        dl->AddText(fNorm, fs, ImVec2(cx, cy), cDim, szSep);
        cx += szSepSz.x;
        dl->AddText(fNorm, fs, ImVec2(cx, cy), cDim, szTime);
    }


    if (C_GET(bool, Vars.bDrawFov))
    {
        ImVec2 sc = io.DisplaySize;
        ImVec2 ctr = ImVec2(sc.x * 0.5f, sc.y * 0.5f);
        float  rad = (C_GET(float, Vars.flLegitFov) / 180.f) * sc.x;
        Color_t col = C_GET(Color_t, Vars.colFov);
        dl->AddCircle(ctr, rad, IM_COL32(col.r, col.g, col.b, col.a), 64, 1.5f);
    }


    ImGui::PopFont();
    dl->PopClipRect();


    ImGui::EndFrame();
    D::Render();
}
bool F::OnDrawObject(void* pAnimatableSceneObjectDesc, void* pDx11, CMeshData* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk, void* pUnk2)
{
    return VISUALS::OnDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
}

void F::OnFrameStageNotify(int nStage)
{

}
