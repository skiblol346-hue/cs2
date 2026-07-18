#include "Hook_CreateMovePredict.hpp"
#include <Core/Sdk/Datatypes/CUserCmd.hpp>
#include <Launcher.hpp>
#include <Core/StellarixClient.hpp>

auto __fastcall Hook_CreateMovePredict( CCSGOInput* pInput, int nSlot, CUserCmd* pUserCmd ) -> void
{
	if ( !GetCheatLauncher( )->CheatIsAlive( ) )
		return CreateMovePredict_o.fastcall<void>( pInput, nSlot, pUserCmd );

	if ( !pInput || !pUserCmd )
		return CreateMovePredict_o.fastcall<void>( pInput, nSlot, pUserCmd );

	GetStellarixClient( )->OnCreateMovePredictHook( pInput, nSlot, pUserCmd );
}
