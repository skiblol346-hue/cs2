#include "Hook_SetupViewModel.hpp"
#include <Core/StellarixClient.hpp>
#include <Core/Interfaces/InterfaceManager.hpp>
#include <Core/Interfaces/List/IEngineClient.hpp>
#include <Launcher.hpp>
#include <Core/Config/Variables.hpp>


auto __fastcall Hook_SetupViewModel( float* flA1, float* flOffsets, float* flViewModelFov ) -> void
{
	if ( !GetCheatLauncher( )->CheatIsAlive( ) )
		return SetupViewModel_o.fastcall<void>( flA1, flOffsets, flViewModelFov );

	SetupViewModel_o.fastcall<void>( flA1, flOffsets, flViewModelFov );

	if ( flOffsets && IM_ARRAYSIZE( flOffsets ) >= 2 )
	{
		flOffsets[ 0 ] = C_GET( float, Vars.flViewModelOffsetX );
		flOffsets[ 1 ] = C_GET( float, Vars.flViewModelOffsetY );
		flOffsets[ 2 ] = C_GET( float, Vars.flViewModelOffsetZ );
	}

	if ( flViewModelFov )
		*flViewModelFov = C_GET( float, Vars.flViewModelFov );
}
