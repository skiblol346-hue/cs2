#pragma once
#include "Const.hpp"
#include "EntityHandle.hpp"
#include <cmath>
#include <Core/Common.hpp>
#include <Core/Engine/FuncrionCS2.hpp>
#include <Core/Interfaces/List/CSchemaSystem.hpp>
#include <Core/Sdk/Datatypes/Transform.hpp>
#include <Core/Sdk/Datatypes/Vector.hpp>
#include <Core/Sdk/Schema/Schema.hpp>
#include <ImGui/imgui.h>


class CFiringModeFloat
{
public:
	float flValue[ 2 ];
};

class CBasePlayerWeaponVData
{
public:
	CS_CLASS_NO_INITIALIZER( CBasePlayerWeaponVData );

public:
	SCHEMA_ADD_FIELD( bool, m_bIsFullAuto, "CBasePlayerWeaponVData->m_bIsFullAuto" );
	SCHEMA_ADD_FIELD( std::int32_t, m_iMaxClip1, "CBasePlayerWeaponVData->m_iMaxClip1" );
};

class CCSWeaponBaseVData : public CBasePlayerWeaponVData
{
public:
	CS_CLASS_NO_INITIALIZER( CCSWeaponBaseVData );

public:
	SCHEMA_ADD_FIELD( std::int32_t, m_WeaponType, "CCSWeaponBaseVData->m_WeaponType" );
	SCHEMA_ADD_FIELD( float, m_flCycleTime, "CCSWeaponBaseVData->m_flCycleTime" );
	SCHEMA_ADD_FIELD( float, m_flRange, "CCSWeaponBaseVData->m_flRange" );
	SCHEMA_ADD_FIELD( float, m_flRangeModifier, "CCSWeaponBaseVData->m_flRangeModifier" );
	SCHEMA_ADD_FIELD( std::int32_t, m_nDamage, "CCSWeaponBaseVData->m_nDamage" );
	SCHEMA_ADD_FIELD( float, m_flPenetration, "CCSWeaponBaseVData->m_flPenetration" );
	SCHEMA_ADD_FIELD( float, m_flHeadshotMultiplier, "CCSWeaponBaseVData->m_flHeadshotMultiplier" );
	SCHEMA_ADD_FIELD( int, m_nNumBullets, "CCSWeaponBaseVData->m_nNumBullets" );
	SCHEMA_ADD_FIELD( CFiringModeFloat, m_flMaxSpeed, "CCSWeaponBaseVData->m_flMaxSpeed" );

public:
	SCHEMA_ADD_FIELD( const char*, m_szName, "CCSWeaponBaseVData->m_szName" );
	SCHEMA_ADD_FIELD( float, m_flArmorRatio, "CCSWeaponBaseVData->m_flArmorRatio" );
};



using GameTime_t = std::float_t;
using GameTick_t = std::int32_t;

class CEntityInstance;
class CUserCmd;

using SurroundingBoundsType_t = std::int32_t;
class CCollisionProperty
{
public:
	CS_CLASS_NO_INITIALIZER( CCollisionProperty );

public:
	auto CollisionMask( ) -> std::uint16_t
	{
		return *reinterpret_cast< std::uint16_t* >( reinterpret_cast< std::uintptr_t >( this ) + 0x38 );
	}

public:
	SCHEMA_ADD_FIELD( Vector_t, m_vecMins, "CCollisionProperty->m_vecMins" );
	SCHEMA_ADD_FIELD( Vector_t, m_vecMaxs, "CCollisionProperty->m_vecMaxs" );

public:
	SCHEMA_ADD_FIELD( std::uint8_t, m_usSolidFlags, "CCollisionProperty->m_usSolidFlags" );
	SCHEMA_ADD_FIELD( std::uint8_t, m_CollisionGroup, "CCollisionProperty->m_CollisionGroup" );
	SCHEMA_ADD_FIELD( SurroundingBoundsType_t, m_nSurroundType, "CCollisionProperty->m_nSurroundType" );
};

class CEntityIdentity
{
public:
	CS_CLASS_NO_INITIALIZER( CEntityIdentity );

public:
	// @note: handle index is not entity index
	SCHEMA_ADD_OFFSET( CBaseHandle, GetIndex, 0x10 );

public:
	SCHEMA_ADD_FIELD( const char*, m_name, "CEntityIdentity->m_name" );
	SCHEMA_ADD_FIELD( const char*, m_designerName, "CEntityIdentity->m_designerName" );
	SCHEMA_ADD_FIELD( std::uint32_t, m_flags, "CEntityIdentity->m_flags" );

public:
	auto IsValid( ) -> bool
	{
		return GetIndex( ).IsValid( );
	}

	auto GetEntryIndex( ) -> int
	{
		if ( !IsValid( ) )
			return ENT_ENTRY_MASK;

		return GetIndex( ).ToInt( ) & ENT_ENTRY_MASK;
	}

	auto GetSerialNumber( ) -> int
	{
		if ( !IsValid( ) )
			return -1;

		return GetIndex( ).ToInt( ) >> NUM_SERIAL_NUM_SHIFT_BITS;
	}

	CEntityInstance* pInstance; // 0x00
};

class CEntityInstance
{
public:
	CS_CLASS_NO_INITIALIZER( CEntityInstance );

public:
	auto GetSchemaClassInfo( SchemaClassInfoData_t** pReturn ) -> void
	{
		return GetMemoryManager( )->CallVFunc<void, 42U>( this, pReturn );
	}

