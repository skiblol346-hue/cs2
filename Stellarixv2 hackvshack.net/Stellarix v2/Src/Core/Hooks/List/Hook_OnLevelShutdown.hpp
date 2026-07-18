#pragma once
#include <SafetyHook/safetyhook.hpp>

inline SafetyHookInline OnLevelShutdown_o{};
auto Hook_OnLevelShutdown( ) -> void*;
