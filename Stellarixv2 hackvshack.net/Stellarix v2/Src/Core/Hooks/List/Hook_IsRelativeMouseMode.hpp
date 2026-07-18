#pragma once
#include <SafetyHook/safetyhook.hpp>

inline SafetyHookInline IsRelativeMouseMode_o{ };
auto Hook_IsRelativeMouseMode( void* pThisptr, bool bActive ) -> void*;
