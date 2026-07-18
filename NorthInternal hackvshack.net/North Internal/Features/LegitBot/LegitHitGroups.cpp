#include "LegitBot.hpp"
#include "../Menu/Menu.hpp"

std::vector<int> LegitBot_HitGroups::GetActiveBones() {
    std::vector<int> bones;
    if (hg_selections[0]) bones.push_back(6);
    if (hg_selections[1]) bones.push_back(5);
    if (hg_selections[2]) bones.push_back(4);
    if (hg_selections[3]) bones.push_back(3);
    if (hg_selections[4]) bones.push_back(2);
    if (hg_selections[5]) {
        bones.push_back(25);
        bones.push_back(22);
    }
    if (bones.empty()) bones.push_back(6);
    return bones;
}