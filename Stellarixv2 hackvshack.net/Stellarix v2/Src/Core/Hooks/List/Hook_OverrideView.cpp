#include "Hook_OverrideView.hpp"
#include <Launcher.hpp>
#include <Core/Interfaces/InterfaceManager.hpp>
#include <Core/Interfaces/List/IEngineClient.hpp>
#include <Core/StellarixClient.hpp>


auto __fastcall Hook_OverrideView( void* pClientModeCSNormal, CViewSetup* pViewSetup ) -> void
{
	if ( !GetCheatLauncher( )->CheatIsAlive( ) )
		return OverrideView_o.fastcall<void>( pClientModeCSNormal, pViewSetup );
	
	if ( !pClientModeCSNormal || !pViewSetup )
		return OverrideView_o.fastcall<void>( pClientModeCSNormal, pViewSetup );

	if ( !GetInterfaceManager( )->GetEngineClient( )->IsConnected( ) || !GetInterfaceManager( )->GetEngineClient( )->IsInGame( ) )
		return OverrideView_o.fastcall<void>( pClientModeCSNormal, pViewSetup );

	GetStellarixClient( )->OnOverrideViewHook( pClientModeCSNormal, pViewSetup );
}
