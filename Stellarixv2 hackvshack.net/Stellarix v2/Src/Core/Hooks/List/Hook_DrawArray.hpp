#pragma once
#include <SafetyHook/safetyhook.hpp>

class CMeshDrawPrimitive;

inline SafetyHookInline DrawArray_o{};
auto __fastcall Hook_DrawArray( void* pAnimatableSceneObjectDesc, void* pDx11, CMeshDrawPrimitive* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk ) -> void*;