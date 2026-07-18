#pragma once
#include <SafetyHook/safetyhook.hpp>

class CBaseSceneData;
class CMaterial2;

inline SafetyHookInline DrawAggregateSceneObjectArray_o{};
auto __fastcall Hook_DrawAggregateSceneObjectArray( void* a1, void* a2, CBaseSceneData* pSceneData ) -> void*;