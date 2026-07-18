#include "ExplorerManager.hpp"
#include <Core/Common.hpp>
#include <Core/Utils/Logger/Logger.hpp>

static CExplorerManager g_CExplorerManager{ };

auto CExplorerManager::OnInit( ) -> bool
{
	m_vecCorePaths = {
		GetCoreDirectory( ),
		GetConfigDirectory( ),
		GetCustomModelsDirectory( ),
		GetHitSoundsDirectory( )
	};

	m_vecTempPaths = {

	};


	for ( auto path : m_vecCorePaths )
	{
		if ( !CreateFolder( path ) )
		{
			L_PRINT( LOG_ERROR ) << _xor( "Failed to create `CORE` folder: " ) << "`" << path.c_str( ) << "`";
			return false;
		}
	}

	for ( auto path : m_vecTempPaths )
	{
		if ( !CreateFolder( path ) )
		{
			L_PRINT( LOG_ERROR ) << _xor( "Failed to create `TEMP` folder: " ) << "`" << path.c_str( ) << "`";
			return false;
		}
	}


	m_bInitialized = true;
	return m_bInitialized;
}

auto CExplorerManager::OnDestroy( ) -> void
{
	if ( !m_bInitialized )
		return;

	for ( auto& path : m_vecTempPaths )
		std::filesystem::remove_all( path );
}

auto CExplorerManager::CreateFolder( const std::filesystem::path& path, const bool bRecreate ) -> bool
{
	if ( std::filesystem::exists( path ) && bRecreate )
		std::filesystem::remove_all( path );

	if ( !std::filesystem::exists( path ) )
		return std::filesystem::create_directory( path );

	return true;
}

auto CExplorerManager::GetExecutablePath( ) -> std::filesystem::path
{
	char szPath[ MAX_PATH ];
	if ( GetModuleFileNameA( nullptr, szPath, MAX_PATH ) == 0 )
		return "";

	return std::filesystem::path( szPath ).parent_path( );
}

auto CExplorerManager::GetCoreDirectory( ) -> std::filesystem::path
{
	if ( m_CoreDirectory.empty( ) )
	{
		auto diskName = std::filesystem::temp_directory_path( ).root_path( );
		m_CoreDirectory = diskName / std::filesystem::path( CHEAT_NAME );
	}

	CreateFolder( m_CoreDirectory );

	return m_CoreDirectory;
}

auto CExplorerManager::GetConfigDirectory( ) -> std::filesystem::path
{
	if ( m_ConfigDirectory.empty( ) )
		m_ConfigDirectory = GetCoreDirectory( ) / std::filesystem::path( CONFIG_DIRECTORY );

	CreateFolder( m_ConfigDirectory );

	return m_ConfigDirectory;
}

auto CExplorerManager::GetCustomModelsDirectory( ) -> std::filesystem::path
{
	if ( m_CustomModelsDirectory.empty( ) )
	{
		auto exePath = GetExecutablePath( );
		auto exePathParent1 = exePath.parent_path( );
		auto exePathParent2 = exePathParent1.parent_path( );
		auto csgoDir = ( exePathParent2.string( ) + "\\" + _xor( "csgo" ) );
		auto charactersDir = ( csgoDir + "\\" + _xor( "characters" ) );

		m_CustomModelsDirectory = std::filesystem::path( charactersDir.c_str( ) );
	}

	CreateFolder( m_CustomModelsDirectory );

	return m_CustomModelsDirectory;
}

auto CExplorerManager::GetHitSoundsDirectory( ) -> std::filesystem::path
{
	if ( m_HitSoundsDirectory.empty( ) )
		m_HitSoundsDirectory = GetCoreDirectory( ) / std::filesystem::path( HITSOUND_DIRECTORY );

	CreateFolder( m_HitSoundsDirectory );

	return m_HitSoundsDirectory;
}

auto GetExplorerManager( ) -> CExplorerManager*
{
	return &g_CExplorerManager;
}
