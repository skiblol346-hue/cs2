#include "Hook_SetModel.hpp"
#include <Core/StellarixClient.hpp>

auto __fastcall Hook_SetModel( void* rcx, char* szModel ) -> void
{
	return SetModel_o.fastcall<void>( rcx, szModel );
}
