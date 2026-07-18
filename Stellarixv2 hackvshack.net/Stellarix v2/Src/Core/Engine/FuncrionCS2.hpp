#pragma once
#include <cstdint>
#include <string>
#include <Core/Sdk/Datatypes/Vector.hpp>
#include <Core/Sdk/Datatypes/StrongHandle.hpp>

class CUtlBuffer;
class CCSPlayerController;
class C_CSPlayerPawn;
class CGameEntitySystem;
class CCSGOInput;
class C_BaseEntity;
class CGameParticleManager;
class CParticleSnapshot;
class C_BasePlayerPawn;
class CPlayer_MovementServices;
class CUserCmd;
class C_BasePlayerWeapon;
class CPlayer_WeaponServices;

struct PlantedBombInfo_t
{
	bool bPlanted = false;
	const char* szBombSite = "None";
	std::string szDefuserName = "";
	int iBombDamage = 0;
	float flBombTime = 15;
	float flTimerLength = 40;
	Vector_t vecPosition;
};

namespace FunctionsCS2
{
	auto OnInit( ) -> bool;

	auto GetBombInfo( ) -> const PlantedBombInfo_t;

	inline void( __fastcall* FnUtlBufferInit )( CUtlBuffer*, int, int, int );
	inline void( __fastcall* FnUtlBufferPutString )( CUtlBuffer*, const char* );
	inline void( __fastcall* FnUtlBufferEnsureCapacity )( CUtlBuffer*, int );

	inline std::int64_t( __fastcall* FnCreateMaterial )( void*, void*, const char*, void*, unsigned int, unsigned int );

	inline void( __fastcall* FnGetLocalPlayerIndex )( void*, int&, int );
	inline CCSPlayerController* ( __fastcall* FnGetLocalPlayerController )( int );
	inline C_CSPlayerPawn* ( __fastcall* FnGetLocalPlayerPawn )( );
	inline void* ( __fastcall* FnGetBaseEntity )( CGameEntitySystem*, int );

	inline void* ( __fastcall* FnRunCommand )( CPlayer_MovementServices*, CUserCmd* );
	inline void* ( __fastcall* FnGetEyePosition )( C_BasePlayerPawn*, Vector_t* );

	inline C_BasePlayerWeapon* ( __fastcall* FnGetActiveWeapon )( CPlayer_WeaponServices* );

	inline std::int64_t( __fastcall* FnGetViewAngles )( CCSGOInput*, std::int32_t );
	inline std::int64_t( __fastcall* FnSetViewAngles )( CCSGOInput*, std::int32_t, QAngle_t& );

	inline bool( __fastcall* FnComputeHitboxSurroundingBox )( C_BaseEntity*, Vector_t&, Vector_t& );
	inline bool( __fastcall* FnScreenTransform )( Vector_t&, Vector_t& );

	inline bool( __fastcall* FnGetIsInGame )( void* );
	inline bool( __fastcall* FnGetIsConnected )( void* );

	inline bool( __fastcall* FnSetPlayerIsReady )( void*, const char* );

	inline CGameParticleManager* ( __fastcall* FnGetGameParticleSystem )( );
	inline int*( __fastcall* FnCacheParticleEffect )( CGameParticleManager*, unsigned int*, const char*, int, __int64, __int64, __int64, int ) = nullptr;
	inline bool( __fastcall* FnCreateParticleEffect )( CGameParticleManager*, unsigned int, int, void*, int ) = nullptr;
	inline bool( __fastcall* FnUnknownParticleFunction )( CGameParticleManager*, int, unsigned int, const void* ) = nullptr;
}