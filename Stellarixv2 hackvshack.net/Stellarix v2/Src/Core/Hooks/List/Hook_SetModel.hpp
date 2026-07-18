#pragma once
#include <SafetyHook/safetyhook.hpp>

inline SafetyHookInline SetModel_o{};
auto __fastcall Hook_SetModel( void* rcx, char* szModel ) -> void;