#include "Hook_MouseInputEnabled.hpp"
#include <StellarixUI.hpp>
#include <Launcher.hpp>

auto __fastcall Hook_MouseInputEnabled( void* pThisptr ) -> bool
{
    if ( !GetCheatLauncher( )->CheatIsAlive( ) )
        return MouseInputEnabled_o.fastcall<bool>( pThisptr );

    return GetStellarixUI( )->bMainWindowOpened ? false : MouseInputEnabled_o.call<bool>( pThisptr );
}
