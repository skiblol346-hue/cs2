#include "LegitBot.hpp"
#include "../Menu/Menu.hpp"
#include "../../SDK/Offsets.hpp"
#include "../../SDK/Vector.hpp"
#include "../../Core/SafeRead.hpp"
#include <windows.h>
#include <cmath>

void LegitBot_AutoFire::Run(uintptr_t clientBase, uintptr_t localPawn) {
    if (!lb_af_enable || !localPawn) return;
    if (!LegitBot_General::IsConditionMet(localPawn)) return;

    int entIndex = 0;
    if (!SafeRead::ReadInt(localPawn + Schemas::C_CSPlayerPawnBase::m_iIDEntIndex, entIndex) || entIndex <= 0)
        return;

    uintptr_t entityList = 0;
    if (!SafeRead::ReadPtr(clientBase + Offsets::dwEntityList, entityList) || !entityList)
        return;

    uintptr_t listEntry = 0;
    if (!SafeRead::ReadPtr(entityList + 0x10 + 0x8LL * (entIndex >> 9), listEntry) || !listEntry)
        return;

    uintptr_t targetPawn = 0;
    if (!SafeRead::ReadPtr(listEntry + 0x70LL * (entIndex & 0x1FF), targetPawn) || !targetPawn)
        return;

    int localTeam = 0, targetTeam = 0, health = 0;
    SafeRead::ReadInt(localPawn  + Schemas::C_BaseEntity::m_iTeamNum, localTeam);
    SafeRead::ReadInt(targetPawn + Schemas::C_BaseEntity::m_iTeamNum, targetTeam);
    SafeRead::ReadInt(targetPawn + Schemas::C_BaseEntity::m_iHealth,  health);

    if (localTeam == targetTeam || health <= 0) return;

    Vector3 localOrigin = {}, targetOrigin = {};
    SafeRead::Read<Vector3>(localPawn  + Schemas::C_BasePlayerPawn::m_vOldOrigin, localOrigin);
    SafeRead::Read<Vector3>(targetPawn + Schemas::C_BasePlayerPawn::m_vOldOrigin, targetOrigin);

    float dx = localOrigin.x - targetOrigin.x;
    float dy = localOrigin.y - targetOrigin.y;
    float dz = localOrigin.z - targetOrigin.z;
    float distance = sqrtf(dx * dx + dy * dy + dz * dz);

    uint32_t flags = 0;
    SafeRead::Read<uint32_t>(localPawn + Schemas::C_BasePlayerPawn::m_fFlags, flags);
    bool inAir = !(flags & (1 << 0));

    float velocity[3] = {};
    SafeRead::Read<float[3]>(localPawn + Schemas::C_BasePlayerPawn::m_vecVelocity, velocity);
    float speed = sqrtf(velocity[0] * velocity[0] + velocity[1] * velocity[1]);

    float targetRadiusInAngles = atan2f(12.0f, distance) * (180.0f / 3.14159265f);
    float baseInaccuracy = 0.3f;
    if (inAir) baseInaccuracy += 10.0f;
    baseInaccuracy += speed / 25.0f;

    bool isScoped = false;
    SafeRead::ReadBool(localPawn + Schemas::C_CSPlayerPawn::m_bIsScoped, isScoped);
    if (isScoped) baseInaccuracy *= 0.1f;

    float hitChance = (targetRadiusInAngles / baseInaccuracy) * 100.0f;
    if (hitChance > 100.0f) hitChance = 100.0f;

    if (hitChance < lb_min_hit_chance && distance >= 150.0f) return;

    if (lb_delay_shots > 0) {
        static ULONGLONG lastShot = 0;
        ULONGLONG now = GetTickCount64();
        if (now - lastShot < static_cast<ULONGLONG>(lb_delay_shots)) return;
        lastShot = now;
    }

    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
    Sleep(1);
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}