	auto GetSchemaNoCrash( ) -> SchemaClassInfoData_t*
	{
		SchemaClassInfoData_t* pClassInfo = nullptr;
		GetSchemaClassInfo( &pClassInfo );
		if ( !pClassInfo )
			return nullptr;

		return pClassInfo;
	}

	auto GetEntityClassName( ) -> const char*
	{
		SchemaClassInfoData_t* pClassInfo = GetSchemaNoCrash( );
		if ( !pClassInfo )
			return "";

		return pClassInfo->szName;
	}

	[[nodiscard]] auto GetRefEHandle( ) -> CBaseHandle
	{
		CEntityIdentity* pIdentity = m_pEntity( );
		if ( !pIdentity )
			return CBaseHandle( );

		return CBaseHandle( pIdentity->GetEntryIndex( ), pIdentity->GetSerialNumber( ) - ( pIdentity->m_flags( ) & 1 ) );
	}

public:
	bool IsWeapon( )
	{
		auto pCEntityIdentity = m_pEntity( );
		if ( !pCEntityIdentity )
			return false;

		auto szDesingerName = pCEntityIdentity->m_designerName( );
		if ( szDesingerName && strstr( szDesingerName, _xor( "weapon_" ) ) )
			return true;

		return false;
	}

public:
	SCHEMA_ADD_FIELD( CEntityIdentity*, m_pEntity, "CEntityInstance->m_pEntity" );
	SCHEMA_ADD_FIELD( bool, m_bVisibleinPVS, "CEntityInstance->m_bVisibleinPVS" );
};


class CSkeletonInstance;
class CTransform;

class CGameSceneNode
{
public:
	CS_CLASS_NO_INITIALIZER( CGameSceneNode );

public:
	SCHEMA_ADD_FIELD( CTransform, m_nodeToWorld, "CGameSceneNode->m_nodeToWorld" );
	SCHEMA_ADD_FIELD( CEntityInstance*, m_pOwner, "CGameSceneNode->m_pOwner" );

public:
	SCHEMA_ADD_FIELD( Vector_t, m_vecOrigin, "CGameSceneNode->m_vecOrigin" );
	SCHEMA_ADD_FIELD( Vector_t, m_vecAbsOrigin, "CGameSceneNode->m_vecAbsOrigin" );
	SCHEMA_ADD_FIELD( Vector_t, m_vRenderOrigin, "CGameSceneNode->m_vRenderOrigin" );

	/*public:
		SCHEMA_ADD_FIELD( QAngle_t, m_angRotation, "CGameSceneNode->m_angRotation" );
		SCHEMA_ADD_FIELD( QAngle_t, m_angAbsRotation, "CGameSceneNode->m_angAbsRotation" );*/

public:
	SCHEMA_ADD_FIELD( bool, m_bDormant, "CGameSceneNode->m_bDormant" );

public:
	auto GetSkeletonInstance( ) -> CSkeletonInstance*
	{
		return GetMemoryManager( )->CallVFunc<CSkeletonInstance*, 8U>( this );
	}
};


class CModel;
struct BoneData_t
{
	Vector_t Location;
	float Scale;
	Quaternion_t Rotation;
};

class CModelState
{
private:
	MEM_PAD( 0x80 );
public:
	BoneData_t* pBoneData;
//private:
//	MEM_PAD( 0x18 );
//public:
//	CStrongHandle<CModel> ModelHandle;
};

class CSkeletonInstance : public CGameSceneNode
{
private:
	MEM_PAD( 0x1EC ); //0x0000
public:
	int nBoneCount; //0x01CC
private:
	MEM_PAD( 0x18 ); //0x01D0
public:
	int nMask; //0x01E8
private:
	MEM_PAD( 0x4 ); //0x01EC
public:
	Matrix2x4_t* pBoneCache; //0x01F0
	//CBoneData* pBoneCache; //0x01F0

public:
	SCHEMA_ADD_FIELD( CModelState, m_modelState, "CSkeletonInstance->m_modelState" );
	SCHEMA_ADD_FIELD( std::uint8_t, m_nHitboxSet, "CSkeletonInstance->m_nHitboxSet" );
};


