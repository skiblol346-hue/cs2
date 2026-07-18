#pragma once
#include <SafetyHook/safetyhook.hpp>

class C_BaseEntity;

inline SafetyHookInline MarkInterpolationLatchFlagsDirty_o{};
auto Hook_MarkInterpolationLatchFlagsDirty( C_BaseEntity* pC_BaseEntity, int iInterpFlag ) -> void;
