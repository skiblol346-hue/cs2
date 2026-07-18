#include "KeyValue3.hpp"
#include <Core/Utils/CRT.hpp>
#include <Core/Utils/PatternManager/PatternManager.hpp>

void CKeyValues3::LoadFromBuffer( const char* szString )
{
	CUtlBuffer buffer( 0, ( CRT::StringLength( szString ) + 10 ), 1 );
	buffer.PutString( szString );
	LoadKV3( &buffer );
}

bool CKeyValues3::LoadKV3( CUtlBuffer* buffer )
{
	using fnLoadKeyValues = bool( __fastcall* )( CKeyValues3*, void*, CUtlBuffer*, KV3ID_t*, const char*, void* );
	static const fnLoadKeyValues oLoadKeyValues = reinterpret_cast< fnLoadKeyValues >( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::LoadKeyValues" ) ).GetAddress( ) );

	const char* szName = _xor( "stellarix" );
	KV3ID_t kv3ID = KV3ID_t( _xor( "generic" ), 0x41B818518343427E, 0xB5F447C23C0CDF8C );
	return oLoadKeyValues( this, nullptr, buffer, &kv3ID, szName, nullptr );
}
