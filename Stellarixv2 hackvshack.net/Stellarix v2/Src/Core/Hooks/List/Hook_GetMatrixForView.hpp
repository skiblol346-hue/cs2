#pragma once
#include <windows.h>
#include <dxgi.h>
#include <Core/Sdk/Datatypes/Matrix.hpp>
#include <SafetyHook/safetyhook.hpp>

class CRenderGameSystem;
class IViewRender;

inline SafetyHookInline GetMatrixForView_o{};
auto Hook_GetMatrixForView( CRenderGameSystem* pRenderGameSystem, IViewRender* pViewRender, ViewMatrix_t* pOutWorldToView, ViewMatrix_t* pOutViewToProjection, ViewMatrix_t* pOutWorldToProjection, ViewMatrix_t* pOutWorldToPixels ) -> void;