class C_BaseEntity : public CEntityInstance
{
public:
	CS_CLASS_NO_INITIALIZER( C_BaseEntity );

public:
	auto IsEnemy( C_BaseEntity* pOther ) -> bool;

public:
	SCHEMA_ADD_FIELD( std::uint64_t, m_nSubclassID, "C_BaseEntity->m_nSubclassID" );

public:
	SCHEMA_ADD_FIELD( CGameSceneNode*, m_pGameSceneNode, "C_BaseEntity->m_pGameSceneNode" );
	SCHEMA_ADD_FIELD( CCollisionProperty*, m_pCollision, "C_BaseEntity->m_pCollision" );
	SCHEMA_ADD_FIELD( std::uint8_t, m_iTeamNum, "C_BaseEntity->m_iTeamNum" );
	SCHEMA_ADD_FIELD( CBaseHandle, m_hOwnerEntity, "C_BaseEntity->m_hOwnerEntity" );

public:
	SCHEMA_ADD_FIELD( Vector_t, m_vecVelocity, "C_BaseEntity->m_vecVelocity" );
	SCHEMA_ADD_FIELD( Vector_t, m_vecBaseVelocity, "C_BaseEntity->m_vecBaseVelocity" );
	SCHEMA_ADD_FIELD( Vector_t, m_vecAbsVelocity, "C_BaseEntity->m_vecAbsVelocity" );

public:
	SCHEMA_ADD_FIELD( bool, m_bTakesDamage, "C_BaseEntity->m_bTakesDamage" );
	SCHEMA_ADD_FIELD( std::uint32_t, m_fFlags, "C_BaseEntity->m_fFlags" );
	SCHEMA_ADD_FIELD( std::int32_t, m_iEFlags, "C_BaseEntity->m_iEFlags" );
	SCHEMA_ADD_FIELD( std::uint8_t, m_nActualMoveType, "C_BaseEntity->m_nActualMoveType" ); // m_nActualMoveType returns CSGO style movetype, m_nMoveType returns bitwise shifted move type
	SCHEMA_ADD_FIELD( std::uint8_t, m_lifeState, "C_BaseEntity->m_lifeState" );
	SCHEMA_ADD_FIELD( std::int32_t, m_iHealth, "C_BaseEntity->m_iHealth" );
	SCHEMA_ADD_FIELD( std::int32_t, m_iMaxHealth, "C_BaseEntity->m_iMaxHealth" );
	SCHEMA_ADD_FIELD( std::float_t, m_flWaterLevel, "C_BaseEntity->m_flWaterLevel" );
	SCHEMA_ADD_FIELD( std::float_t, m_flSimulationTime, "C_BaseEntity->m_flSimulationTime" );

public:
	SCHEMA_ADD_FIELD_OFFSET( void*, GetVData, "C_BaseEntity->m_nSubclassID", 0x8 );

public:
	auto IsBasePlayerController( ) -> bool
	{
		SchemaClassInfoData_t* pClassInfo;
		GetSchemaClassInfo( &pClassInfo );
		if ( !pClassInfo )
			return false;

		return FNV1A::Hash( pClassInfo->szName ) == FNV1A::HashConst( "CCSPlayerController" );
	}
	auto IsBasePlayerWeapon( ) -> bool
	{
		static SchemaClassInfoData_t* pWeaponBaseClass = nullptr;
		if ( !pWeaponBaseClass )
			GetInterfaceManager( )->GetSchemaSystem( )->FindTypeScopeForModule( _xor( "client.dll" ) )->FindDeclaredClass( &pWeaponBaseClass, _xor( "C_CSWeaponBase" ) );

		SchemaClassInfoData_t* pClassInfo;
		GetSchemaClassInfo( &pClassInfo );
		if ( !pClassInfo )
			return false;

		return pClassInfo->InheritsFrom( pWeaponBaseClass );
	}

public:
	auto GetOrigin( ) -> Vector_t
	{
		if ( auto pGameSceneNode = m_pGameSceneNode( ); pGameSceneNode )
			return pGameSceneNode->m_vecAbsOrigin( );

		return Vector_t( );
	}
	auto GetHitboxMinsMaxs( Vector_t& mins, Vector_t& maxs ) -> bool;
	auto GetEntityBoundingBox( ImVec4* pVecOut, bool bComputeSurroundingBox = false ) -> bool;
};

class C_BaseModelEntity : public C_BaseEntity
{
public:
	CS_CLASS_NO_INITIALIZER( C_BaseModelEntity );

public:
	SCHEMA_ADD_FIELD( CCollisionProperty*, m_Collision, "C_BaseModelEntity->m_Collision" );
	SCHEMA_ADD_FIELD( Vector_t, m_vecViewOffset, "C_BaseModelEntity->m_vecViewOffset" );
	SCHEMA_ADD_FIELD( CBaseHandle, m_hOldMoveParent, "C_BaseModelEntity->m_hOldMoveParent" );
	SCHEMA_ADD_FIELD( std::float_t, m_flAnimTime, "C_BaseModelEntity->m_flAnimTime" );

public:
	auto SetModel( const char* szName ) -> void;
};

class CPlayer_WeaponServices
{
public:
	CS_CLASS_NO_INITIALIZER( CPlayer_WeaponServices );

public:
	SCHEMA_ADD_FIELD( CBaseHandle, m_hActiveWeapon, "CPlayer_WeaponServices->m_hActiveWeapon" );                   // CHandle<C_BasePlayerWeapon>
	SCHEMA_ADD_FIELD( C_NetworkUtlVectorBase<CBaseHandle>, m_hMyWeapons, "CPlayer_WeaponServices->m_hMyWeapons" ); // C_BasePlayerWeapon
};

class CCSPlayer_WeaponServices : public CPlayer_WeaponServices
{
public:
	CS_CLASS_NO_INITIALIZER( CCSPlayer_WeaponServices );

public:
	SCHEMA_ADD_FIELD( GameTime_t, m_flNextAttack, "CCSPlayer_WeaponServices->m_flNextAttack" );
};

class CPlayer_ViewModelServices
{
public:
	CS_CLASS_NO_INITIALIZER( CPlayer_ViewModelServices );
};

class CCSPlayer_ViewModelServices : public CPlayer_ViewModelServices
{
public:
	CS_CLASS_NO_INITIALIZER( CCSPlayer_ViewModelServices );
};


