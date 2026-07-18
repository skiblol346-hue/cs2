#include "Hook_RenderCrosshair.hpp"
#include <Core/Config/Variables.hpp>
#include <Core/Sdk/EntityData.hpp>
#include <Launcher.hpp>

auto __fastcall Hook_RenderCrosshair( void* a1 ) -> bool
{
	if ( !GetCheatLauncher( )->CheatIsAlive( ) )
		return RenderCrosshair_o.call<bool>( a1 );

	try
	{
		CCSPlayerController* pLocalController = CCSPlayerController::GetLocalPlayerController( );
		C_CSPlayerPawn* pLocalPawn = C_CSPlayerPawn::GetPawnFromController( pLocalController );
		if ( pLocalPawn && pLocalPawn->m_bIsScoped( ) )
			return RenderCrosshair_o.call<bool>( a1 );
	}
	catch ( ... )
	{

	}

	if ( C_GET( bool, Vars.bForceCrosshair ) )
		return true;

	return RenderCrosshair_o.call<bool>( a1 );
}
