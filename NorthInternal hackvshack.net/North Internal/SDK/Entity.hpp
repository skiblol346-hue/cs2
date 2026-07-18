#pragma once
#include "Vector.hpp"
#include <cstdint>

struct EntityData_t {
    uintptr_t pawn_ptr;
    int health;
    int team;
    Vector3 origin;
    bool is_scoped;
    int armor;
};

struct BBox_t {
    float x, y, w, h;
};