class CPlayer_MovementServices
{
public:
	CS_CLASS_NO_INITIALIZER( CPlayer_MovementServices );

public:
	SCHEMA_ADD_FIELD( float, m_flMaxspeed, "CPlayer_MovementServices->m_flMaxspeed" );
	SCHEMA_ADD_FIELD( std::float_t, m_flForwardMove, "CPlayer_MovementServices->m_flForwardMove" );
	SCHEMA_ADD_FIELD( std::float_t, m_flLeftMove, "CPlayer_MovementServices->m_flLeftMove" );
	SCHEMA_ADD_FIELD( std::float_t, m_flSurfaceFriction, "CPlayer_MovementServices_Humanoid->m_flSurfaceFriction" );

public:
	auto RunCommand( CUserCmd* pUserCmd )
	{
		FunctionsCS2::FnRunCommand( this, pUserCmd );
	}

	void* SetPredictionCommand( CUserCmd* pUserCmd )
	{
		using fnSetPredictionCommand = void* ( __fastcall* )( void*, void* );
		static auto oSetPredictionCommand = reinterpret_cast< fnSetPredictionCommand >( GetMemoryManager()->FindPattern( CLIENT_DLL, _xor( "48 89 5C 24 ? 57 48 83 EC ? 48 8B DA E8 ? ? ? ? 48 8B F8 48 85 C0 74" ) ) );

		return oSetPredictionCommand( this, pUserCmd );
	}

	void ResetPredictionCommand( )
	{
		using fnResetPredictionCommand = void* ( __fastcall* )( );
		static auto oResetPredictionCommand = reinterpret_cast< fnResetPredictionCommand >( GetMemoryManager( )->FindPattern( CLIENT_DLL, _xor( "48 83 EC ?? B9 ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 C7 05" ) ) );

		oResetPredictionCommand( );
	}
};

class CPlayer_MovementServices_Humanoid : public CPlayer_MovementServices
{
public:
	CS_CLASS_NO_INITIALIZER( CPlayer_MovementServices_Humanoid );

public:
	SCHEMA_ADD_FIELD( bool, m_bDucking, "CPlayer_MovementServices_Humanoid->m_bDucking" );
};

class CCSPlayer_MovementServices : public CPlayer_MovementServices_Humanoid
{
public:
	CS_CLASS_NO_INITIALIZER( CCSPlayer_MovementServices );

public:
	SCHEMA_ADD_FIELD( std::float_t, m_flDuckAmount, "CCSPlayer_MovementServices->m_flDuckAmount" );
	SCHEMA_ADD_FIELD( float, m_flStamina, "CCSPlayer_MovementServices->m_flStamina" );
	SCHEMA_ADD_FIELD( Vector2D_t, m_vecLastMovementImpulses, "CPlayer_MovementServices->m_vecLastMovementImpulses" );
};

class CPlayer_ObserverServices
{
public:
	CS_CLASS_NO_INITIALIZER( CPlayer_ObserverServices );

public:
	SCHEMA_ADD_FIELD( CBaseHandle, m_hObserverTarget, "CPlayer_ObserverServices->m_hObserverTarget" );
	SCHEMA_ADD_FIELD( ObserverMode_t, m_iObserverMode, "CPlayer_ObserverServices->m_iObserverMode" );
	SCHEMA_ADD_FIELD( float, m_flObserverChaseDistance, "CPlayer_ObserverServices->m_flObserverChaseDistance" );
	SCHEMA_ADD_FIELD( GameTime_t, m_flObserverChaseDistanceCalcTime, "CPlayer_ObserverServices->m_flObserverChaseDistanceCalcTime" );
};

#pragma region pawn
class C_BasePlayerPawn : public C_BaseModelEntity
{
public:
	CS_CLASS_NO_INITIALIZER( C_BasePlayerPawn );

public:
	SCHEMA_ADD_FIELD( CBaseHandle, m_hController, "C_BasePlayerPawn->m_hController" );
	SCHEMA_ADD_FIELD( CPlayer_ObserverServices*, m_pObserverServices, "C_BasePlayerPawn->m_pObserverServices" );
	SCHEMA_ADD_FIELD( CCSPlayer_WeaponServices*, m_pWeaponServices, "C_BasePlayerPawn->m_pWeaponServices" );
	SCHEMA_ADD_FIELD( CCSPlayer_MovementServices*, m_pMovementServices, "C_BasePlayerPawn->m_pMovementServices" );
	SCHEMA_ADD_FIELD( Vector_t, m_vOldOrigin, "C_BasePlayerPawn->m_vOldOrigin" );
	SCHEMA_ADD_FIELD( std::float_t, m_flOldSimulationTime, "C_BasePlayerPawn->m_flOldSimulationTime" );

public:
	auto GetEyePosition( ) -> Vector_t
	{
		Vector_t vecEyePosition = Vector_t( );
		FunctionsCS2::FnGetEyePosition( this, &vecEyePosition );

		/*if ( m_pGameSceneNode( ) )
			vecEyePosition = m_pGameSceneNode( )->m_vecAbsOrigin( ) + m_vecViewOffset( );*/

		return vecEyePosition;
	}
};

