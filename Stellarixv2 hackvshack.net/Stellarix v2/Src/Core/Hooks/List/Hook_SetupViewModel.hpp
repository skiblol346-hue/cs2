#pragma once
#include <SafetyHook/safetyhook.hpp>

inline SafetyHookInline SetupViewModel_o{};
auto __fastcall Hook_SetupViewModel( float* flA1, float* flOffsets, float* flViewModelFov ) -> void;