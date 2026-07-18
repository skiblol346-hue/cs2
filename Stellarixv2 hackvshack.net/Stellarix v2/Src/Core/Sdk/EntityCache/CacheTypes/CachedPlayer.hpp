#pragma once
#include "CachedEntity.hpp"
#include <Core/Config/Variables.hpp>
#include <Core/Interfaces/List/IGlobalVars.hpp>
#include <Core/Utils/Math/Math.hpp>
#include <Features/Rage/LagcompRecord.hpp>
#include <ImGui/imgui.h>
#include <ImGui/imgui_internal.h>
#include <vector>


class CCachedPlayer : public CCachedEntity
{
public:
	CCachedPlayer( ) = default;
	CCachedPlayer( CBaseHandle Handle ) : CCachedEntity( Handle )
	{
	}
	~CCachedPlayer( ) = default;

public:
	auto OnInit( ) -> void override
	{
		m_bInitialized = GetPlayerPawn( ) != nullptr && GetPlayerWeaponServices( ) != nullptr;
	}
	auto OnUpdate( ) -> void override
	{
		if ( !IsValid( ) )
			return OnInit( );

		if ( !IsAlive( ) )
			return;

		/*CSteamID SteamID( GetPlayerController( )->m_steamID( ) );
		auto Image = SteamFriends( )->GetSmallFriendAvatar( SteamID );*/

		//m_flDistanceToLocalPlayer = LocalPlayerData.vecOrigin.IsZero( ) ? 0 : GetPlayerPawn( )->GetOrigin( ).DistTo( LocalPlayerData.vecOrigin );
	}
	auto OnUpdateHealthAnimation( float flDeltaTime ) -> void
	{
		if ( !IsAlive( ) )
			return;

		int iCurrentHealth = GetCurrentHealth( );
		if ( iCurrentHealth == static_cast< int >( m_flAnimatedHealth ) )
			return;

		if ( m_flAnimatedHealth <= 0 )
			m_flAnimatedHealth = iCurrentHealth;

		m_flAnimatedHealth = ImLerp<float>( m_flAnimatedHealth, static_cast< float >( iCurrentHealth ), C_GET( float, Vars.flPlayerEspHealthBarAnimSpeed ) * flDeltaTime );

		/*int iCurrentHealth = GetCurrentHealth( );
		if ( iCurrentHealth != ( int )GetAnimatedHealth( ) )
		{
			if ( GetAnimatedHealth( ) <= 0 )
				m_flAnimatedHealth = iCurrentHealth;

			if ( std::abs( static_cast< float >( iCurrentHealth ) - GetAnimatedHealth( ) ) <= 2.f )
				m_flAnimatedHealth = iCurrentHealth;
			else if ( iCurrentHealth > GetAnimatedHealth( ) )
			{
				m_flAnimatedHealth += C_GET( float, Vars.flPlayerEspHealthBarAnimSpeed ) * flDeltaTime;
				m_flAnimatedHealth = MATH::Min<float>( m_flAnimatedHealth, iCurrentHealth );
			}
			else if ( iCurrentHealth < GetAnimatedHealth( ) )
			{
				m_flAnimatedHealth -= C_GET( float, Vars.flPlayerEspHealthBarAnimSpeed ) * flDeltaTime;
				m_flAnimatedHealth = MATH::Max<float>( m_flAnimatedHealth, iCurrentHealth );
			}
		}*/
	}
	auto OnUpdateLagCompRecords( const int iMaxRecordsTick, const float flLastValidSimTime ) -> void
	{
		if ( !IsAlive( ) )
			return vecLagCompRecords.clear( );

		CLagCompRecord LagCompRecord = CLagCompRecord( GetPlayerPawn( ) );
		if ( !LagCompRecord.bValidRecord )
			return;

		vecLagCompRecords.insert( vecLagCompRecords.begin( ), LagCompRecord );
		if ( vecLagCompRecords.size( ) > iMaxRecordsTick )
			vecLagCompRecords.resize( iMaxRecordsTick );
	}

public:
	auto Reset( ) -> void
	{
		m_pBaseEntity = nullptr;
		m_pController = nullptr;
		m_pPawn = nullptr;
		m_pWeaponServices = nullptr;
		m_hWeaponHandle = CBaseHandle( );
		m_szPlayerName = "";

		m_bInitialized = false;
	}

public:
	auto IsValid( ) -> bool
	{
		m_bInitialized = GetBaseEntity( ) != nullptr && GetPlayerController( ) != nullptr && GetPlayerPawn( ) != nullptr;
		return m_bInitialized;
	}
	auto IsAlive( ) -> bool
	{
		if ( !IsValid( ) )
			return false;

		return GetPlayerController( )->m_bPawnIsAlive( );
	}
	auto IsEnemy( ) -> bool
	{
		try
		{
			return GetPlayerPawn( )->IsEnemy( nullptr );
		}
		catch ( ... )
		{
			return false;
		}
	}
	auto OnScreen( ) -> bool
	{
		return m_bOnScreen;
	}
	auto HasHelmet( ) -> bool
	{
		try
		{
			return GetPlayerController( )->m_bPawnHasHelmet( );
		}
		catch ( ... )
		{

		}
		return false;
	}
	auto HasDefuser( ) -> bool
	{
		try
		{
			return GetPlayerController( )->m_bPawnHasDefuser( );
		}
		catch ( ... )
		{

		}
		return false;
	}

public:
	auto GetBaseEntity( bool bReSearch = false ) -> C_BaseEntity*
	{
		if ( !m_pBaseEntity || bReSearch )
		{
			m_pBaseEntity = GetInterfaceManager( )->GetGameEntitySystem( )->Get( hHandle );
			if ( !m_pBaseEntity )
				m_pBaseEntity = nullptr;
		}

		return m_pBaseEntity;
	}
	auto GetPlayerController( ) -> CCSPlayerController*
	{
		if ( !m_pController )
		{
			auto pBaseEntity = GetBaseEntity( );
			if ( pBaseEntity )
				m_pController = reinterpret_cast< CCSPlayerController* >( pBaseEntity );
			else
				m_pController = nullptr;
		}
		return m_pController;
	}
	auto GetPlayerPawn( ) -> C_CSPlayerPawn*
	{
		if ( !m_pPawn )
			m_pPawn = C_CSPlayerPawn::GetPawnFromController( GetPlayerController( ) );

		return m_pPawn;
	}
	auto GetPlayerWeaponServices( ) -> CCSPlayer_WeaponServices*
	{
		if ( !m_pWeaponServices )
		{
			auto pPawn = GetPlayerPawn( );
			if ( pPawn )
				m_pWeaponServices = pPawn->m_pWeaponServices( );
			else
				m_pWeaponServices = nullptr;
		}
		return m_pWeaponServices;
	}
	auto GetPlayerObserverServices( ) -> CPlayer_ObserverServices*
	{
		if ( !m_pObserverServices )
		{
			auto pPawn = GetPlayerPawn( );
			if ( pPawn )
				m_pObserverServices = pPawn->m_pObserverServices( );
			else
				m_pObserverServices = nullptr;
		}
		return m_pObserverServices;
	}
	auto GetPlayerName( ) -> std::string
	{
		if ( m_szPlayerName.empty( ) )
		{
			try
			{
				m_szPlayerName = std::string( GetPlayerController( )->m_sSanitizedPlayerName( ) );
			}
			catch ( ... )
			{

			}
		}

		return m_szPlayerName;
	}
	auto GetPlayerPing( ) -> int
	{
		try
		{
			return GetPlayerController( )->m_iPing( );
		}
		catch ( ... )
		{
			return 0;
		}
	}
	auto GetPlayerZoom( ) -> int
	{
		try
		{
			return GetPlayerPawn( )->m_bIsScoped( );
		}
		catch ( ... )
		{
			return false;
		}
	}
	auto GetPlayerTeamId( ) -> int
	{
		try
		{
			return GetPlayerPawn( )->m_iTeamNum( );
		}
		catch ( ... )
		{
			return -1;
		}

	}
	auto GetDistanceToLocalPlayer( ) -> float
	{
		return m_flDistanceToLocalPlayer;
	}
	auto GetLastValidRecord( ) -> CLagCompRecord
	{
		if ( vecLagCompRecords.empty( ) )
			return CLagCompRecord( );

		std::vector<CLagCompRecord> reversedVec( vecLagCompRecords.size( ) );
		std::reverse_copy( vecLagCompRecords.begin( ), vecLagCompRecords.end( ), reversedVec.begin( ) );

		for ( CLagCompRecord& LagRecord : reversedVec )
		{
			if ( LagRecord.bValidRecord )
				return LagRecord;
		}

		return CLagCompRecord( );
	}
	auto GetFirstValidRecord( ) -> CLagCompRecord
	{
		if ( vecLagCompRecords.empty( ) )
			return CLagCompRecord( );

		for ( CLagCompRecord& LagRecord : vecLagCompRecords )
		{
			if ( LagRecord.bValidRecord )
				return LagRecord;
		}

		return CLagCompRecord( );
	}

public:
	auto GetCurrentHealth( ) -> int
	{
		try
		{
			return GetPlayerPawn( )->m_iHealth( );
		}
		catch ( ... )
		{
			return 100;
		}
	}
	auto GetAnimatedHealth( ) -> float
	{
		return m_flAnimatedHealth;
	}
	auto GetMaxHealth( ) -> int
	{
		try
		{
			return MATH::Max<int>( GetPlayerPawn( )->m_iMaxHealth( ), GetCurrentHealth( ) );
		}
		catch ( ... )
		{
			return 100;
		}
	}

public:
	auto CalculateBoundingBox( ) -> void override
	{
		m_bOnScreen = GetPlayerPawn( )->GetEntityBoundingBox( &vecEntityBoundingBox );
	}

private:
	C_BaseEntity* m_pBaseEntity = nullptr;
	CCSPlayerController* m_pController = nullptr;
	C_CSPlayerPawn* m_pPawn = nullptr;
	CCSPlayer_WeaponServices* m_pWeaponServices = nullptr;
	CPlayer_ObserverServices* m_pObserverServices = nullptr;
	CBaseHandle m_hWeaponHandle = CBaseHandle( );
	std::string m_szPlayerName = "";

public:
	std::vector<CLagCompRecord> vecLagCompRecords{ };

private:
	float m_flDistanceToLocalPlayer = 0.f;

private:
	float m_flAnimatedHealth = 0.f;
	bool m_bOnScreen = false;

public:
	ImVec4 vecEntityBoundingBox = ImVec4( );

private:
	bool m_bInitialized = false;
};