class C_CSPlayerPawnBase : public C_BasePlayerPawn
{
public:
	CS_CLASS_NO_INITIALIZER( C_CSPlayerPawnBase );

public:
	SCHEMA_ADD_FIELD( QAngle_t, m_angEyeAngles, "C_CSPlayerPawnBase->m_angEyeAngles" );
	SCHEMA_ADD_FIELD( float, m_flLastSpawnTimeIndex, "C_CSPlayerPawnBase->m_flLastSpawnTimeIndex" );
	SCHEMA_ADD_FIELD( float, m_flLowerBodyYawTarget, "C_CSPlayerPawnBase->m_flLowerBodyYawTarget" );
	SCHEMA_ADD_FIELD( float, m_flFlashMaxAlpha, "C_CSPlayerPawnBase->m_flFlashMaxAlpha" );
	SCHEMA_ADD_FIELD( float, m_flFlashDuration, "C_CSPlayerPawnBase->m_flFlashDuration" );
	SCHEMA_ADD_FIELD( Vector_t, m_vLastSmokeOverlayColor, "C_CSPlayerPawnBase->m_vLastSmokeOverlayColor" );
	SCHEMA_ADD_FIELD( bool, m_bGunGameImmunity, "C_CSPlayerPawnBase->m_bGunGameImmunity" );
	SCHEMA_ADD_FIELD( CPlayer_ViewModelServices*, m_pViewModelServices, "C_CSPlayerPawnBase->m_pViewModelServices" );
};

class C_CSPlayerPawn : public C_CSPlayerPawnBase
{
public:
	CS_CLASS_NO_INITIALIZER( C_CSPlayerPawn );

public:
	/*[[nodiscard]] bool IsOtherEnemy( C_CSPlayerPawn* pOther );
	[[nodiscard]] int GetAssociatedTeam( );
	[[nodiscard]] bool CanAttack( const float flServerTime );
	[[nodiscard]] std::uint32_t GetOwnerHandleIndex( );
	[[nodiscard]] std::uint16_t GetCollisionMask( );
	[[nodiscard]] C_CSWeaponBase* GetPlayerWeapon( );
	[[nodiscard]] bool HasArmor( const int hitgroup );
	[[nodiscard]] bool IsThrowing( );
	[[nodiscard]] QAngle_t GetAimPunch( );*/
	static auto GetPawnFromController( CCSPlayerController* pCCSPlayerController ) -> C_CSPlayerPawn*;

public:
	SCHEMA_ADD_FIELD( bool, m_bIsScoped, "C_CSPlayerPawn->m_bIsScoped" );
	SCHEMA_ADD_FIELD( bool, m_bIsDefusing, "C_CSPlayerPawn->m_bIsDefusing" );
	SCHEMA_ADD_FIELD( bool, m_bOnGroundLastTick, "C_CSPlayerPawn->m_bOnGroundLastTick" );
	SCHEMA_ADD_FIELD( bool, m_bIsGrabbingHostage, "C_CSPlayerPawn->m_bIsGrabbingHostage" );
	SCHEMA_ADD_FIELD( bool, m_bWaitForNoAttack, "C_CSPlayerPawn->m_bWaitForNoAttack" );
	SCHEMA_ADD_FIELD( int, m_iShotsFired, "C_CSPlayerPawn->m_iShotsFired" );
	SCHEMA_ADD_FIELD( GameTime_t, m_flHealthShotBoostExpirationTime, "C_CSPlayerPawn->m_flHealthShotBoostExpirationTime" );
	SCHEMA_ADD_FIELD( std::int32_t, m_ArmorValue, "C_CSPlayerPawn->m_ArmorValue" );
	SCHEMA_ADD_FIELD( QAngle_t, m_aimPunchAngle, "C_CSPlayerPawn->m_aimPunchAngle" );
	SCHEMA_ADD_FIELD( bool, m_bNeedToReApplyGloves, ( "C_CSPlayerPawn->m_bNeedToReApplyGloves" ) );
};
#pragma endregion

#pragma region controller
class CBasePlayerController : public C_BaseModelEntity
{
public:
	CS_CLASS_NO_INITIALIZER( CBasePlayerController );

public:
	SCHEMA_ADD_FIELD( std::uint64_t, m_steamID, "CBasePlayerController->m_steamID" );
	SCHEMA_ADD_FIELD( std::uint32_t, m_nTickBase, "CBasePlayerController->m_nTickBase" );
	SCHEMA_ADD_FIELD( CBaseHandle, m_hPawn, "CBasePlayerController->m_hPawn" );
	SCHEMA_ADD_FIELD( bool, m_bIsLocalPlayerController, "CBasePlayerController->m_bIsLocalPlayerController" );
	SCHEMA_ADD_FIELD_OFFSET( CUserCmd*, m_currentCommand, "CBasePlayerController->m_steamID", -0x8 );
};

class CCSPlayerController : public CBasePlayerController
{
public:
	CS_CLASS_NO_INITIALIZER( CCSPlayerController );

public:
	static CCSPlayerController* GetLocalPlayerController( );

