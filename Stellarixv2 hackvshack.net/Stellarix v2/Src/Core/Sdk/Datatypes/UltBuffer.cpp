#include "UltBuffer.hpp"
#include <Core/Interfaces/InterfaceManager.hpp>
#include <Core/Engine/FuncrionCS2.hpp>

CUtlBuffer::CUtlBuffer( int a1, int nSize, int a3 )
{
	FunctionsCS2::FnUtlBufferInit( this, a1, nSize, a3 );
}

auto CUtlBuffer::PutString( const char* szString ) -> void
{
	FunctionsCS2::FnUtlBufferPutString( this, szString );
}

auto CUtlBuffer::EnsureCapacity( int nSize ) -> void
{
	FunctionsCS2::FnUtlBufferEnsureCapacity( this, nSize );
}
