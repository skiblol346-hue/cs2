#include "Hook_OnLevelInit.hpp"
#include <Core/StellarixClient.hpp>

auto Hook_OnLevelInit( void* pClientModeShared, const char* szNewMap ) -> void*
{
	GetStellarixClient( )->OnLevelInitHook( szNewMap );

	return OnLevelInit_o.fastcall<void*>( pClientModeShared, szNewMap );
}