	// @note: always get origin from pawn not controller
	//[[nodiscard]] const Vector_t& GetPawnOrigin( );
	//[nodiscard]] void PhysicsRunThink( );
	//[[nodiscard]] void set_player_tickbase( int nTickBase );

public:
	SCHEMA_ADD_FIELD( std::uint32_t, m_iPing, "CCSPlayerController->m_iPing" );
	SCHEMA_ADD_FIELD( const char*, m_sSanitizedPlayerName, "CCSPlayerController->m_sSanitizedPlayerName" );
	SCHEMA_ADD_FIELD( std::int32_t, m_iPawnHealth, "CCSPlayerController->m_iPawnHealth" );
	SCHEMA_ADD_FIELD( std::int32_t, m_iPawnArmor, "CCSPlayerController->m_iPawnArmor" );
	SCHEMA_ADD_FIELD( bool, m_bPawnHasDefuser, "CCSPlayerController->m_bPawnHasDefuser" );
	SCHEMA_ADD_FIELD( bool, m_bPawnHasHelmet, "CCSPlayerController->m_bPawnHasHelmet" );
	SCHEMA_ADD_FIELD( bool, m_bPawnIsAlive, "CCSPlayerController->m_bPawnIsAlive" );
	SCHEMA_ADD_FIELD( CBaseHandle, m_hPlayerPawn, "CCSPlayerController->m_hPlayerPawn" );
};
#pragma endregion

class C_EconItemView
{
public:
	CS_CLASS_NO_INITIALIZER( C_EconItemView );

public:
	SCHEMA_ADD_FIELD( std::uint16_t, m_iItemDefinitionIndex, "C_EconItemView->m_iItemDefinitionIndex" );
	SCHEMA_ADD_FIELD( std::uint64_t, m_iItemID, "C_EconItemView->m_iItemID" );
	SCHEMA_ADD_FIELD( std::uint32_t, m_iItemIDHigh, "C_EconItemView->m_iItemIDHigh" );
	SCHEMA_ADD_FIELD( std::uint32_t, m_iItemIDLow, "C_EconItemView->m_iItemIDLow" );
	SCHEMA_ADD_FIELD( std::uint32_t, m_iAccountID, "C_EconItemView->m_iAccountID" );

public:
	SCHEMA_ADD_FIELD( bool, m_bDisallowSOC, "C_EconItemView->m_bDisallowSOC" );
	SCHEMA_ADD_FIELD( bool, m_bInitialized, "C_EconItemView->m_bInitialized" );

	SCHEMA_ADD_FIELD( char[ 161 ], m_szCustomName, "C_EconItemView->m_szCustomName" );
	SCHEMA_ADD_FIELD( char[ 161 ], m_szCustomNameOverride, "C_EconItemView->m_szCustomNameOverride" );
};

class CAttributeManager
{
public:
	CS_CLASS_NO_INITIALIZER( CAttributeManager );

public:
	SCHEMA_ADD_OFFSET( C_EconItemView*, m_Item, 0x50 );
	virtual ~CAttributeManager( ) = 0;
};

class C_AttributeContainer : public CAttributeManager
{
public:
	CS_CLASS_NO_INITIALIZER( C_AttributeContainer );

public:
	SCHEMA_ADD_FIELD( C_EconItemView*, m_Item, "C_AttributeContainer->m_Item" );
};

class CBaseAnimGraph : public C_BaseModelEntity
{
public:
	CS_CLASS_NO_INITIALIZER( CBaseAnimGraph );

public:
	SCHEMA_ADD_FIELD( bool, m_bClientRagdoll, "CBaseAnimGraph->m_bClientRagdoll" );

public:
	SCHEMA_ADD_FIELD( bool, m_bSequenceFinished, "CBaseAnimGraphController->m_bSequenceFinished" );
	SCHEMA_ADD_FIELD( int, m_hSequence, "CBaseAnimGraphController->m_hSequence" );
	SCHEMA_ADD_FIELD( int, m_flSeqFixedCycle, "CBaseAnimGraphController->m_flSeqFixedCycle" );
	SCHEMA_ADD_FIELD( float, m_flPlaybackRate, "CBaseAnimGraphController->m_flPlaybackRate" );
	SCHEMA_ADD_FIELD( int, m_flSeqStartTime, "CBaseAnimGraphController->m_flSeqStartTime" );
};

class C_BaseFlex : public CBaseAnimGraph
{
public:
	CS_CLASS_NO_INITIALIZER( C_BaseFlex );
	/* not implemented */
};

class C_EconEntity : public C_BaseFlex
{
public:
	CS_CLASS_NO_INITIALIZER( C_EconEntity );

public:
	SCHEMA_ADD_FIELD( C_AttributeContainer*, m_AttributeManager, "C_EconEntity->m_AttributeManager" );
	SCHEMA_ADD_FIELD( std::uint32_t, m_OriginalOwnerXuidLow, "C_EconEntity->m_OriginalOwnerXuidLow" );
	SCHEMA_ADD_FIELD( std::uint32_t, m_OriginalOwnerXuidHigh, "C_EconEntity->m_OriginalOwnerXuidHigh" );
	SCHEMA_ADD_FIELD( std::int32_t, m_nFallbackPaintKit, "C_EconEntity->m_nFallbackPaintKit" );
	SCHEMA_ADD_FIELD( std::int32_t, m_nFallbackSeed, "C_EconEntity->m_nFallbackSeed" );
	SCHEMA_ADD_FIELD( std::int32_t, m_flFallbackWear, "C_EconEntity->m_flFallbackWear" );
	SCHEMA_ADD_FIELD( std::int32_t, m_nFallbackStatTrak, "C_EconEntity->m_nFallbackStatTrak" );
	SCHEMA_ADD_FIELD( CBaseHandle, m_hViewmodelAttachment, "C_EconEntity->m_hViewmodelAttachment" );

