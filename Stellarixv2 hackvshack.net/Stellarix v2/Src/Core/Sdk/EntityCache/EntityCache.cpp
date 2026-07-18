#include "EntityCache.hpp"
#include <algorithm>
#include <Core/Engine/Convar/Convar.hpp>
#include <Core/Interfaces/List/CGameEntitySystem.hpp>
#include <Core/Interfaces/List/IEngineCVar.hpp>
#include <Core/Utils/Logger/Logger.hpp>
#include <Core/Hooks/List/Hook_FrameStageNotify.hpp>


static CEntityCache g_CEntityCache{ };


auto CCachedEntities::OnInit( ) -> bool
{
	return true;
}

auto CCachedEntities::OnDestroy( ) -> void
{
	ClearEntities( );
}

auto CCachedEntities::AddEntity( CBaseHandle hHandle, ECachedEntityType nCachedEntityType ) -> void
{
	if ( !hHandle.IsValid( ) || nCachedEntityType == ECachedEntityType::kUnknown )
		return;

	std::unique_lock<std::shared_mutex> lock( m_SharedLock );
	switch ( nCachedEntityType )
	{
	case ECachedEntityType::kPlayer:
		m_mapCachedEntities.insert( std::make_pair( hHandle, std::make_pair( CCachedPlayer( hHandle ), nCachedEntityType ) ) );
		break;
	case ECachedEntityType::kWeapon:
		m_mapCachedEntities.insert( std::make_pair( hHandle, std::make_pair( CCachedWeapon( hHandle ), nCachedEntityType ) ) );
		break;
	case ECachedEntityType::kGrenade:
		break;
	case ECachedEntityType::kC_PlantedC4:
		m_mapCachedEntities.insert( std::make_pair( hHandle, std::make_pair( CCachedEntity( hHandle ), nCachedEntityType ) ) );
		break;
	default:
		break;
	}
}

auto CCachedEntities::AddEntity( CBaseHandle hHandle ) -> void
{
	AddEntity( hHandle, GetEntityType( hHandle ) );
}

auto CCachedEntities::RemoveEntity( CBaseHandle hHandle ) -> void
{
	std::unique_lock<std::shared_mutex> lock( m_SharedLock );
	const auto it = m_mapCachedEntities.find( hHandle );
	if ( it != m_mapCachedEntities.end( ) )
		m_mapCachedEntities.erase( hHandle.ToInt( ) );
}


auto CEntityCache::OnInit( ) -> bool
{
	RefreshEntities( );

	return true;
}

// Used Cached Lists
auto CEntityCache::OnDestroy( ) -> void
{
	ClearEntities( );

	delete[ ] VisualsCachedEntities;
	delete[ ] SpectatorsCachedEntities;
	delete[ ] LagCompCachedEntities;
}

auto CEntityCache::OnLevelInitHook( ) -> void
{
	ClearEntities( );
}

auto CEntityCache::OnLevelShutdownHook( ) -> void
{
	ClearEntities( );
}

auto CEntityCache::OnFrameStageNotifyHook( int nFrameStage ) -> void
{
	if ( nFrameStage == FRAME_NET_UPDATE_END )
	{
		auto& mapLagCompCachedEntities = LagCompCachedEntities->GetEntities( );
		if ( !mapLagCompCachedEntities.empty( ) )
		{
			const int iMaxRecordsTick = TIME_TO_TICKS( CONVAR::sv_maxunlag->value.fl );
			const float flLastValidSimTime = TIME_TO_TICKS( CONVAR::sv_maxunlag->value.fl );

			GetEntityCache( )->LagCompCachedEntities->Lock( );
			for ( auto& pairLagCompEntity : mapLagCompCachedEntities )
			{
				CBaseHandle hCachedEntityHandle = pairLagCompEntity.first;
				CCachedEntities::VariantCachedEntities& variantCachedEntity = pairLagCompEntity.second.first;
				ECachedEntityType nCachedEntityType = pairLagCompEntity.second.second;

				if ( !hCachedEntityHandle.IsValid( ) )
					continue;

				if ( nCachedEntityType != ECachedEntityType::kPlayer )
					continue;

				CCachedPlayer* pCachedPlayer = std::get_if<CCachedPlayer>( &variantCachedEntity );
				if ( !pCachedPlayer )
					continue;

				pCachedPlayer->OnInit( );
				pCachedPlayer->OnUpdateLagCompRecords( iMaxRecordsTick, flLastValidSimTime );
			}
			GetEntityCache( )->LagCompCachedEntities->UnLock( );
		}
	}
	
	if ( nFrameStage == FRAME_SIMULATE_END )
	{
		auto& mapVisualsCachedEntities = VisualsCachedEntities->GetEntities( );
		if ( !mapVisualsCachedEntities.empty( ) )
		{
			const int iMaxRecordsTick = TIME_TO_TICKS( CONVAR::sv_maxunlag->value.fl );
			const float flLastValidSimTime = TIME_TO_TICKS( CONVAR::sv_maxunlag->value.fl );

			VisualsCachedEntities->Lock( );
			for ( auto& pairLagCompEntity : mapVisualsCachedEntities )
			{
				CBaseHandle hCachedEntityHandle = pairLagCompEntity.first;
				CCachedEntities::VariantCachedEntities& variantCachedEntity = pairLagCompEntity.second.first;
				ECachedEntityType nCachedEntityType = pairLagCompEntity.second.second;

				if ( !hCachedEntityHandle.IsValid( ) )
					continue;

				if ( nCachedEntityType != ECachedEntityType::kPlayer )
					continue;

				CCachedPlayer* pCachedPlayer = std::get_if<CCachedPlayer>( &variantCachedEntity );
				if ( !pCachedPlayer )
					continue;

				pCachedPlayer->OnInit( );
				pCachedPlayer->OnUpdateLagCompRecords( iMaxRecordsTick, flLastValidSimTime );
			}
			VisualsCachedEntities->UnLock( );
		}
	}
}

