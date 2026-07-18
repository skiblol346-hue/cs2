#pragma once
#include <cstdint>
#include <vector>

namespace LegitBot_General {
    bool IsConditionMet(uintptr_t localPawn);
}

namespace LegitBot_HitGroups {
    std::vector<int> GetActiveBones();
}

namespace LegitBot_Aim {
    void Run(uintptr_t clientBase, uintptr_t localPawn);
    void Draw(uintptr_t localPawn);
}

namespace LegitBot_AutoFire {
    void Run(uintptr_t clientBase, uintptr_t localPawn);
}