	uint64_t GetOriginalOwnerXuid( )
	{
		return ( ( uint64_t )( m_OriginalOwnerXuidHigh( ) ) << 32 ) |
			m_OriginalOwnerXuidLow( );
	}
};

enum CSWeaponMode : int /* it works atleast */
{
	kPrimaryMode = 0,
	kSecondaryMode = 1,
	kWeaponModeMAX = 2,
};
class C_BasePlayerWeapon : public C_EconEntity
{
public:
	CS_CLASS_NO_INITIALIZER( C_BasePlayerWeapon );

public:
	SCHEMA_ADD_FIELD( GameTick_t, m_nNextPrimaryAttackTick, "C_BasePlayerWeapon->m_nNextPrimaryAttackTick" );
	SCHEMA_ADD_FIELD( float, m_flNextPrimaryAttackTickRatio, "C_BasePlayerWeapon->m_flNextPrimaryAttackTickRatio" );
	SCHEMA_ADD_FIELD( GameTick_t, m_nNextSecondaryAttackTick, "C_BasePlayerWeapon->m_nNextSecondaryAttackTick" );
	SCHEMA_ADD_FIELD( float, m_flNextSecondaryAttackTickRatio, "C_BasePlayerWeapon->m_flNextSecondaryAttackTickRatio" );

public:
	SCHEMA_ADD_FIELD( std::int32_t, m_iClip1, "C_BasePlayerWeapon->m_iClip1" );
	SCHEMA_ADD_FIELD( std::int32_t, m_iClip2, "C_BasePlayerWeapon->m_iClip2" );
	SCHEMA_ADD_FIELD( std::int32_t[ 2 ], m_pReserveAmmo, "C_BasePlayerWeapon->m_pReserveAmmo" );
	SCHEMA_ADD_FIELD( CSWeaponMode, m_weaponMode, "C_CSWeaponBase->m_weaponMode" );
};

class C_CSWeaponBase : public C_BasePlayerWeapon
{
public:
	CS_CLASS_NO_INITIALIZER( C_CSWeaponBase );

public:
	SCHEMA_ADD_FIELD( bool, m_bInReload, "C_CSWeaponBase->m_bInReload" );
	SCHEMA_ADD_FIELD( float, m_flWatTickOffset, "C_CSWeaponBase->m_flWatTickOffset" );
	SCHEMA_ADD_FIELD( int, m_iOriginalTeamNumber, ( "C_CSWeaponBase->m_iOriginalTeamNumber" ) );
	SCHEMA_ADD_FIELD( int, m_iRecoilIndex, "C_CSWeaponBase->m_iRecoilIndex" );
	SCHEMA_ADD_FIELD( bool, m_bUIWeapon, "C_CSWeaponBase->m_bUIWeapon" );
	SCHEMA_ADD_FIELD( float, m_fAccuracyPenalty, "C_CSWeaponBase->m_fAccuracyPenalty" );
	SCHEMA_ADD_FIELD( GameTick_t, m_nPostponeFireReadyTicks, "C_CSWeaponBase->m_nPostponeFireReadyTicks" );

	[[nodiscard]] int GetWeaponDefinitionIndex( )
	{
		if ( auto attribute_manager = m_AttributeManager( ) )
		{
			if ( auto item = attribute_manager->m_Item( ) )
			{
				return item->m_iItemDefinitionIndex( );
			}
		}

		return -1;
	}

	CCSWeaponBaseVData* GetWeaponVData( )
	{
		return static_cast< CCSWeaponBaseVData* >( GetVData( ) );
	}
};

class C_CSWeaponBaseGun : public C_CSWeaponBase
{
public:
	CS_CLASS_NO_INITIALIZER( C_CSWeaponBaseGun );

public:
	SCHEMA_ADD_FIELD( std::int32_t, m_zoomLevel, "C_CSWeaponBaseGun->m_zoomLevel" );
	SCHEMA_ADD_FIELD( std::int32_t, m_iBurstShotsRemaining, "C_CSWeaponBaseGun->m_iBurstShotsRemaining" );
	SCHEMA_ADD_FIELD( bool, m_bBurstMode, "C_CSWeaponBase->m_bBurstMode" );
	SCHEMA_ADD_FIELD( float, m_flPostponeFireReadyFrac, "C_CSWeaponBase->m_flPostponeFireReadyFrac" );
};



class C_C4 : public C_CSWeaponBase
{
public:
	CS_CLASS_NO_INITIALIZER( C_C4 );
};



class C_CSPlayerResource : public C_BaseEntity
{
public:
	CS_CLASS_NO_INITIALIZER( C_CSPlayerResource );

public:
	SCHEMA_ADD_FIELD( Vector_t, m_bombsiteCenterA, "C_CSPlayerResource->m_bombsiteCenterA" );
	SCHEMA_ADD_FIELD( Vector_t, m_bombsiteCenterB, "C_CSPlayerResource->m_bombsiteCenterB" );
};


class C_BaseToggle : public C_BaseModelEntity
{
public:
	CS_CLASS_NO_INITIALIZER( C_BaseToggle );
};

class C_BaseTrigger : public C_BaseToggle
{
public:
	CS_CLASS_NO_INITIALIZER( C_BaseTrigger );
};

class CBombTarget : public C_BaseTrigger
{
public:
	CS_CLASS_NO_INITIALIZER( CBombTarget );
};


