#include "Hook_GetMatrixForView.hpp"
#include <Core/Sdk/EntityCache/EntityCache.hpp>
#include <Core/Interfaces/List/IEngineClient.hpp>
#include <Core/Sdk/Globals.hpp>


auto Hook_GetMatrixForView( CRenderGameSystem* pRenderGameSystem, IViewRender* pViewRender, ViewMatrix_t* pOutWorldToView, ViewMatrix_t* pOutViewToProjection, ViewMatrix_t* pOutWorldToProjection, ViewMatrix_t* pOutWorldToPixels ) -> void
{
	GetMatrixForView_o.fastcall<void>( pRenderGameSystem, pViewRender, pOutWorldToView, pOutViewToProjection, pOutWorldToProjection, pOutWorldToPixels );

	if ( pOutWorldToProjection )
		CheatData.viewMatrix = *pOutWorldToProjection;
}
