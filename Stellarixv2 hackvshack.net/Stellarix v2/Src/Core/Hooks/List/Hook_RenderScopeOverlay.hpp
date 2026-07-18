#pragma once
#include <SafetyHook/safetyhook.hpp>

class C_CSPlayerPawn;

inline SafetyHookInline RenderScopeOverlay_o{};
auto __fastcall Hook_RenderScopeOverlay( C_CSPlayerPawn* pC_CSPlayerPawn, void* a2 ) -> void;