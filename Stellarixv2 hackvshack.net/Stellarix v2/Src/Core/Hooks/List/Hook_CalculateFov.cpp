#include "Hook_CalculateFov.hpp"
#include <Launcher.hpp>
#include <Core/Sdk/EntityData.hpp>
#include <Core/Config/Variables.hpp>

auto __fastcall Hook_CalculateFov( CCSPlayerBase_CameraServices* pCCSPlayerBase_CameraServices ) -> float
{
	if ( !GetCheatLauncher( )->CheatIsAlive( ) )
		return CalculateFov_o.fastcall<float>( pCCSPlayerBase_CameraServices );

	CCSPlayerController* pLocalController = CCSPlayerController::GetLocalPlayerController( );
	C_CSPlayerPawn* pLocalPawn = C_CSPlayerPawn::GetPawnFromController( pLocalController );

	if ( pLocalPawn && pLocalPawn->m_pWeaponServices( ) )
	{
		try
		{
			if ( pLocalPawn->m_bIsScoped( ) )
			{
				 if (C_GET( bool, Vars.bForceFovInZoom ) )
					 return C_GET( float, Vars.flFov );
				 else 
					 return CalculateFov_o.fastcall<float>( pCCSPlayerBase_CameraServices );
			}
		}
		catch ( ... )
		{

		}
	}

	return C_GET( float, Vars.flFov );
}
