#pragma once
#include <Core/Engine/FuncrionCS2.hpp>
#include <Core/Interfaces/InterfaceManager.hpp>
#include <Core/Interfaces/List/IEngineClient.hpp>
#include <Core/Sdk/EntityHandle.hpp>
#include <Core/Utils/PatternManager/PatternManager.hpp>


class CEntityInstance;
class CCSPlayerController;
class C_BaseEntity;

class CGameEntitySystem
{
public:
	// GetClientEntity
	template <typename T = C_BaseEntity>
	auto Get( int nIndex ) -> T*
	{
		return reinterpret_cast< T* >( this->GetBaseEntity( nIndex ) );
	}

	// GetClientEntityFromHandle
	template <typename T = C_BaseEntity>
	auto Get( const CBaseHandle hHandle ) -> T*
	{
		if ( !hHandle.IsValid( ) )
			return nullptr;

		return reinterpret_cast< T* >( this->GetBaseEntity( hHandle.GetEntryIndex( ) ) );
	}

	auto GetHighestEntityIndex( ) -> int
	{
		uintptr_t* entlist = reinterpret_cast< uintptr_t* >( this + 0x10 );
		int highestIdx = -1;

		for ( int v2 = 0; v2 < 64; ++v2 )
		{
			uintptr_t v3 = entlist[ v2 ];
			if ( !v3 )
				continue;

			for ( int v4 = 0; v4 < 512; ++v4 )
			{
				uintptr_t entptr = v3 + v4 * 0x70;
				uintptr_t ent = *reinterpret_cast< uintptr_t* >( entptr );
				if ( !ent )
					continue;

				int entIdx = -1;
				uintptr_t enthandle = *reinterpret_cast< uintptr_t* >( ent + 0x10 );
				if ( !enthandle )
					continue;

				for ( int v9 = 0, v10 = 0; v9 < 32; ++v9 )
				{
					uintptr_t defent = entlist[ v9 ];
					if ( defent && enthandle >= defent && enthandle < defent + 61440 )
					{
						entIdx = v10 + static_cast< int >( ( enthandle - defent ) / 0x70 );
						break;
					}

					uintptr_t dynamicent = entlist[ v9 + 0x20 ];
					if ( dynamicent && enthandle >= dynamicent && enthandle < dynamicent + 61440 )
					{
						entIdx = v10 + static_cast< int >( ( enthandle - dynamicent ) / 0x70 ) + 0x4000;
						break;
					}

					v10 += 512;
				}

				if ( entIdx > highestIdx )
					highestIdx = entIdx;
			}
		}

		return highestIdx;
	}

	auto GetLocalPlayerController( ) -> CCSPlayerController*;
	auto GetLocalPlayerPawn( ) -> C_CSPlayerPawn*
	{
		return FunctionsCS2::FnGetLocalPlayerPawn( );
	}

private:
	auto GetBaseEntity( int nIndex ) -> void*
	{
		// @ida: #STR: "(missing),", "(missing)", "Ent %3d: %s class %s name %s\n" | or find "cl_showents" cvar -> look for callback
		//	do { pEntity = GetBaseEntityByIndex(g_pGameEntitySystem, nCurrentIndex); ... }
		return FunctionsCS2::FnGetBaseEntity( this, nIndex );
	}
};