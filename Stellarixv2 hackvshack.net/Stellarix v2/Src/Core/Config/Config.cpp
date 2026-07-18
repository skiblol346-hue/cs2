#include "Config.hpp"
#include "Variables.hpp"
#include <Core/Common.hpp>
#include <Core/Sdk/Datatypes/Color.hpp>
#include <Core/StellarixClient.hpp>
#include <Core/Utils/Logger/Logger.hpp>
#include <External/Extensions/Binary.hpp>
#include <fstream>
#include <json.hpp>
#include <shlobj.h>

// default configurations working path
static wchar_t wszConfigurationsPath[ MAX_PATH ];



#pragma region config_user_data_type

std::size_t CONFIG::UserDataType_t::GetSerializationSize( ) const
{
	std::size_t nTotalDataSize = 0U;

	for ( const UserDataMember_t& member : vecMembers )
		nTotalDataSize += sizeof( FNV1A_t[ 2 ] ) + member.nDataSize;

	return nTotalDataSize;
}

#pragma endregion

#pragma region config_variable_object

void CONFIG::VariableObject_t::SetStorage( const void* pValue )
{
	// check is available to store value in the local storage
	if ( this->nStorageSize <= sizeof( this->storage.uLocal ) )
	{
		CRT::MemorySet( &this->storage.uLocal, 0U, sizeof( this->storage.uLocal ) );
		CRT::MemoryCopy( &this->storage.uLocal, pValue, this->nStorageSize );
	}
	// otherwise use heap memory to store it
	else
	{

		CRT::MemorySet( this->storage.pHeap, 0U, this->nStorageSize );
		CRT::MemoryCopy( this->storage.pHeap, pValue, this->nStorageSize );
	}
}

std::size_t CONFIG::VariableObject_t::GetSerializationSize( ) const
{
	std::size_t nSerializationSize = this->nStorageSize;

	// denote a custom serialization size when it different from the storage size
	switch ( this->uTypeHash )
	{
		// lookup for array data type
	case FNV1A::HashConst( "bool[]" ):
	case FNV1A::HashConst( "int[]" ):
	case FNV1A::HashConst( "unsigned int[]" ):
	case FNV1A::HashConst( "float[]" ):
	case FNV1A::HashConst( "char[][]" ):
		// arrays also serialize their size
		nSerializationSize += sizeof( std::size_t );
		break;
		// lookup for user-defined data type
	default:
	{
		for ( const UserDataType_t& userType : vecUserTypes )
		{
			if ( userType.uTypeHash == this->uTypeHash )
			{
				nSerializationSize = sizeof( std::size_t ) + userType.GetSerializationSize( );
				break;
			}
		}
		break;
	}
	}

	return nSerializationSize;
}

#pragma endregion


bool CONFIG::Setup( const wchar_t* wszDefaultFileName )
{
	CRT::StringCopy( wszConfigurationsPath, ( GetExplorerManager( )->GetConfigDirectory( ) / std::filesystem::path( "" ) ).wstring( ).c_str( ) );

	// @note: define custom data types we want to serialize
	AddUserType( FNV1A::HashConst( "KeyBind_t" ),
				 {
					 UserDataMember_t{ FNV1A::HashConst( "uKey" ), FNV1A::HashConst( "unsigned int" ), &KeyBind_t::uKey },
					 UserDataMember_t{ FNV1A::HashConst( "nMode" ), FNV1A::HashConst( "int" ), &KeyBind_t::nMode }
				 } );

	// create default configuration
	if ( !CONFIG::CreateFile( wszDefaultFileName ) )
		return false;

	// store existing configurations list
	Refresh( );

	return true;
}


#pragma region config_main

void CONFIG::Refresh( )
{
	// clear and free previous stored file names
	vecFileNames.clear( );

	// make configuration files path filter
	wchar_t wszPathFilter[ MAX_PATH ];
	CRT::StringCat( CRT::StringCopy( wszPathFilter, wszConfigurationsPath ), _xor( L"*" L_CONFIG_EXTENSION ) );

	// iterate through all files with our filter
	WIN32_FIND_DATAW findData;
	if ( const HANDLE hFindFile = ::FindFirstFileW( wszPathFilter, &findData ); hFindFile != INVALID_HANDLE_VALUE )
	{
		do
		{
			vecFileNames.push_back( new wchar_t[ CRT::StringLength( findData.cFileName ) + 1U ] );
			CRT::StringCopy( vecFileNames.back( ), findData.cFileName );

			L_PRINT( LOG_INFO ) << _xor( "found configuration file: \"" ) << findData.cFileName << _xor( "\"" );
		}
		while ( ::FindNextFileW( hFindFile, &findData ) );

		::FindClose( hFindFile );
	}
}

