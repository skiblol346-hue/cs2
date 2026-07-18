#include "Hook_DrawArray.hpp"
#include <Core/Config/Variables.hpp>
#include <Core/StellarixClient.hpp>


auto __fastcall Hook_DrawArray( void* pAnimatableSceneObjectDesc, void* pDx11, CMeshDrawPrimitive* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk ) -> void*
{
	if ( nDataCount < 1 )
		return nullptr;

	GetStellarixClient( )->OnDrawArrayHook( pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk );

	return DrawArray_o.unsafe_fastcall<void*>( pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk );
}
