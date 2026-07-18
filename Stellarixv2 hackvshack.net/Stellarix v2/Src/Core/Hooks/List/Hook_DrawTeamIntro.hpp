#pragma once
#include <SafetyHook/safetyhook.hpp>

class C_CSGameRules;

inline SafetyHookInline DrawTeamIntro_o{};
auto __fastcall Hook_DrawTeamIntro( C_CSGameRules* pC_CSGameRules, void* a2, bool* bNeedShow ) -> void;