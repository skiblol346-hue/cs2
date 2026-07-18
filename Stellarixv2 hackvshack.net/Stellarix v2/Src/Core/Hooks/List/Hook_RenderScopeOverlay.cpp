#include "Hook_RenderScopeOverlay.hpp"
#include <Core/Sdk/EntityData.hpp>
#include <Core/Config/Variables.hpp>
#include <Launcher.hpp>

auto __fastcall Hook_RenderScopeOverlay( C_CSPlayerPawn* pC_CSPlayerPawn, void* a2 ) -> void
{
	if ( !GetCheatLauncher( )->CheatIsAlive( ) )
		return RenderScopeOverlay_o.call<void>( pC_CSPlayerPawn, a2 );

	if ( !pC_CSPlayerPawn )
		return RenderScopeOverlay_o.call<void>( pC_CSPlayerPawn, a2 );

	if ( !pC_CSPlayerPawn->m_pWeaponServices( ) )
		return RenderScopeOverlay_o.call<void>( pC_CSPlayerPawn, a2 );

	bool bScope = false;
	try
	{
		bScope = pC_CSPlayerPawn->m_bIsScoped( );
		if ( C_GET( bool, Vars.bRemoveScope ) )
			pC_CSPlayerPawn->m_bIsScoped( ) = false;
	}
	catch ( ... )
	{
	}

	RenderScopeOverlay_o.call<void>( pC_CSPlayerPawn, a2 );
	pC_CSPlayerPawn->m_bIsScoped( ) = bScope;
}
