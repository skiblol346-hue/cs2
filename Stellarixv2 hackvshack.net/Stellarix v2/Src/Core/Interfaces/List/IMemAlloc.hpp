#pragma once
#include <cstddef>
// used: mem::CallVFunc
#include <Core/Memory/Memory.hpp>
#pragma warning(push)
#pragma warning(disable : 4191)


class IMemAlloc
{
public:
	void* Alloc( std::size_t nSize )
	{
		return GetMemoryManager( )->CallVFunc<void*, 1>( this, nSize );
	}

	void* ReAlloc( const void* pMemory, std::size_t nSize )
	{
		return GetMemoryManager( )->CallVFunc<void*, 2>( this, pMemory, nSize );
	}

	void Free( const void* pMemory )
	{
		return GetMemoryManager( )->CallVFunc<void, 3>( this, pMemory );
	}

	std::size_t GetSize( const void* pMemory )
	{
		return GetMemoryManager( )->CallVFunc<std::size_t, 21>( this, pMemory );
	}
};

#pragma warning(pop)