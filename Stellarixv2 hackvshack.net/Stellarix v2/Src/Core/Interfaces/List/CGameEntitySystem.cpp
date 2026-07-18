#include "CGameEntitySystem.hpp"

auto CGameEntitySystem::GetLocalPlayerController( ) -> CCSPlayerController*
{
	return GetInterfaceManager( )->GetGameEntitySystem( )->Get<CCSPlayerController>( GetInterfaceManager( )->GetEngineClient( )->GetLocalPlayerIndex( ) );
	//return FunctionsCS2::FnGetLocalPlayerController( -1 );
}