void CONFIG::AddUserType( const FNV1A_t uTypeHash, const std::initializer_list<UserDataMember_t> vecUserMembers )
{
	if ( vecUserMembers.size( ) == 0U )
		return;

	UserDataType_t userDataType;
	userDataType.uTypeHash = uTypeHash;

	for ( const auto& userDataMember : vecUserMembers )
		userDataType.vecMembers.push_back( userDataMember );

	vecUserTypes.emplace_back( CRT::Move( userDataType ) );
}

bool CONFIG::SaveFileVariable( const std::size_t nFileIndex, const VariableObject_t& variable )
{
	const wchar_t* wszFileName = vecFileNames[ nFileIndex ];

	wchar_t wszFilePath[ MAX_PATH ];
	CRT::StringCat( CRT::StringCopy( wszFilePath, wszConfigurationsPath ), wszFileName );

#if defined(CS_CONFIGURATION_BINARY)
	if ( BIN::SaveVariable( wszFilePath, variable ) )
#elif defined(CS_CONFIGURATION_JSON)
	if ( JSON::SaveVariable( wszFilePath, variable ) )
#elif defined(CS_CONFIGURATION_TOML)
	if ( TOML::SaveVariable( wszFilePath, variable ) )
#endif
	{
		return true;
	}

	return false;
}

bool CONFIG::LoadFileVariable( const std::size_t nFileIndex, VariableObject_t& variable )
{
	const wchar_t* wszFileName = vecFileNames[ nFileIndex ];

	wchar_t wszFilePath[ MAX_PATH ];
	CRT::StringCat( CRT::StringCopy( wszFilePath, wszConfigurationsPath ), wszFileName );

#if defined(CS_CONFIGURATION_BINARY)
	if ( BIN::LoadVariable( wszFilePath, variable ) )
#elif defined(CS_CONFIGURATION_JSON)
	if ( JSON::LoadVariable( wszFilePath, variable ) )
#elif defined(CS_CONFIGURATION_TOML)
	if ( TOML::LoadVariable( wszFilePath, variable ) )
#endif
	{
		return true;
	}

	return false;
}

bool CONFIG::RemoveFileVariable( const std::size_t nFileIndex, const VariableObject_t& variable )
{
	const wchar_t* wszFileName = vecFileNames[ nFileIndex ];

	wchar_t wszFilePath[ MAX_PATH ];
	CRT::StringCat( CRT::StringCopy( wszFilePath, wszConfigurationsPath ), wszFileName );

#if defined(CS_CONFIGURATION_BINARY)
	if ( BIN::RemoveVariable( wszFilePath, variable ) )
#elif defined(CS_CONFIGURATION_JSON)
	if ( JSON::RemoveVariable( wszFilePath, variable ) )
#elif defined(CS_CONFIGURATION_TOML)
	if ( TOML::RemoveVariable( wszFilePath, variable ) )
#endif
	{
		return true;
	}

	return false;
}

bool CONFIG::CreateFile( const wchar_t* wszFileName )
{
	const wchar_t* wszFileExtension = CRT::StringCharR( wszFileName, L'.' );

	// get length of the given filename and strip out extension if there any
	const std::size_t nFileNameLength = ( wszFileExtension != nullptr ? wszFileExtension - wszFileName : CRT::StringLength( wszFileName ) );
	wchar_t* wszFullFileName = new wchar_t[ nFileNameLength + CRT::StringLength( L_CONFIG_EXTENSION ) + 1U ];

	// copy filename without extension
	wchar_t* wszFullFileNameEnd = CRT::StringCopyN( wszFullFileName, wszFileName, nFileNameLength );
	*wszFullFileNameEnd = L'\0';
	// append correct extension to the filename
	CRT::StringCat( wszFullFileNameEnd, _xor( L_CONFIG_EXTENSION ) );

	// add filename to the list
	vecFileNames.push_back( wszFullFileName );

	// create and save it by the index
	if ( SaveFile( vecFileNames.size( ) - 1U ) )
	{
		L_PRINT( LOG_INFO ) << _xor( "created configuration file: \"" ) << wszFullFileName << _xor( "\"" );
		return true;
	}

	L_PRINT( LOG_WARNING ) << _xor( "failed to create configuration file: \"" ) << wszFullFileName << _xor( "\"" );
	return false;
}

