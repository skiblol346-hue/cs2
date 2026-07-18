#pragma once
#include <cstdint>
#include <Core/Memory/Memory.hpp>
#include <Core/Sdk/Datatypes/CBufferString.hpp>


struct ResourceBinding_t;
class CMaterial2;


class ResourceArray_t
{
public:
	uint64_t nCount;
	CMaterial2*** aResources;
	uint64_t pad_0010[ 3 ];
};

class CResourceSystem
{
public:
	void* QueryInterface( const char* szInterfaceName )
	{
		return GetMemoryManager( )->CallVFunc<void*, 2U>( this, szInterfaceName );
	}

	// 48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 41 8B E9            (resourcesystem.dll) 13.05.25 - 28.08.25
	// 48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 41 8B D9 49 8B F0   (resourcesystem.dll) 05.11.25
	void EnumMaterials( uint64_t iTypeHash, ResourceArray_t* pResult, uint8_t Flag )
	{
		GetMemoryManager( )->CallVFunc<void, 32>( this, iTypeHash, pResult, Flag );
	}

	// @IDA STR: " Resource \"%s\" was not precached but was loaded by a just in time blocking load.\n "
	void* PrecacheResource( const char* name )
	{
		// Create the buffer string with initial data
		CBufferString names( name );

		// Call the virtual function with the initialized buffer
		return GetMemoryManager( )->CallVFunc<void*, 41U>( this, &names, "" );
	}
};

class CResourceHandleUtils
{
public:
	void DeleteResource( const ResourceBinding_t* pBinding )
	{
		GetMemoryManager( )->CallVFunc<void, 2U>( this, pBinding );
	}
};
