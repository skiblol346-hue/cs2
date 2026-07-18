#include "Hook_OnAddEntity.hpp"
#include <Core/StellarixClient.hpp>
#include <Launcher.hpp>

auto __fastcall Hook_OnAddEntity( void* rcx, CEntityInstance* pInstance, CBaseHandle hHandle ) -> void*
{
	if ( !GetCheatLauncher( )->CheatIsAlive( ) )
		return OnAddEntity_o.fastcall<void*>( rcx, pInstance, hHandle );

	GetStellarixClient( )->OnAddEntityHook( pInstance, hHandle );

	return OnAddEntity_o.fastcall<void*>( rcx, pInstance, hHandle );
}
