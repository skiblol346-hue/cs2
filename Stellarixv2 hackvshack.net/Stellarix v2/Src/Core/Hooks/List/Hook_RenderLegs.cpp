#include "Hook_RenderLegs.hpp"
#include <Core/Config/Variables.hpp>
#include <Launcher.hpp>

auto __fastcall Hook_RenderLegs( void* a1, void* a2, void* a3, void* a4, void* a5 ) -> void*
{
	if ( !GetCheatLauncher( )->CheatIsAlive( ) )
		return RenderLegs_o.fastcall<void*>( a1, a2, a3, a4, a5 );

	if ( C_GET( bool, Vars.bRemoveLegs ) )
		return nullptr;

	return RenderLegs_o.fastcall<void*>( a1, a2, a3, a4, a5 );
}
