#pragma once
#include <SafetyHook/safetyhook.hpp>

class CCSGOInput;
class CUserCmd;

inline SafetyHookInline CreateMovePredict_o{};
auto __fastcall Hook_CreateMovePredict( CCSGOInput* pInput, int nSlot, CUserCmd* pUserCmd ) -> void;