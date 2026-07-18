#pragma once
#include <SafetyHook/safetyhook.hpp>

class CCSGOInput;

inline SafetyHookInline ValidateCamera_o{};
auto __fastcall Hook_ValidateCamera( CCSGOInput* pCCSGOInput, int a2 ) -> void;