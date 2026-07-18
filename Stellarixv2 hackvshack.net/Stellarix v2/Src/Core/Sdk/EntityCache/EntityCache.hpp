#pragma once
#include "CacheTypes/CachedEntity.hpp"
#include "CacheTypes/CachedPlayer.hpp"
#include "CacheTypes/CachedWeapon.hpp"
#include <Core/Sdk/EntityData.hpp>
#include <functional>
#include <map>
#include <shared_mutex>
#include <utility>
#include <variant>


static const FNV1A_t uCCSPlayerController_Hash = FNV1A::HashConst( "CCSPlayerController" );
static const FNV1A_t uCMapInfo_Hash = FNV1A::HashConst( "CMapInfo" );
static const FNV1A_t uC_PlantedC4_Hash = FNV1A::HashConst( "C_PlantedC4" );


enum class ECachedEntityType : int
{
	kUnknown = -1,
	kPlayer,
	kWeapon,
	kGrenade,
	kCMapInfo,
	kC_PlantedC4,
	kC_CS2HudModelArms,
	kC_CS2HudModelWeapon
};

class CCachedEntities
{
public:
	using VariantCachedEntities = std::variant<CCachedEntity, CCachedPlayer, CCachedWeapon>;
	using MapCachedEntities_t = std::map<CBaseHandle, std::pair<VariantCachedEntities, ECachedEntityType>>;

public:
	CCachedEntities( ) = default;
	~CCachedEntities( )
	{
		OnDestroy( );
	}

public:
	auto OnInit( ) -> bool;
	auto OnDestroy( ) -> void;

public:
	auto GetEntityType( CBaseHandle hHandle ) -> ECachedEntityType
	{
		auto pC_BaseEntity = GetInterfaceManager( )->GetGameEntitySystem( )->Get<C_BaseEntity>( hHandle );
		if ( pC_BaseEntity )
		{
			const FNV1A_t uClassName = FNV1A::Hash( pC_BaseEntity->GetEntityClassName( ) );
			if ( uClassName == uCCSPlayerController_Hash )
				return ECachedEntityType::kPlayer;
			else if ( uClassName == uCMapInfo_Hash )
				return ECachedEntityType::kCMapInfo;
			else if ( uClassName == uC_PlantedC4_Hash )
				return ECachedEntityType::kC_PlantedC4;
			else if ( pC_BaseEntity->IsWeapon( ) )
				return ECachedEntityType::kWeapon;
		}

		return ECachedEntityType::kUnknown;
	}

public:
	auto AddEntity( CBaseHandle hHandle, ECachedEntityType nCachedEntityType ) -> void;
	auto AddEntity( CBaseHandle hHandle ) -> void;
	auto RemoveEntity( CBaseHandle hHandle ) -> void;
	auto ClearEntities( ) -> void
	{
		std::unique_lock<std::shared_mutex> lock( m_SharedLock );
		m_mapCachedEntities.clear( );
	}
	auto GetEntities( ) -> MapCachedEntities_t&
	{
		std::unique_lock<std::shared_mutex> lock( m_SharedLock );
		return m_mapCachedEntities;
	}

public:
	auto Lock( ) -> void
	{
		m_SharedLock.lock( );
	}
	auto UnLock( ) -> void
	{
		m_SharedLock.unlock( );
	}

private:
	std::shared_mutex m_SharedLock;
	MapCachedEntities_t m_mapCachedEntities{};
};


class CEntityCache
{
public:
	CEntityCache( ) = default;
	~CEntityCache( )
	{
		OnDestroy( );
	}

public:
	auto OnInit( ) -> bool;
	auto OnDestroy( ) -> void;

public:
	auto OnLevelInitHook( ) -> void;
	auto OnLevelShutdownHook( ) -> void;
	auto OnFrameStageNotifyHook( int nFrameStage ) -> void;
	auto OnCreateMoveClientHook( ) -> void;
	auto OnAddEntityHook( CEntityInstance* pInstance, CBaseHandle hHandle ) -> void;
	auto OnRemoveEntityHook( CEntityInstance* pInstance, CBaseHandle hHandle ) -> void;

public:
	auto OnPlayerBeginNewMatchEvent( ) -> void;
	auto OnGameEndEvent( ) -> void;

public:
	auto GetEntityType( CEntityInstance* pInstance ) -> ECachedEntityType;

public:
	auto ClearEntities( ) -> void;
	auto RefreshEntities( ) -> void;

public:
	std::atomic<CMapInfo*> pCMapInfo = nullptr;
	std::atomic<C_PlantedC4*> pC_PlantedC4 = nullptr;

public:
	CCachedEntities* VisualsCachedEntities = new CCachedEntities( );
	CCachedEntities* SpectatorsCachedEntities = new CCachedEntities( );
	CCachedEntities* LagCompCachedEntities = new CCachedEntities( );
};

auto GetEntityCache( ) -> CEntityCache*;