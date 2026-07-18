#pragma once
#include <SafetyHook/safetyhook.hpp>

inline SafetyHookInline MouseInputEnabled_o{};
auto __fastcall Hook_MouseInputEnabled( void* pThisptr )->bool;