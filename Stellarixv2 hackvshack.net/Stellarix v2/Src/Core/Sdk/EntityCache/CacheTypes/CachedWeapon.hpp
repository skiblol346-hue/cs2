#pragma once
#include "CachedEntity.hpp"
#include <Core/Renderer/Renderer.hpp>


class CCachedWeapon : public CCachedEntity
{
public:
	CCachedWeapon( ) = default;
	CCachedWeapon( CBaseHandle Handle ) : CCachedEntity( Handle )
	{
	}
	~CCachedWeapon( ) = default;

public:
	auto OnInit( ) -> void override
	{
		m_bInitialized = GetCCSWeaponBaseVData( ) != nullptr;
	}
	auto OnUpdate( ) -> void override
	{
		if ( !IsValid( ) )
			return OnInit( );

		GetOwnerHandle( );
		m_flDistanceToLocalPlayer = LocalPlayerData.vecOrigin.IsZero( ) ? 0 : GetCSWeaponBase( )->GetOrigin( ).DistTo( LocalPlayerData.vecOrigin );
	}

public:
	auto Reset( ) -> void
	{
		m_pBaseEntity = nullptr;
		m_pCSWeaponBase = nullptr;
		m_pWeaponVData = nullptr;
		m_hOwnerHandle = CBaseHandle( );
		m_iWeaponType = -1;
		m_szIcon = nullptr;

		m_bInitialized = false;
	}

public:
	auto IsValid( ) -> bool
	{
		return m_bInitialized && GetBaseEntity( ) != nullptr && GetCSWeaponBase( ) != nullptr && GetCCSWeaponBaseVData( ) != nullptr;
	}
	auto IsGrenade( ) -> bool
	{
		return IsValid( ) && GetWeaponType( ) == WEAPONTYPE_GRENADE;
	}
	auto IsC4( ) -> bool
	{
		return IsValid( ) && GetWeaponType( ) == WEAPONTYPE_C4;
	}
	auto OnScreen( ) -> bool
	{
		return IsValid( ) && m_bOnScreen;
	}
	auto HasOwner( ) -> bool
	{
		return IsValid( ) && m_hOwnerHandle.IsValid( );
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
	auto GetCSWeaponBase( ) -> C_CSWeaponBase*
	{
		auto pBaseEntity = GetBaseEntity( );
		if ( pBaseEntity && pBaseEntity->GetRefEHandle( ).IsValid( ) )
			m_pCSWeaponBase = GetInterfaceManager( )->GetGameEntitySystem( )->Get<C_CSWeaponBase>( pBaseEntity->GetRefEHandle( ) );
		else
			m_pCSWeaponBase = nullptr;

		return m_pCSWeaponBase;
	}
	auto GetCCSWeaponBaseVData( ) -> CCSWeaponBaseVData*
	{
		auto pCSWeaponBase = GetCSWeaponBase( );
		if ( pCSWeaponBase )
			m_pWeaponVData = pCSWeaponBase->GetWeaponVData( );
		else
			m_pWeaponVData = nullptr;

		return m_pWeaponVData;
	}
	auto GetCCollisionProperty( ) -> CCollisionProperty*
	{
		auto pCSWeaponBase = GetCSWeaponBase( );
		if ( pCSWeaponBase )
			m_pCCollisionProperty = pCSWeaponBase->m_pCollision( );
		else
			m_pCCollisionProperty = nullptr;

		return m_pCCollisionProperty;
	}
	auto GetWeaponName( ) -> std::string
	{
		try
		{
			if ( m_szWeaponName.empty( ) )
			{
				const char* szWeaponName = GetCCSWeaponBaseVData( )->m_szName( );
				const char* szNameLength = strstr( szWeaponName, "weapon_" );

				m_szWeaponName = szNameLength ? szNameLength + strlen( "weapon_" ) : szWeaponName;
				std::transform( m_szWeaponName.begin( ), m_szWeaponName.end( ), m_szWeaponName.begin( ), ::tolower );
			}
		}
		catch ( ... )
		{
			m_szWeaponName = "";
		}

		return m_szWeaponName;
	}
	auto GetWeaponType( ) -> int
	{
		try
		{
			if ( m_iWeaponType == -1 )
				m_iWeaponType = GetCCSWeaponBaseVData( )->m_WeaponType( );
		}
		catch ( ... )
		{
			m_iWeaponType = -1;
		}

		return m_iWeaponType;
	}
	auto GetWeaponIcon( ) -> const char*
	{
		try
		{
			if ( !m_szIcon )
				m_szIcon = StellarixFonts::GunWeaponIcon( GetCCSWeaponBaseVData( )->m_szName( ) );
		}
		catch ( ... )
		{
			m_szIcon = "";
		}

		if ( !m_szIcon )
			m_szIcon = "";

		return m_szIcon;
	}
	auto GetOwnerHandle( ) -> CBaseHandle
	{
		m_hOwnerHandle = CBaseHandle( );

		try
		{
			auto pCollision = GetCCollisionProperty( );
			if ( pCollision && !( pCollision->m_usSolidFlags( ) & 4 ) )
			{
				auto hOwner = GetCSWeaponBase( )->m_hOwnerEntity( );
				if ( hOwner.IsValid( ) )
				{
					auto pBaseEntity = GetInterfaceManager( )->GetGameEntitySystem( )->Get( hOwner );
					if ( pBaseEntity )
						m_hOwnerHandle = hOwner;
				}
			}
		}
		catch ( ... )
		{

		}

		return m_hOwnerHandle;
	}
	auto GetDistanceToLocalPlayer( ) -> float
	{
		return m_flDistanceToLocalPlayer;
	}

public:
	auto CalculateBoundingBox( ) -> void override
	{
		m_bOnScreen = false;
		if ( !IsValid( ) )
			return;
		try
		{
			if ( !GetOwnerHandle( ).IsValid( ) )
				m_bOnScreen = GetBaseEntity( )->GetEntityBoundingBox( &vecEntityBoundingBox, true );
		}
		catch ( ... )
		{
			m_bOnScreen = false;
		}
	}

private:
	C_BaseEntity* m_pBaseEntity = nullptr;
	C_CSWeaponBase* m_pCSWeaponBase = nullptr;
	CCSWeaponBaseVData* m_pWeaponVData = nullptr;
	CCollisionProperty* m_pCCollisionProperty = nullptr;
	CBaseHandle m_hOwnerHandle = CBaseHandle( );
	int m_iWeaponType = -1;
	std::string m_szWeaponName = "";
	const char* m_szIcon = nullptr;
	float m_flDistanceToLocalPlayer = 0.f;

private:
	bool m_bOnScreen = false;

public:
	ImVec4 vecEntityBoundingBox = ImVec4( );

private:
	bool m_bInitialized = false;
};