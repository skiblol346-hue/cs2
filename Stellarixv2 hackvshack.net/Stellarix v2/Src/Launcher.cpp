#include "Launcher.hpp"
#include <Core/Common.hpp>
#include <Core/Memory/Memory.hpp>
#include <Core/Utils/Logger/Logger.hpp>
#include <Core/Utils/ExplorerManager/ExplorerManager.hpp>
#include <Core/Utils/PatternManager/PatternManager.hpp>
#include <Core/Interfaces/InterfaceManager.hpp>
#include <Core/Renderer/Renderer.hpp>
#include <Core/Hooks/HookManager.hpp>
#include <Core/StellarixClient.hpp>
#include <Core/Sdk/Schema/Schema.hpp>
#include <Core/Config/Config.hpp>
#include <Core/Engine/FuncrionCS2.hpp>
#include <Core/Engine/Convar/Convar.hpp>


static CCheatLauncher g_CCheatLauncher{};

auto CCheatLauncher::OnCheatMain( HINSTANCE hInstance ) -> void
{
	if ( !LOGGING::OnInit( ) )
		return;
	L_PRINT( LOG_INFO ) << _xor( "Logging System Initialized" );

	if ( !GetExplorerManager( )->OnInit( ) )
		return;
	L_PRINT( LOG_INFO ) << _xor( "Explorer Manager Initialized" );

	if ( !CONFIG::Setup( L_DEFAULT_CONFIG ) )
		return;
	L_PRINT( LOG_INFO ) << _xor( "CONFIG Initialized" );

	if ( !GetMemoryManager( )->OnInit( ) )
		return;
	L_PRINT( LOG_INFO ) << _xor( "Memory Manager Initialized" );

	if ( !GetPatternManager( )->OnInit( ) )
		return;
	L_PRINT( LOG_INFO ) << _xor( "Pattern Manager Initialized" );

	if ( !GetInterfaceManager( )->OnInit( ) )
		return;
	L_PRINT( LOG_INFO ) << _xor( "Interface Manager Initialized" );

	if ( !FunctionsCS2::OnInit( ) )
		return;
	L_PRINT( LOG_INFO ) << _xor( "CS2 Functions Initialized" );

	if ( !CONVAR::Setup( ) )
		return;
	L_PRINT( LOG_INFO ) << _xor( "CONVAR Initialized" );

	if ( !GetRenderer( )->OnInit( ) )
		return;
	L_PRINT( LOG_INFO ) << _xor( "Renderer Initialized" );
	std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );

	if ( !SCHEMA::OnInit( ) )
		return;
	L_PRINT( LOG_INFO ) << _xor( "Schema Initialized" );

	if ( !GetHookManager( )->OnInit( ) )
		return;
	L_PRINT( LOG_INFO ) << _xor( "Hook Manager Initialized" );

	if ( !GetStellarixClient( )->OnInit( ) )
		return;
	L_PRINT( LOG_INFO ) << _xor( "Stellarix Client Initialized" );

	// ReLoad Default Config -> Call SlellarixClient::OnLoadConfig
	//CONFIG::LoadFile( CONFIG:: );

	m_bInitialized = true;
	L_PRINT( LOG_INFO ) << LOGGING::SetColor( LOG_COLOR_FORE_GREEN ) << _xor( "====== Cheat Initialized ======" );
}

auto CCheatLauncher::OnDestroy( ) -> void
{
	if ( m_bDestroyed )
		return;
	m_bInitialized = false;
	m_bDestroyed = true;

	L_PRINT( LOG_INFO ) << LOGGING::SetColor( LOG_COLOR_FORE_MAGENTA ) << _xor( "Unload Cheat" );

	GetStellarixClient( )->OnDestroy( );
	std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
	GetHookManager( )->OnDestroy( );
	std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
	GetRenderer( )->OnDestroy( );
	std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
	GetInterfaceManager( )->OnDestroy( );
	std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
	GetMemoryManager( )->OnDestroy( );
	std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
	GetPatternManager( )->OnDestroy( );
	std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
	GetExplorerManager( )->OnDestroy( );
	std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
	LOGGING::OnDestroy( );
	std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
}

auto GetCheatLauncher( ) -> CCheatLauncher*
{
	return &g_CCheatLauncher;
}
