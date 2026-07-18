#include "LegitBot.hpp"
#include "../Menu/Menu.hpp"
#include "../../SDK/Offsets.hpp"
#include "../../Core/SafeRead.hpp"

bool LegitBot_General::IsConditionMet(uintptr_t localPawn) {
    if (!lb_enable || !localPawn) return false;
    if (!SafeRead::IsValidPtr(localPawn)) return false;

    int health = 0;
    if (!SafeRead::ReadInt(localPawn + Schemas::C_BaseEntity::m_iHealth, health))
        return false;

    return health > 0;
}
