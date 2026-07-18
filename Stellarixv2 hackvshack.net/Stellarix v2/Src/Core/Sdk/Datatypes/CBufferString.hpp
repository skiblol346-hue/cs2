#pragma once
#include <cstring>
#include <Core/Memory/Memory.hpp>

enum EStringConvertErrorPolicy
{
	_STRINGCONVERTFLAG_SKIP = 1,
	_STRINGCONVERTFLAG_FAIL = 2,
	_STRINGCONVERTFLAG_ASSERT = 4,

	STRINGCONVERT_REPLACE = 0,
	STRINGCONVERT_SKIP = _STRINGCONVERTFLAG_SKIP,
	STRINGCONVERT_FAIL = _STRINGCONVERTFLAG_FAIL,

	STRINGCONVERT_ASSERT_REPLACE =
	_STRINGCONVERTFLAG_ASSERT + STRINGCONVERT_REPLACE,
	STRINGCONVERT_ASSERT_SKIP = _STRINGCONVERTFLAG_ASSERT + STRINGCONVERT_SKIP,
	STRINGCONVERT_ASSERT_FAIL = _STRINGCONVERTFLAG_ASSERT + STRINGCONVERT_FAIL,
};

class CBufferString
{
public:
	enum EAllocationOption_t
	{
		UNK1 = -1,
		UNK2 = 0,
		UNK3 = ( 1 << 1 ),
		UNK4 = ( 1 << 8 ),
		UNK5 = ( 1 << 9 )
	};

	enum EAllocationFlags_t
	{
		LENGTH_MASK = ( 1 << 30 ) - 1,
		FLAGS_MASK = ~LENGTH_MASK,

		// Flags in m_nLength
		// Means it tried to grow larger than static size and heap allocation was disabled
		OVERFLOWED_MARKER = ( 1 << 30 ),
		// Means it owns the heap buffer and it needs to be cleaned up
		FREE_HEAP_MARKER = ( 1 << 31 ),

		// Flags in m_nAllocatedSize
		// Means it uses stack allocated buffer
		STACK_ALLOCATED_MARKER = ( 1 << 30 ),
		// Allows the buffer to grow beyond the static size on the heap
		ALLOW_HEAP_ALLOCATION = ( 1 << 31 )
	};

public:
	CBufferString( bool bAllowHeapAllocation = true ) :
		m_nLength( 0 ),
		m_nAllocatedSize( ( bAllowHeapAllocation* ALLOW_HEAP_ALLOCATION ) | STACK_ALLOCATED_MARKER | sizeof( m_szString ) ),
		m_pString( nullptr )
	{
	}

	CBufferString( const char* pString, bool bAllowHeapAllocation = true ) :
		CBufferString( bAllowHeapAllocation )
	{
		Insert( 0, pString );
	}

public:
	CBufferString( const CBufferString& other ) : CBufferString( )
	{
		*this = other;
	}

	~CBufferString( )
	{
		Purge( );
	}

	void SetHeapAllocationState( bool state )
	{
		if ( state )
			m_nAllocatedSize |= ALLOW_HEAP_ALLOCATION;
		else
			m_nAllocatedSize &= ~ALLOW_HEAP_ALLOCATION;
	}

	int AllocatedNum( ) const
	{
		return m_nAllocatedSize & LENGTH_MASK;
	}
	int Length( ) const
	{
		return m_nLength & LENGTH_MASK;
	}

	bool CanHeapAllocate( ) const
	{
		return ( m_nAllocatedSize & ALLOW_HEAP_ALLOCATION ) != 0;
	}
	bool IsStackAllocated( ) const
	{
		return ( m_nAllocatedSize & STACK_ALLOCATED_MARKER ) != 0;
	}
	bool ShouldFreeMemory( ) const
	{
		return ( m_nLength & FREE_HEAP_MARKER ) != 0;
	}
	bool IsOverflowed( ) const
	{
		return ( m_nLength & OVERFLOWED_MARKER ) != 0;
	}

	bool IsInputStringUnsafe( const char* pData ) const
	{
		return ( ( void* )pData >= this && ( void* )pData < &this[ 1 ] ) ||
			( !IsAllocationEmpty( ) && pData >= Base( ) && pData < ( Base( ) + AllocatedNum( ) ) );
	}

	bool IsAllocationEmpty( ) const
	{
		return AllocatedNum( ) == 0;
	}

protected:
	char* Base( )
	{
		return IsStackAllocated( ) ? m_szString : ( !IsAllocationEmpty( ) ? m_pString : nullptr );
	}
	const char* Base( ) const
	{
		return const_cast< CBufferString* >( this )->Base( );
	}

public:
	void Clear( )
	{
		if ( !IsAllocationEmpty( ) )
			Base( )[ 0 ] = '\0';

		m_nLength &= ~LENGTH_MASK;
	}

public:
	const char* Insert( int nIndex, const char* pBuf, int nCount = -1, bool bIgnoreAlignment = false )
	{
		static uintptr_t fnInsert = ( uintptr_t )GetProcAddress( ( HMODULE )GetMemoryManager( )->GetModuleBaseHandle( TIER0_DLL ), "?Insert@CBufferString@@QEAAPEBDHPEBDH_N@Z" );
		return GetMemoryManager()->CallFunction<const char*>( fnInsert, this, nIndex, pBuf, nCount, bIgnoreAlignment );
	}

	void Purge( int nAllocatedBytesToPreserve = 0 )
	{
		static uintptr_t fnPurge = ( uintptr_t )GetProcAddress( ( HMODULE )GetMemoryManager( )->GetModuleBaseHandle( TIER0_DLL ), "?Purge@CBufferString@@QEAAXH@Z" );
		GetMemoryManager( )->CallFunction<void>( fnPurge, this, nAllocatedBytesToPreserve );
	}

	void SetExtension( const char* Extension, bool bIDK = 0 )
	{
		static uintptr_t fnSetExtension = ( uintptr_t )GetProcAddress( ( HMODULE )GetMemoryManager( )->GetModuleBaseHandle( TIER0_DLL ), "?SetExtension@CBufferString@@QEAAPEBDPEBD_N@Z" );
		GetMemoryManager( )->CallFunction<void>( fnSetExtension, this, Extension, bIDK );
	}

private:
	int m_nLength;
	int m_nAllocatedSize;

	union
	{
		char* m_pString;
		char m_szString[ 8 ];
	};
};
