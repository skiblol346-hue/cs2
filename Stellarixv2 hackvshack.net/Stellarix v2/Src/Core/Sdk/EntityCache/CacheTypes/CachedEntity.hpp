#pragma once
#include <Core/Interfaces/List/CGameEntitySystem.hpp>
#include <Core/Sdk/EntityData.hpp>
#include <Core/Sdk/EntityHandle.hpp>
#include <Core/Sdk/Globals.hpp>
#include <memory>
#include <shared_mutex>


class CCachedEntity
{
public:
	CCachedEntity( ) = default;
	CCachedEntity( CBaseHandle handle ) :
		hHandle( handle )
	{
	}
	~CCachedEntity( ) = default;

public:
	bool operator==( const CCachedEntity& other ) const
	{
		return hHandle == other.hHandle;
	}
	bool operator==( CCachedEntity& other ) const
	{
		return hHandle == other.hHandle;
	}


public:
	virtual auto OnInit( ) -> void
	{
	}
	virtual auto OnUpdate( ) -> void
	{
	}

public:
	virtual auto CalculateBoundingBox( ) -> void
	{
	}

public:
	CBaseHandle hHandle = CBaseHandle( );


public:
	struct HashFunction
	{
		size_t operator()( const CCachedEntity& obj ) const
		{
			size_t xHash = std::hash<int>( )( obj.hHandle.GetEntryIndex( ) );
			return xHash;
		}
	};
};