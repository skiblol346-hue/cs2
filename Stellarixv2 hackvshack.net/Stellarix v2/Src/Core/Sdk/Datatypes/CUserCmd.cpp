#include "CUserCmd.hpp"

auto CUserCmd::GetUserCmdBySequence( CCSPlayerController* pController ) -> CUserCmd*
{
	if ( !pController )
		return nullptr;

	static auto get_command_index = reinterpret_cast< void* ( __fastcall* )( void*, int* ) >( GetMemoryManager( )->GetAbsoluteAddress( GetMemoryManager( )->FindPattern( CLIENT_DLL, "E8 ? ? ? ? 8B 8D ? ? ? ? 8D 51" ), 0x1, 0x0 ) );
	if ( !get_command_index )
		return nullptr;

	int index = 0;
	get_command_index( pController, &index );
	int command_index = index - 1;

	if ( command_index == -1 )
		command_index = -1;

	static auto get_command_array = reinterpret_cast< void* ( __fastcall* )( void*, int ) >( GetMemoryManager( )->GetAbsoluteAddress( GetMemoryManager( )->FindPattern( CLIENT_DLL, "E8 ? ? ? ? 48 8B CF 4C 8B F8" ), 0x1, 0x0 ) );
	static auto g_cmds = *reinterpret_cast< void** >( GetMemoryManager( )->ResolveRelativeAddress( GetMemoryManager( )->FindPattern( CLIENT_DLL, "48 8B 0D ? ? ? ? E8 ? ? ? ? 48 8B CF 4C 8B F8" ), 0x3, 0x7 ) );
	if ( !get_command_array || !g_cmds )
		return nullptr;

	auto pCommandArray = get_command_array( g_cmds, command_index );
	if ( !pCommandArray )
		return nullptr;

	auto nSequenceNumber = *reinterpret_cast< DWORD* >( ( uintptr_t )pCommandArray + 22952 );
	static auto get_user_cmd = reinterpret_cast< CUserCmd * ( __fastcall* )( void*, DWORD ) >( GetMemoryManager( )->GetAbsoluteAddress( GetMemoryManager( )->FindPattern( CLIENT_DLL, "E8 ? ? ? ? 48 8B 0D ? ? ? ? 45 33 E4 48 89 45" ), 0x1, 0x0 ) );
	if ( !get_user_cmd )
		return nullptr;

	auto user_cmd = get_user_cmd( pController, nSequenceNumber );
	return user_cmd;
}
