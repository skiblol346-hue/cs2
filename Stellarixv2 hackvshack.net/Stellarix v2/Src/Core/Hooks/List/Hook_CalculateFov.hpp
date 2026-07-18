#pragma once
#include <SafetyHook/safetyhook.hpp>

class CCSPlayerBase_CameraServices;

inline SafetyHookInline CalculateFov_o{};
auto __fastcall Hook_CalculateFov( CCSPlayerBase_CameraServices* pCCSPlayerBase_CameraServices ) -> float;