class C_PointEntity : public C_BaseEntity
{
public:
	CS_CLASS_NO_INITIALIZER( C_PointEntity );
};

class CMapInfo : public C_PointEntity
{
public:
	CS_CLASS_NO_INITIALIZER( CMapInfo );

public:
	SCHEMA_ADD_FIELD( float, m_flBombRadius, "CMapInfo->m_flBombRadius" );

};


class C_PlantedC4 : public CBaseAnimGraph
{
public:
	CS_CLASS_NO_INITIALIZER( C_PlantedC4 );

public:
	SCHEMA_ADD_FIELD( std::uint32_t, m_nBombSite, "C_PlantedC4->m_nBombSite" );
	SCHEMA_ADD_FIELD( bool, m_bC4Activated, "C_PlantedC4->m_bC4Activated" );
	SCHEMA_ADD_FIELD( bool, m_bHasExploded, "C_PlantedC4->m_bHasExploded" );
	SCHEMA_ADD_FIELD( bool, m_bBombTicking, "C_PlantedC4->m_bBombTicking" );
	SCHEMA_ADD_FIELD( GameTime_t, m_flC4Blow, "C_PlantedC4->m_flC4Blow" );
	SCHEMA_ADD_FIELD( GameTime_t, m_flDefuseLength, "C_PlantedC4->m_flDefuseLength" );
	SCHEMA_ADD_FIELD( std::float_t, m_flTimerLength, "C_PlantedC4->m_flTimerLength" );
	SCHEMA_ADD_FIELD( GameTime_t, m_flDefuseCountDown, "C_PlantedC4->m_flDefuseCountDown" );
	SCHEMA_ADD_FIELD( CBaseHandle, m_hBombDefuser, "C_PlantedC4->m_hBombDefuser" ); // CHandle<C_CSPlayerPawn>
	SCHEMA_ADD_FIELD( CBaseHandle, m_pBombDefuser, "C_PlantedC4->m_pBombDefuser" ); // CHandle<C_CSPlayerPawn>

};


class C_BaseGrenade : public C_BaseFlex
{
public:
	CS_CLASS_NO_INITIALIZER( C_BaseGrenade );

public:
	SCHEMA_ADD_FIELD( bool, m_bIsSmokeGrenade, "C_BaseGrenade->m_bIsSmokeGrenade" );
	SCHEMA_ADD_FIELD( float, m_DmgRadius, "C_BaseGrenade->m_DmgRadius" );
	SCHEMA_ADD_FIELD( GameTime_t, m_flDetonateTime, "C_BaseGrenade->m_flDetonateTime" );

};

class InfoForResourceTypeIParticleSnapshot;
class C_BaseCSGrenadeProjectile : public C_BaseGrenade
{
public:
	CS_CLASS_NO_INITIALIZER( C_BaseCSGrenadeProjectile );

public:
	SCHEMA_ADD_FIELD( Vector_t, m_vInitialPosition, "C_BaseCSGrenadeProjectile->m_vInitialPosition" );
	SCHEMA_ADD_FIELD( Vector_t, m_vInitialVelocity, "C_BaseCSGrenadeProjectile->m_vInitialVelocity" );
	SCHEMA_ADD_FIELD( std::uint32_t, m_nBounces, "C_BaseCSGrenadeProjectile->m_nBounces" );

public:
	SCHEMA_ADD_FIELD( GameTime_t, m_flSpawnTime, "C_BaseCSGrenadeProjectile->m_flSpawnTime" );

public:
	SCHEMA_ADD_FIELD( Vector_t, vecLastTrailLinePos, "C_BaseCSGrenadeProjectile->vecLastTrailLinePos" );
	SCHEMA_ADD_FIELD( GameTime_t, flNextTrailLineTime, "C_BaseCSGrenadeProjectile->flNextTrailLineTime" );

public:
	SCHEMA_ADD_FIELD( bool, m_bCanCreateGrenadeTrail, "C_BaseCSGrenadeProjectile->m_bCanCreateGrenadeTrail" );

public:
	SCHEMA_ADD_FIELD( int, m_nSnapshotTrajectoryEffectIndex, "C_BaseCSGrenadeProjectile->m_nSnapshotTrajectoryEffectIndex" );
	SCHEMA_ADD_FIELD( CStrongHandle<InfoForResourceTypeIParticleSnapshot>, m_hSnapshotTrajectoryParticleSnapshot, "C_BaseCSGrenadeProjectile->m_hSnapshotTrajectoryParticleSnapshot" );

public:
	SCHEMA_ADD_FIELD( CUtlVector<Vector_t>, m_arrTrajectoryTrailPoints, "C_BaseCSGrenadeProjectile->m_arrTrajectoryTrailPoints" );
	SCHEMA_ADD_FIELD( CUtlVector<float>, m_arrTrajectoryTrailPointCreationTimes, "C_BaseCSGrenadeProjectile->m_arrTrajectoryTrailPointCreationTimes" );
	SCHEMA_ADD_FIELD( float, m_flTrajectoryTrailEffectCreationTime, "C_BaseCSGrenadeProjectile->m_flTrajectoryTrailEffectCreationTime" );
};

class C_FlashbangProjectile : public C_BaseCSGrenadeProjectile
{
public:
	CS_CLASS_NO_INITIALIZER( C_FlashbangProjectile );
};