bool CONFIG::SaveFile( const std::size_t nFileIndex )
{
	const wchar_t* wszFileName = vecFileNames[ nFileIndex ];

	wchar_t wszFilePath[ MAX_PATH ];
	CRT::StringCat( CRT::StringCopy( wszFilePath, wszConfigurationsPath ), wszFileName );

#if defined(CS_CONFIGURATION_BINARY)
	if ( BIN::SaveFile( wszFilePath ) )
#elif defined(CS_CONFIGURATION_JSON)
	if ( JSON::SaveFile( wszFilePath ) )
#elif defined(CS_CONFIGURATION_TOML)
	if ( TOML::SaveFile( wszFilePath ) )
#endif
	{
		L_PRINT( LOG_INFO ) << _xor( "saved configuration file: \"" ) << wszFileName << _xor( "\"" );
		return true;
	}

	L_PRINT( LOG_WARNING ) << _xor( "failed to save configuration file: \"" ) << wszFileName << _xor( "\"" );
	return false;
}

bool CONFIG::LoadFile( const std::size_t nFileIndex )
{
	const wchar_t* wszFileName = vecFileNames[ nFileIndex ];

	wchar_t wszFilePath[ MAX_PATH ];
	CRT::StringCat( CRT::StringCopy( wszFilePath, wszConfigurationsPath ), wszFileName );

#if defined(CS_CONFIGURATION_BINARY)
	if ( BIN::LoadFile( wszFilePath ) )
#elif defined(CS_CONFIGURATION_JSON)
	if ( JSON::LoadFile( wszFilePath ) )
#elif defined(CS_CONFIGURATION_TOML)
	if ( TOML::LoadFile( wszFilePath ) )
#endif
	{
		std::wstring ws( wszFileName );
		szCurrentConfig = std::string( ws.begin( ), ws.end( ) );

		// Call Main Event
		GetStellarixClient( )->OnLoadConfig( );

		L_PRINT( LOG_INFO ) << _xor( "loaded configuration file: \"" ) << wszFileName << _xor( "\"" );
		return true;
	}

	L_PRINT( LOG_WARNING ) << _xor( "failed to load configuration file: \"" ) << wszFileName << _xor( "\"" );
	return false;
}

void CONFIG::RemoveFile( const std::size_t nFileIndex )
{
	const wchar_t* wszFileName = vecFileNames[ nFileIndex ];

	// unable to delete default config
	if ( CRT::StringCompare( wszFileName, _xor( L_DEFAULT_CONFIG L_CONFIG_EXTENSION ) ) == 0 )
	{
		L_PRINT( LOG_WARNING ) << _xor( "unable to remove default configuration file: \"" ) << wszFileName << _xor( "\"" );
		return;
	}

	wchar_t wszFilePath[ MAX_PATH ];
	CRT::StringCat( CRT::StringCopy( wszFilePath, wszConfigurationsPath ), wszFileName );

	if ( ::DeleteFileW( wszFilePath ) )
	{
		// erase and free filename from the list
		vecFileNames.erase( vecFileNames.cbegin( ) + nFileIndex );

		L_PRINT( LOG_INFO ) << _xor( "removed configuration file: \"" ) << wszFileName << _xor( "\"" );
	}
}

#pragma endregion


#pragma region config_get

std::size_t CONFIG::GetVariableIndex( const FNV1A_t uNameHash )
{
	for ( std::size_t i = 0U; i < vecVariables.size( ); i++ )
	{
		if ( vecVariables[ i ].uNameHash == uNameHash )
			return i;
	}

	return C_INVALID_VARIABLE;
}

#pragma endregion