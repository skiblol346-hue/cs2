#pragma once
#include <filesystem>
#include <windows.h>
#include <vector>

class CExplorerManager final
{
public:
	CExplorerManager( ) = default;
	~CExplorerManager( ) = default;

public:
	auto OnInit( ) -> bool;
	auto OnDestroy( ) -> void;

public:
	auto CreateFolder( const std::filesystem::path& path, const bool bRecreate=false ) -> bool;

public:
	auto GetExecutablePath( ) -> std::filesystem::path;

public:
	auto GetCoreDirectory( ) -> std::filesystem::path;
	auto GetConfigDirectory( ) -> std::filesystem::path;
	auto GetCustomModelsDirectory( ) -> std::filesystem::path;
	auto GetHitSoundsDirectory( ) -> std::filesystem::path;

public:
	auto IsAudioByExtension( const std::string& filename ) -> bool
	{
		std::string extension;
		size_t dotPos = filename.find_last_of( '.' );
		if ( dotPos != std::string::npos )
		{
			extension = filename.substr( dotPos + 1 );
		}

		// Convert to lowercase for case-insensitive comparison
		for ( char& c : extension )
		{
			c = std::tolower( c );
		}

		return ( extension == "mp3" || extension == "wav" || extension == "flac" || extension == "ogg" );
	}

private:
	std::filesystem::path m_CoreDirectory;
	std::filesystem::path m_ConfigDirectory;
	std::filesystem::path m_CustomModelsDirectory;
	std::filesystem::path m_HitSoundsDirectory;

private:
	std::vector<std::filesystem::path> m_vecCorePaths;
	std::vector<std::filesystem::path> m_vecTempPaths;

private:
	bool m_bInitialized = false;
};

auto GetExplorerManager( ) -> CExplorerManager*;