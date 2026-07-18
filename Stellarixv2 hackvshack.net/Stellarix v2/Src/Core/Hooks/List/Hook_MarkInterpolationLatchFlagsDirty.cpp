#include "Hook_MarkInterpolationLatchFlagsDirty.hpp"
#include <Core/Sdk/EntityData.hpp>
#include <Core/Sdk/Globals.hpp>

auto Hook_MarkInterpolationLatchFlagsDirty( C_BaseEntity* pC_BaseEntity, int iInterpFlag ) -> void
{
	if ( !GetCheatLauncher( )->CheatIsAlive( ) )
		return MarkInterpolationLatchFlagsDirty_o.call<void>( pC_BaseEntity, iInterpFlag );


	try
	{
		CCSPlayerController* pLocalController = CCSPlayerController::GetLocalPlayerController( );
		C_CSPlayerPawn* pLocalPawn = C_CSPlayerPawn::GetPawnFromController( pLocalController );

		if ( pC_BaseEntity && pC_BaseEntity->m_iHealth( ) > 0 && pLocalPawn )
		{
			if ( pC_BaseEntity->GetRefEHandle( ) != pLocalPawn->GetRefEHandle( ) )
				return;
		}

	}
	catch ( ... )
	{

	}
	return MarkInterpolationLatchFlagsDirty_o.call<void>( pC_BaseEntity, iInterpFlag );
}
