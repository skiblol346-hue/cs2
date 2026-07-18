#include "EntityList.hpp"
#include "Offsets.hpp"
#include "Vector.hpp"
#include <windows.h>
#include <algorithm>
#include <cmath>

CEntityList g_EntityList_Logic;

static bool SRead(uintptr_t addr, void* out, size_t size) {
    __try {
        if (addr < 0x10000 || addr > 0x7FFFFFFEFFFF) return false;
        memcpy(out, (void*)addr, size);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

template <typename T>
static bool SR(uintptr_t addr, T& out) {
    return SRead(addr, &out, sizeof(T));
}

void CEntityList::Update() {
    uintptr_t clientBase = (uintptr_t)GetModuleHandleA("client.dll");
    if (!clientBase || Offsets::dwEntityList == 0) return;

    uintptr_t entityList = 0;
    if (!SR(clientBase + Offsets::dwEntityList, entityList) || !entityList) return;

    uintptr_t localPawn = 0;
    if (Offsets::dwLocalPlayerPawn) {
        SR(clientBase + Offsets::dwLocalPlayerPawn, localPawn);
    }

    std::vector<EntityData> temp;
    temp.reserve(64);

    for (int i = 1; i < 64; i++) {
        uintptr_t listEntry = 0;
        if (!SR(entityList + 0x10 + 0x8LL * (i >> 9), listEntry) || !listEntry) continue;

        uintptr_t entityController = 0;
        if (!SR(listEntry + 0x70LL * (i & 0x1FF), entityController) || !entityController || entityController < 0x10000) continue;

        uint32_t pawnHandle = 0;
        if (!SR(entityController + Schemas::CCSPlayerController::m_hPawn, pawnHandle)) continue;
        if (!pawnHandle || pawnHandle == 0xFFFFFFFF) continue;

        uint32_t pawnIndex = pawnHandle & 0x7FFF;

        uintptr_t pawnListEntry = 0;
        if (!SR(entityList + 0x10 + 0x8LL * (pawnIndex >> 9), pawnListEntry) || !pawnListEntry) continue;

        uintptr_t pawn = 0;
        if (!SR(pawnListEntry + 0x70LL * (pawnIndex & 0x1FF), pawn) || !pawn || pawn < 0x10000) continue;

        if (pawn == localPawn) continue;

        bool duplicate = false;
        for (int j = 0; j < (int)temp.size(); j++) {
            if (temp[j].pawn == pawn) { duplicate = true; break; }
        }
        if (duplicate) continue;

        int health = 0;
        if (!SR(pawn + Schemas::C_BaseEntity::m_iHealth, health)) continue;
        if (health <= 0 || health > 100) continue;

        EntityData ed;
        ed.pawn = pawn;
        ed.controller = entityController;
        temp.push_back(ed);
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    m_entities = std::move(temp);
}

EntityData CEntityList::GetEntityByIndex(int index) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (index < 0 || index >= (int)m_entities.size()) return { 0, 0 };
    return m_entities[index];
}

int CEntityList::GetEntityCount() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return (int)m_entities.size();
}