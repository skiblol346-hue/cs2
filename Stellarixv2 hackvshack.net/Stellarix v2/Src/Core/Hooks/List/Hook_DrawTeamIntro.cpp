#include "Hook_DrawTeamIntro.hpp"
#include <Core/Config/Variables.hpp>
#include <Launcher.hpp>

auto __fastcall Hook_DrawTeamIntro( C_CSGameRules* pC_CSGameRules, void* a2, bool* bNeedShow ) -> void
{
	if ( !GetCheatLauncher( )->CheatIsAlive( ) )
		return DrawTeamIntro_o.call<void>( pC_CSGameRules, a2, bNeedShow );

	if ( !pC_CSGameRules || !bNeedShow )
		return DrawTeamIntro_o.call<void>( pC_CSGameRules, a2, bNeedShow );

	if ( C_GET( bool, Vars.bRemoveTeamIntro ) )
		*bNeedShow = false;

	DrawTeamIntro_o.call<void>( pC_CSGameRules, a2, bNeedShow );
}
