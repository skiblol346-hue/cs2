#include "Hook_ValidateCamera.hpp"
#include <Core/Interfaces/List/CCSGOInput.hpp>
#include <Core/StellarixClient.hpp>
#include <Core/Sdk/Globals.hpp>


auto __fastcall Hook_ValidateCamera( CCSGOInput* pCCSGOInput, int a2 ) -> void
{
	if ( !GetCheatLauncher( )->CheatIsAlive( ) )
		return ValidateCamera_o.call<void>( pCCSGOInput, a2 );

	if ( !pCCSGOInput )
		return ValidateCamera_o.call<void>( pCCSGOInput, a2 );

	QAngle_t angOriginalAngle = pCCSGOInput->GetViewAngle( );
	pCCSGOInput->SetViewAngle( LocalPlayerData.angViewAngle );
	ValidateCamera_o.call<void>( pCCSGOInput, a2 );
	pCCSGOInput->SetViewAngle( angOriginalAngle );
}
