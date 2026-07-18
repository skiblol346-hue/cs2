#include "FuncrionCS2.hpp"
#include <Core/Interfaces//List/IEngineClient.hpp>
#include <Core/Memory/Memory.hpp>
#include <Core/Sdk/EntityData.hpp>
#include <Core/Utils/PatternManager/PatternManager.hpp>
#include <Core/Interfaces/List/IGlobalVars.hpp>
#include <Core/Sdk/EntityCache/EntityCache.hpp>


auto FunctionsCS2::OnInit( ) -> bool
{
	bool bResult = true;

	FnUtlBufferInit = reinterpret_cast< decltype( FnUtlBufferInit ) >( GetMemoryManager( )->GetExportAddress( MODULES::pTier0DLL, _xor( "??0CUtlBuffer@@QEAA@HHW4BufferFlags_t@0@@Z" ) ) );
	bResult &= ( FnUtlBufferInit != nullptr );
	FnUtlBufferPutString = reinterpret_cast< decltype( FnUtlBufferPutString ) >( GetMemoryManager( )->GetExportAddress( MODULES::pTier0DLL, _xor( "?PutString@CUtlBuffer@@QEAAXPEBD@Z" ) ) );
	bResult &= ( FnUtlBufferPutString != nullptr );
	FnUtlBufferEnsureCapacity = reinterpret_cast< decltype( FnUtlBufferEnsureCapacity ) >( GetMemoryManager( )->GetExportAddress( MODULES::pTier0DLL, _xor( "?EnsureCapacity@CUtlBuffer@@QEAAXH@Z" ) ) );
	bResult &= ( FnUtlBufferEnsureCapacity != nullptr );


	FnCreateMaterial = reinterpret_cast< decltype( FnCreateMaterial ) >( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::CreateMaterial" ) ).GetAddress( ) );
	bResult &= ( FnCreateMaterial != nullptr );


	FnGetLocalPlayerIndex = reinterpret_cast< decltype( FnGetLocalPlayerIndex ) >( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::GetLocalPlayerIndex" ) ).GetAddress( ) );
	bResult &= ( FnGetLocalPlayerIndex != nullptr );
	FnGetLocalPlayerController = reinterpret_cast< decltype( FnGetLocalPlayerController ) >( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::GetLocalPlayerController" ) ).GetAddress( ) );
	bResult &= ( FnGetLocalPlayerController != nullptr );
	FnGetLocalPlayerPawn = reinterpret_cast< decltype( FnGetLocalPlayerPawn ) >( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::GetLocalPlayerPawn" ) ).GetAddress( ) );
	bResult &= ( FnGetLocalPlayerPawn != nullptr );
	FnGetBaseEntity = reinterpret_cast< decltype( FnGetBaseEntity ) >( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::GetBaseEntity" ) ).GetAddress( ) );
	bResult &= ( FnGetBaseEntity != nullptr );
	FnGetEyePosition = reinterpret_cast< decltype( FnGetEyePosition ) >( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::GetEyePosition" ) ).GetAddress( ) );
	bResult &= ( FnGetEyePosition != nullptr );
	
	
	FnGetActiveWeapon = reinterpret_cast< decltype( FnGetActiveWeapon ) >( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::GetActiveWeapon" ) ).GetAddress( ) );
	bResult &= ( FnGetActiveWeapon != nullptr );
	

	FnRunCommand = reinterpret_cast< decltype( FnRunCommand ) >( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::RunCommand" ) ).GetAddress( ) );
	bResult &= ( FnRunCommand != nullptr );


	FnGetViewAngles = reinterpret_cast< decltype( FnGetViewAngles ) >( GetPatternManager( )->GetPattern( FNV1A::HashConst( "CCSGOInput::GetViewAngle" ) ).GetAddress( ) );
	bResult &= ( FnGetViewAngles != nullptr );
	FnSetViewAngles = reinterpret_cast< decltype( FnSetViewAngles ) >( GetPatternManager( )->GetPattern( FNV1A::HashConst( "CCSGOInput::SetViewAngle" ) ).GetAddress( ) );
	bResult &= ( FnSetViewAngles != nullptr );


	FnComputeHitboxSurroundingBox = reinterpret_cast< decltype( FnComputeHitboxSurroundingBox ) >( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::ComputeHitboxSurroundingBox" ) ).GetAddress( ) );
	bResult &= ( FnComputeHitboxSurroundingBox != nullptr );
	
	FnScreenTransform = reinterpret_cast< decltype( FnScreenTransform ) >( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::ScreenTransform" ) ).GetAddress( ) );
	bResult &= ( FnScreenTransform != nullptr );


	FnGetIsInGame = reinterpret_cast< decltype( FnGetIsInGame ) >( GetPatternManager( )->GetPattern( FNV1A::HashConst( "IEngineClient::IsInGame" ) ).GetAddress( ) );
	bResult &= ( FnGetIsInGame != nullptr );
	FnGetIsConnected = reinterpret_cast< decltype( FnGetIsConnected ) >( GetPatternManager( )->GetPattern( FNV1A::HashConst( "IEngineClient::IsConnected" ) ).GetAddress( ) );
	bResult &= ( FnGetIsConnected != nullptr );


	FnSetPlayerIsReady = reinterpret_cast< decltype( FnSetPlayerIsReady ) >( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::SetReadyPlayer" ) ).GetAddress( ) );
	bResult &= ( FnSetPlayerIsReady != nullptr );
	
	
	FnGetGameParticleSystem = reinterpret_cast< decltype( FnGetGameParticleSystem ) >( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::GetGameParticleSystem" ) ).GetAddress( ) );
	bResult &= ( FnGetGameParticleSystem != nullptr );
	FnCacheParticleEffect = reinterpret_cast< decltype( FnCacheParticleEffect ) >( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::CacheParticleEffect" ) ).GetAddress( ) );
	bResult &= ( FnCacheParticleEffect != nullptr );
	FnCreateParticleEffect = reinterpret_cast< decltype( FnCreateParticleEffect ) >( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::CreateParticleEffect" ) ).GetAddress( ) );
	bResult &= ( FnCreateParticleEffect != nullptr );
	FnUnknownParticleFunction = reinterpret_cast< decltype( FnUnknownParticleFunction ) >( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::UnknownParticleFunction" ) ).GetAddress( ) );
	bResult &= ( FnUnknownParticleFunction != nullptr );


	return bResult;
}

auto FunctionsCS2::GetBombInfo( ) -> const PlantedBombInfo_t
{
	if ( !GetEntityCache( )->pC_PlantedC4.load( ) )
		return PlantedBombInfo_t( );

	CCSPlayerController* pLocalController = CCSPlayerController::GetLocalPlayerController( );
	C_CSPlayerPawn* pLocalPawn = C_CSPlayerPawn::GetPawnFromController( pLocalController );
	Vector_t vecLocalPlayerPos = pLocalPawn ? pLocalPawn->GetOrigin( ) : GetEntityCache( )->pC_PlantedC4.load( )->GetOrigin( );
	float flLocalPlayerArmorValue = pLocalPawn ? pLocalPawn->m_ArmorValue( ) : 0;

	static auto lambdaGetBombSite = [&]( )
	{
		switch ( GetEntityCache( )->pC_PlantedC4.load( )->m_nBombSite( ) )
		{
		case 0:
			return "A";
		case 1:
			return "B";
		};

		return "";
	};
	static auto lambdaGetBombData = [&]( )
	{
		// 48 89 5CONFIG 24 ? 57 48 83 EC ? 48 8B 05 ? ? ? ? F3 0F 10 05 (server.dll) 13.05.25
		float v0 = 500.f;
		if ( GetEntityCache( )->pCMapInfo.load( ) )
			v0 = GetEntityCache( )->pCMapInfo.load( )->m_flBombRadius( );

		return std::pair<float, float>( v0, v0 * 3.5f );
	};
	static auto lambdaGetBombArmorModifier = [&]( float flDamage, float flArmor )
	{
		if ( flArmor > 0 )
		{
			const float armor_ratio = 0.5f;
			const float armor_bonus = 0.5f;
			float armor_ratio_multiply = flDamage * armor_ratio;
			float actual = ( flDamage - armor_ratio_multiply ) * armor_bonus;

			if ( actual > flArmor )
			{
				actual = flArmor * ( 1.f / armor_bonus );
				armor_ratio_multiply = flDamage - actual;
			}

			flDamage = armor_ratio_multiply;
		}
		return flDamage;
	};
	static auto lambdaGetBombDamage = [&]( Vector_t playerPos, Vector_t bombPos, float flArmor )
	{
		playerPos.z = bombPos.z; // @leakmem: without it line flDamage we get invalid distance

		const std::pair<float, float> bomb_calculations = lambdaGetBombData( );
		const float bomb_damage = bomb_calculations.first;
		const float bomb_radius = bomb_calculations.second;

		double c = bomb_radius / 3.0;
		int distance = bombPos.DistTo( playerPos );

		float damage = bomb_damage * ( float )exp( -pow( distance, 2 ) / ( 2 * pow( c, 2 ) ) );
		float damageArmor = lambdaGetBombArmorModifier( damage, flArmor );

		return ( int )floor( damageArmor );
	};

	float flCurrentTime = CheatData.flGVCurrentTime;

	PlantedBombInfo_t plantedBombInfo;
	plantedBombInfo.bPlanted = GetEntityCache( )->pC_PlantedC4.load( )->m_bC4Activated( ) && GetEntityCache( )->pC_PlantedC4.load( )->m_bBombTicking( ) && !GetEntityCache( )->pC_PlantedC4.load( )->m_bHasExploded( );
	plantedBombInfo.szBombSite = plantedBombInfo.bPlanted ? lambdaGetBombSite( ) : _xor( "None" );
	plantedBombInfo.iBombDamage = plantedBombInfo.bPlanted ? lambdaGetBombDamage( vecLocalPlayerPos, GetEntityCache( )->pC_PlantedC4.load( )->GetOrigin( ), flLocalPlayerArmorValue ) : 0;
	plantedBombInfo.flBombTime = MATH::Clamp<float>( GetEntityCache( )->pC_PlantedC4.load( )->m_flC4Blow( ) - flCurrentTime, 0.f, 1000.f );
	plantedBombInfo.vecPosition = GetEntityCache( )->pC_PlantedC4.load( )->GetOrigin( );


	static float flStartDefuseTime = -1.f;
	if ( GetEntityCache( )->pC_PlantedC4.load( )->m_hBombDefuser( ).IsValid( ) )
	{
		if ( flStartDefuseTime == -1.f )
			flStartDefuseTime = flCurrentTime;

		plantedBombInfo.flTimerLength = GetEntityCache( )->pC_PlantedC4.load( )->m_flDefuseLength( );
		plantedBombInfo.flBombTime = -1 * ( flStartDefuseTime - flCurrentTime );
		if ( plantedBombInfo.szDefuserName.empty( ) )
		{
			auto pDefuserPawn = GetInterfaceManager( )->GetGameEntitySystem( )->Get<C_CSPlayerPawn>( GetEntityCache( )->pC_PlantedC4.load( )->m_hBombDefuser( ) );
			if ( pDefuserPawn && pDefuserPawn->m_hController( ).IsValid( ) )
			{
				auto pDefuserController = GetInterfaceManager( )->GetGameEntitySystem( )->Get<CCSPlayerController>( pDefuserPawn->m_hController( ) );
				if ( pDefuserController )
					plantedBombInfo.szDefuserName = pDefuserController->m_sSanitizedPlayerName( );
			}
		}
	}
	else
	{
		flStartDefuseTime = -1.f;

		plantedBombInfo.flTimerLength = GetEntityCache( )->pC_PlantedC4.load( )->m_flTimerLength( );
		plantedBombInfo.szDefuserName = "";
	}

	return plantedBombInfo;
}