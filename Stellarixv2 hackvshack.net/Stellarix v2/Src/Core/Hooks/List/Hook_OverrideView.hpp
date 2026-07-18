#pragma once
#include <SafetyHook/safetyhook.hpp>

class CViewSetup;

inline SafetyHookInline OverrideView_o{};
auto __fastcall Hook_OverrideView( void* pClientModeCSNormal, CViewSetup* pViewSetup ) -> void;