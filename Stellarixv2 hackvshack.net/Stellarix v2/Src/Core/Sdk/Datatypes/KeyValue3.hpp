#pragma once
#include "UltBuffer.hpp"
#include <Core/Memory/Memory.hpp>
#include <Core/Utils/PatternManager/PatternManager.hpp>


struct KV3ID_t
{
	const char* szName;
	std::uint64_t unk0;
	std::uint64_t unk1;
};

class CKeyValues3
{
public:
	MEM_PAD( 0x100 );
	std::uint64_t uKey;
	void* pValue;
	MEM_PAD( 0x8 );

	void LoadFromBuffer( const char* szString );
	bool LoadKV3( CUtlBuffer* buffer );

	static CKeyValues3* CreateMaterialResource( )
	{
		using fnSetTypeKV3 = CKeyValues3 * ( __fastcall* )( CKeyValues3*, unsigned int, unsigned int );
		static const fnSetTypeKV3 oSetTypeKV3 = reinterpret_cast< fnSetTypeKV3 >( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::SetTypeKV3" ) ).GetAddress( ) );

		CKeyValues3* pKeyValue = new CKeyValues3[ 0x10 ];
		return oSetTypeKV3( pKeyValue, 1U, 6U );
	}
};
