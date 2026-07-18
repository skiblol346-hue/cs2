#include "Hook_OnRemoveEntity.hpp"
#include <Launcher.hpp>
#include <Core/StellarixClient.hpp>

auto __fastcall Hook_OnRemoveEntity( void* rcx, CEntityInstance* pInstance, CBaseHandle hHandle ) -> void*
{
	if ( !GetCheatLauncher( )->CheatIsAlive( ) )
		return OnRemoveEntity_o.fastcall<void*>( rcx, pInstance, hHandle );

	GetStellarixClient( )->OnRemoveEntityHook( pInstance, hHandle );

	return OnRemoveEntity_o.fastcall<void*>( rcx, pInstance, hHandle );
}
