#include "Hook_OnLevelShutdown.hpp"
#include <Core/StellarixClient.hpp>

auto Hook_OnLevelShutdown( ) -> void*
{
	GetStellarixClient( )->OnLevelShutdownHook( );

	return OnLevelShutdown_o.fastcall<void*>( );
}
