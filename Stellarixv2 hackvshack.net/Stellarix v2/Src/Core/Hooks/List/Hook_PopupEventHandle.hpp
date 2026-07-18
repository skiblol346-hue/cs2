#pragma once
#include <SafetyHook/safetyhook.hpp>


inline SafetyHookInline PopupEventHandle_o{};
auto __fastcall Hook_PopupEventHandle( void* a1, const char* szPopupName, void* a3, float flUnk ) -> void*;