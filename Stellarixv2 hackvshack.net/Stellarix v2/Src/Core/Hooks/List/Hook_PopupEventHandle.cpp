#include "Hook_PopupEventHandle.hpp"
#include <Core/Config/Variables.hpp>
#include <Core/Engine/FuncrionCS2.hpp>
#include <Fnv1a/Fnv1a.h>
#include <Launcher.hpp>

static const FNV1A_t uPopupAcceptMatch_Hash = FNV1A::HashConst( "popup_accept_match_found" );

auto __fastcall Hook_PopupEventHandle( void* a1, const char* szPopupName, void* a3, float flUnk ) -> void*
{
	auto result = PopupEventHandle_o.unsafe_call<void*>( a1, szPopupName, a3, flUnk );

	if ( !GetCheatLauncher( )->CheatIsAlive( ) )
		return result;

	if ( C_GET( bool, Vars.bMatchAutoAccept ) )
	{
		if ( FNV1A::Hash( szPopupName ) == uPopupAcceptMatch_Hash )
		{
			FunctionsCS2::FnSetPlayerIsReady( nullptr, "" );
			PopupEventHandle_o.unsafe_call<void*>( a1, _xor( "UIPanorama.mainmenu_press_GO" ), nullptr, 0.0f );
		}
	}

	return result;
}
