#include "EntityData.hpp"
#include <Core/Engine/Convar/Convar.hpp>
#include <Core/Engine/FuncrionCS2.hpp>
#include <Core/Interfaces/List/CGameEntitySystem.hpp>
#include <Core/Interfaces/List/IEngineCVar.hpp>
#include <Core/Renderer/Renderer.hpp>
#include <Core/Sdk/Datatypes/Transform.hpp>
#include <Core/Utils/Logger/Logger.hpp>
#include <Core/Utils/PatternManager/PatternManager.hpp>
#include <Core/Hooks/List/Hook_SetModel.hpp>


auto C_BaseEntity::GetHitboxMinsMaxs( Vector_t& mins, Vector_t& maxs ) -> bool
{
	return FunctionsCS2::FnComputeHitboxSurroundingBox( this, mins, maxs );
}

auto C_BaseEntity::GetEntityBoundingBox( ImVec4* pVecOut, bool bComputeSurroundingBox ) -> bool
{
	auto pCollision = m_pCollision( );
	if ( !pCollision )
		return false;

	Vector_t vecMins;
	Vector_t vecMaxs;

	if ( bComputeSurroundingBox )
	{
		if ( !GetHitboxMinsMaxs( vecMins, vecMaxs ) )
			return false;
	}
	else
	{
		const auto absOrigin = GetOrigin( );

		vecMins = pCollision->m_vecMins( ) + absOrigin;
		vecMaxs = pCollision->m_vecMaxs( ) + absOrigin;
	}

#undef max
#undef min

	pVecOut->x = pVecOut->y = std::numeric_limits<float>::max( );
	pVecOut->z = pVecOut->w = -std::numeric_limits<float>::max( );

	for ( int i = 0; i < 8; ++i )
	{
		const Vector_t vecPoint{
			i & 1 ? vecMaxs.x : vecMins.x,
			i & 2 ? vecMaxs.y : vecMins.y,
			i & 4 ? vecMaxs.z : vecMins.z
		};
		ImVec2 vecScreen;
		if ( !GetRenderer( )->WorldToScreen( vecPoint, &vecScreen ) )
			return false;

		pVecOut->x = MATH::Min( pVecOut->x, vecScreen.x );
		pVecOut->y = MATH::Min( pVecOut->y, vecScreen.y );
		pVecOut->z = MATH::Max( pVecOut->z, vecScreen.x );
		pVecOut->w = MATH::Max( pVecOut->w, vecScreen.y );
	}

	return true;
}

auto C_BaseModelEntity::SetModel( const char* szName ) -> void
{
	return SetModel_o.call<void>( this, szName );
}

auto C_BaseEntity::IsEnemy( C_BaseEntity* pOther ) -> bool
{
	// check are other player is invalid or we're comparing against ourselves
	if ( !pOther || this == pOther )
		return false;

	if ( this->m_iTeamNum( ) != pOther->m_iTeamNum( ) )
		return true;

	// @todo: check is deathmatch
	return CONVAR::mp_teammates_are_enemies->value.bl;
}

auto C_CSPlayerPawn::GetPawnFromController( CCSPlayerController* pCCSPlayerController ) -> C_CSPlayerPawn*
{
	if ( !pCCSPlayerController )
		return nullptr;

	if ( !pCCSPlayerController->m_hPawn( ).IsValid( ) )
		return nullptr;

	return GetInterfaceManager( )->GetGameEntitySystem( )->Get<C_CSPlayerPawn>( pCCSPlayerController->m_hPawn( ) );
}

CCSPlayerController* CCSPlayerController::GetLocalPlayerController( )
{
	int iLocalPlayerIndex = GetInterfaceManager( )->GetEngineClient( )->GetLocalPlayerIndex( );
	if ( iLocalPlayerIndex <= -1 )
		return nullptr;

	return GetInterfaceManager( )->GetGameEntitySystem( )->Get<CCSPlayerController>( GetInterfaceManager( )->GetEngineClient( )->GetLocalPlayerIndex( ) );
}
