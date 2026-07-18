#pragma once
#include <Core/Memory/Memory.hpp>

class CUtlBuffer
{
public:
	MEM_PAD( 0x80 );

	CUtlBuffer( int a1, int nSize, int a3 );

	auto PutString( const char* szString ) -> void;
	auto EnsureCapacity( int nSize ) -> void;
};
