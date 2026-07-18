#pragma once
#include <Core/Memory/Memory.hpp>
#include <Core/Engine/FuncrionCS2.hpp>


class IEngineClient
{
public:
	auto GetMaxClients( ) -> int
	{
		return GetMemoryManager( )->CallVFunc<int, 34U>( this );
	}

	auto IsInGame( ) -> bool
	{
		//return GetMemoryManager( )->CallVFunc<bool, 35U>( this );
		return FunctionsCS2::FnGetIsInGame( this );
	}

	auto IsConnected( ) -> bool
	{
		//return GetMemoryManager( )->CallVFunc<bool, 36U>( this );
		return FunctionsCS2::FnGetIsConnected( this );
	}

	// return CBaseHandle index
	auto GetLocalPlayerIndex( ) -> int
	{
		int nIndex = -1;
		FunctionsCS2::FnGetLocalPlayerIndex( this, std::ref( nIndex ), 0 );
		return nIndex + 1;
	}
};
