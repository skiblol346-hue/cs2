#pragma once
#include <SafetyHook/safetyhook.hpp>

inline SafetyHookInline OnLevelInit_o{};
auto Hook_OnLevelInit( void* pClientModeShared, const char* szNewMap ) -> void*;
