#include "LegitBot.hpp"
#include "../Menu/Menu.hpp"
#include "../../SDK/Vector.hpp"
#include "../../SDK/Offsets.hpp"
#include "../../Core/SafeRead.hpp"
#include "../../Dependencies/ImGui/imgui.h"
#include <windows.h>
#include <cmath>
#include <algorithm>

struct QAngle {
    float x, y;
    void Normalize() {
        while (y < -180.0f) y += 360.0f;
        while (y > 180.0f) y -= 360.0f;
        if (x >  89.0f) x =  89.0f;
        if (x < -89.0f) x = -89.0f;
    }
};

static QAngle CalcAngle(const Vector3& src, const Vector3& dst) {
    Vector3 delta = { dst.x - src.x, dst.y - src.y, dst.z - src.z };
    float hyp = sqrtf(delta.x * delta.x + delta.y * delta.y);
    QAngle angle;
    angle.x = atan2f(-delta.z, hyp) * (180.0f / 3.14159265f);
    angle.y = atan2f(delta.y, delta.x) * (180.0f / 3.14159265f);
    angle.Normalize();
    return angle;
}

static bool GetBonePos(uintptr_t pawn, int boneIndex, Vector3& out) {
    __try {
        uintptr_t gsn = 0;
        if (!SafeRead::ReadPtr(pawn + Schemas::C_BaseEntity::m_pGameSceneNode, gsn) || !gsn)
            return false;

        uintptr_t boneArray = 0;
        if (!SafeRead::ReadPtr(gsn + Schemas::CGameSceneNode::m_modelState + Schemas::CModelState::m_boneArray, boneArray) || !boneArray)
            return false;

        uintptr_t boneAddr = boneArray + static_cast<uintptr_t>(boneIndex) * 32;
        if (!SafeRead::IsValidPtr(boneAddr)) return false;

        float bone[3];
        if (!SafeRead::Read<float[3]>(boneAddr, bone)) return false;

        if (isnan(bone[0]) || isnan(bone[1]) || isnan(bone[2])) return false;
        if (bone[0] == 0.0f && bone[1] == 0.0f && bone[2] == 0.0f) return false;

        out = { bone[0], bone[1], bone[2] };
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

void LegitBot_Aim::Run(uintptr_t clientBase, uintptr_t localPawn) {
    if (!lb_enable || !localPawn) return;

    if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000)) return;

    if (!LegitBot_General::IsConditionMet(localPawn)) return;

    uintptr_t entityList = 0;
    if (!SafeRead::ReadPtr(clientBase + Offsets::dwEntityList, entityList) || !entityList)
        return;

    QAngle* viewAngles = reinterpret_cast<QAngle*>(clientBase + Offsets::dwViewAngles);

    Vector3 localOrigin = {}, viewOffset = {};
    SafeRead::Read<Vector3>(localPawn + Schemas::C_BasePlayerPawn::m_vOldOrigin, localOrigin);
    SafeRead::Read<Vector3>(localPawn + Schemas::C_CSPlayerPawnBase::m_vecViewOffset, viewOffset);
    Vector3 eyePos = { localOrigin.x + viewOffset.x, localOrigin.y + viewOffset.y, localOrigin.z + viewOffset.z };

    int localTeam = 0;
    SafeRead::ReadInt(localPawn + Schemas::C_BaseEntity::m_iTeamNum, localTeam);

    float bestFov = lb_amount;
    QAngle targetAngle = { 0.0f, 0.0f };
    bool found = false;
    float targetDistance = 0.0f;

    for (int i = 1; i < 64; i++) {
        uintptr_t listEntry = 0;
        if (!SafeRead::ReadPtr(entityList + 0x10 + 0x8LL * (i >> 9), listEntry) || !listEntry) continue;

        uintptr_t controller = 0;
        if (!SafeRead::ReadPtr(listEntry + 0x70LL * (i & 0x1FF), controller) || !controller) continue;

        uint32_t pawnHandle = 0;
        if (!SafeRead::Read<uint32_t>(controller + Schemas::CCSPlayerController::m_hPawn, pawnHandle)) continue;
        if (!pawnHandle || pawnHandle == 0xFFFFFFFF) continue;

        uintptr_t pListEntry = 0;
        if (!SafeRead::ReadPtr(entityList + 0x10 + 0x8LL * ((pawnHandle & 0x7FFF) >> 9), pListEntry) || !pListEntry) continue;

        uintptr_t pawn = 0;
        if (!SafeRead::ReadPtr(pListEntry + 0x70LL * (pawnHandle & 0x1FF), pawn) || !pawn || pawn == localPawn) continue;

        int health = 0, team = 0;
        SafeRead::ReadInt(pawn + Schemas::C_BaseEntity::m_iHealth, health);
        SafeRead::ReadInt(pawn + Schemas::C_BaseEntity::m_iTeamNum, team);
        if (health <= 0 || team == localTeam) continue;

        for (int b : LegitBot_HitGroups::GetActiveBones()) {
            Vector3 bonePos;
            if (!GetBonePos(pawn, b, bonePos)) continue;

            QAngle angle = CalcAngle(eyePos, bonePos);

            float fovX = fabsf(angle.y - viewAngles->y);
            float fovY = fabsf(angle.x - viewAngles->x);
            if (fovX > 180.0f) fovX = 360.0f - fovX;
            float fov = sqrtf(fovX * fovX + fovY * fovY);

            if (fov < bestFov) {
                bestFov = fov;
                targetAngle = angle;
                found = true;
                float dx = eyePos.x - bonePos.x;
                float dy = eyePos.y - bonePos.y;
                float dz = eyePos.z - bonePos.z;
                targetDistance = sqrtf(dx * dx + dy * dy + dz * dz);
            }
        }
    }

    if (!found) return;

    QAngle delta = { targetAngle.x - viewAngles->x, targetAngle.y - viewAngles->y };
    delta.Normalize();

    float smooth = (std::max)(1.0f, lb_smooth_yaw);

    uint32_t flags = 0;
    SafeRead::Read<uint32_t>(localPawn + Schemas::C_BasePlayerPawn::m_fFlags, flags);
    bool inAir = !(flags & (1 << 0));

    float velocity[3] = {};
    SafeRead::Read<float[3]>(localPawn + Schemas::C_BasePlayerPawn::m_vecVelocity, velocity);
    float speed = sqrtf(velocity[0] * velocity[0] + velocity[1] * velocity[1]);

    float targetRadiusInAngles = atan2f(14.0f, targetDistance) * (180.0f / 3.14159265f);
    float baseInaccuracy = 0.5f;
    if (inAir) baseInaccuracy += 12.0f;
    baseInaccuracy += speed / 20.0f;

    float hitChance = (targetRadiusInAngles / baseInaccuracy) * 100.0f;
    if (hitChance > 100.0f) hitChance = 100.0f;

    if (hitChance >= lb_min_hit_chance || targetDistance < 200.0f) {
        if (inAir || speed > 150.0f)
            smooth = 1.0f;

        viewAngles->x += delta.x / smooth;
        viewAngles->y += delta.y / smooth;
        viewAngles->Normalize();
    }
}

void LegitBot_Aim::Draw(uintptr_t localPawn) {
    if (!lb_enable || !lb_draw_fov || !localPawn) return;
    ImVec2 center = ImGui::GetIO().DisplaySize;
    center.x /= 2.0f;
    center.y /= 2.0f;
    float radius = (lb_amount / 90.0f) * center.x;
    ImGui::GetBackgroundDrawList()->AddCircle(center, radius, ImColor(255, 255, 255, 100), 100);
}