auto CEntityCache::OnCreateMoveClientHook( ) -> void
{

}

// Used Cached Lists
auto CEntityCache::OnAddEntityHook( CEntityInstance* pInstance, CBaseHandle hHandle ) -> void
{
	ECachedEntityType eCachedEntityType = GetEntityType( pInstance );
	if ( eCachedEntityType == ECachedEntityType::kUnknown )
		return;

	VisualsCachedEntities->AddEntity( hHandle );
	SpectatorsCachedEntities->AddEntity( hHandle );
	LagCompCachedEntities->AddEntity( hHandle );

	switch ( eCachedEntityType )
	{
	case ECachedEntityType::kCMapInfo:
		pCMapInfo.store( GetInterfaceManager( )->GetGameEntitySystem( )->Get<CMapInfo>( hHandle ) );
		break;
	case ECachedEntityType::kC_PlantedC4:
		pC_PlantedC4.store( GetInterfaceManager( )->GetGameEntitySystem( )->Get<C_PlantedC4>( hHandle ) );
		break;
	default:
		break;
	}
}
// Used Cached Lists
auto CEntityCache::OnRemoveEntityHook( CEntityInstance* pInstance, CBaseHandle hHandle ) -> void
{
	ECachedEntityType eCachedEntityType = GetEntityType( pInstance );
	if ( eCachedEntityType == ECachedEntityType::kUnknown )
		return;

	VisualsCachedEntities->RemoveEntity( hHandle );
	SpectatorsCachedEntities->RemoveEntity( hHandle );
	LagCompCachedEntities->RemoveEntity( hHandle );

	switch ( eCachedEntityType )
	{
	case ECachedEntityType::kCMapInfo:
		if ( pCMapInfo.load( ) && hHandle == pCMapInfo.load( )->GetRefEHandle( ) )
			pCMapInfo.store( nullptr );
		break;
	case ECachedEntityType::kC_PlantedC4:
		if ( pC_PlantedC4.load( ) && hHandle == pC_PlantedC4.load( )->GetRefEHandle( ) )
			pC_PlantedC4.store( nullptr );
		break;
	default:
		break;
	}
}


auto CEntityCache::OnPlayerBeginNewMatchEvent( ) -> void
{
	RefreshEntities( );
}

auto CEntityCache::OnGameEndEvent( ) -> void
{
	ClearEntities( );
}


auto CEntityCache::GetEntityType( CEntityInstance* pInstance ) -> ECachedEntityType
{
	const FNV1A_t uClassName = FNV1A::Hash( pInstance->GetEntityClassName( ) );
	if ( uClassName == FNV1A::HashConst( "CCSPlayerController" ) )
		return ECachedEntityType::kPlayer;
	else if ( uClassName == FNV1A::HashConst( "CMapInfo" ) )
		return ECachedEntityType::kCMapInfo;
	else if ( uClassName == FNV1A::HashConst( "C_PlantedC4" ) )
		return ECachedEntityType::kC_PlantedC4;
	else if ( pInstance->IsWeapon( ) )
		return ECachedEntityType::kWeapon;

	return ECachedEntityType::kUnknown;
}

// Used Cached Lists
auto CEntityCache::ClearEntities( ) -> void
{
	VisualsCachedEntities->ClearEntities( );
	SpectatorsCachedEntities->ClearEntities( );
	LagCompCachedEntities->ClearEntities( );

	pCMapInfo.store( nullptr );
	pC_PlantedC4.store( nullptr );
}

auto CEntityCache::RefreshEntities( ) -> void
{
	ClearEntities( );

	for ( int nEntityIndex = 0; nEntityIndex < GetInterfaceManager( )->GetGameEntitySystem( )->GetHighestEntityIndex( ); nEntityIndex++ )
	{
		auto pBaseEntity = GetInterfaceManager( )->GetGameEntitySystem( )->Get( nEntityIndex );
		if ( !pBaseEntity ) continue;

		OnAddEntityHook( pBaseEntity, pBaseEntity->GetRefEHandle( ) );
	}
}

auto GetEntityCache( ) -> CEntityCache*
{
	return &g_CEntityCache;
}

