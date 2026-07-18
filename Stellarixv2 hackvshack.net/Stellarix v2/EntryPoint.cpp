#include <windows.h>
#include <Launcher.hpp>

extern "C" BOOL WINAPI _CRT_INIT( HMODULE hModule, DWORD dwReason, LPVOID lpReserved );

BOOL APIENTRY CoreEntryPoint( HMODULE hModule, DWORD dwReason, LPVOID lpReserved )
{
	DisableThreadLibraryCalls( hModule );
	if ( !_CRT_INIT( hModule, dwReason, lpReserved ) )
		return FALSE;

	// Set Priority CS2
	auto pCS2Process = GetCurrentProcess( );
	SetPriorityClass( pCS2Process, HIGH_PRIORITY_CLASS );

	switch ( dwReason )
	{
	case DLL_PROCESS_ATTACH:
		GetCheatLauncher( )->OnCheatMain( reinterpret_cast< HINSTANCE >( hModule ) );
		break;

	case DLL_PROCESS_DETACH:
		GetCheatLauncher( )->OnDestroy( );
		break;
	}

	return TRUE;
}