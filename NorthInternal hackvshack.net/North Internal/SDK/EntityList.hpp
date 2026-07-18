#pragma once
#include <vector>
#include <mutex>
#include <cstdint>

struct EntityData {
    uintptr_t pawn;
    uintptr_t controller;
};

class CEntityList {
public:
    void Update();
    EntityData GetEntityByIndex(int index);
    int GetEntityCount();

private:
    std::vector<EntityData> m_entities;
    std::mutex m_mutex;
};

extern CEntityList g_EntityList_Logic;