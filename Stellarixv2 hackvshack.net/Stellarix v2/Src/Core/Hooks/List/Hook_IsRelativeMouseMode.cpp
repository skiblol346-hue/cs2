#include "Hook_IsRelativeMouseMode.hpp"
#include <StellarixUI.hpp>
#include <Launcher.hpp>

auto Hook_IsRelativeMouseMode( void* pThisptr, bool bActive ) -> void*
{
	if ( !GetCheatLauncher( )->CheatIsAlive( ) )
		return IsRelativeMouseMode_o.fastcall<void*>( pThisptr, bActive );

	GetStellarixUI( )->bMainActive = bActive;

	if ( GetStellarixUI( )->bMainWindowOpened )
		return IsRelativeMouseMode_o.fastcall<void*>( pThisptr, false );

	return IsRelativeMouseMode_o.fastcall<void*>( pThisptr, bActive );
}
