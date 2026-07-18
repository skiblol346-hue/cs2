#include "Hook_RenderLighting.hpp"
#include <Core/StellarixClient.hpp>

auto __fastcall Hook_RenderLighting( void* a1, CAggregateSceneObjectLighting* pCAggregateSceneObjectLighting, void* a2 ) -> void
{
	GetStellarixClient( )->OnRenderLightingHook( pCAggregateSceneObjectLighting );

	RenderLighting_o.unsafe_call<void>( a1, pCAggregateSceneObjectLighting, a2 );
}
