#pragma once
#include <Core/Memory/Memory.hpp>
#include <Core/Sdk/Datatypes/Vector.hpp>
#include <Core/Sdk/Datatypes/StrongHandle.hpp>
#include <Core/Engine/FuncrionCS2.hpp>

class InfoForResourceTypeIParticleSnapshot;

class CParticleSnapshot
{
public:
	void Draw( int nCount, void* pData )
	{
		GetMemoryManager( )->CallVFunc<void, 1U>( this, nCount, pData );
	}
};

class CParticleEffect
{
public:
	const char* name{};
	char pad[ 0x30 ]{};
};

struct particle_color
{
	float r;
	float g;
	float b;
};

class CParticleInformation
{
public:
	float time{};
	float unk2{};
	float width{};
};

// This class changed slightly when molotov's where changed.
// I did not reverse this and getting positions and times as pointers like this is most certainly not a big brain move.
// credits: epoxi [https://www.unknowncheats.me/forum/members/5188035.html]
class CParticleData
{
public:
	Vector_t* m_vecPosition;

private:
	char pad_06[ 0x74 ];

public:
	;
	float* m_flTimes;

private:
	char pad_09[ 0x28 ];

public:
	;
	float* m_flTimes2;

private:
	char pad_013[ 0x98 ];

public:
	;
}; //Size: 0x0150

class CBeamInfo
{
public:
	unsigned int m_nEffectIndex = -1;
	Vector_t* m_vPositions = nullptr;
	float* m_flTimes = nullptr;
	CStrongHandle<CParticleSnapshot> m_hSnapsotParticle{};
	CParticleData m_ParticleData;
};

struct CParticleColor
{
	float r;
	float g;
	float b;
};

class IParticleSystemMgr
{
public:
	void CreateSnapshot( CStrongHandle<CParticleSnapshot>* pOutParticleSnapshot )
	{
		__int64 nUnknown = 0;
		GetMemoryManager( )->CallVFunc<void, 41U>( this, pOutParticleSnapshot, &nUnknown );
	}
	
	void CreateSnapshot( CStrongHandle<InfoForResourceTypeIParticleSnapshot>* pOutParticleSnapshot )
	{
		__int64 nUnknown = 0;
		GetMemoryManager( )->CallVFunc<void, 41U>( this, pOutParticleSnapshot, &nUnknown );
	}

	// This does the same as calling the Draw function from particle snapshot at index 1
	void Draw( CStrongHandle<CParticleSnapshot>* pParticleSnapshot, int nCount, void* pData )
	{
		GetMemoryManager( )->CallVFunc<void, 42U>( this, pParticleSnapshot, nCount, pData );
	}
	void Draw( CStrongHandle<InfoForResourceTypeIParticleSnapshot>* pParticleSnapshot, int nCount, void* pData )
	{
		GetMemoryManager( )->CallVFunc<void, 42U>( this, pParticleSnapshot, nCount, pData );
	}
};

class CGameParticleManager
{
public:
	// inline int*(__fastcall* fnCacheParticleEffect)(CGameParticleManager*, unsigned int*, const char*, int, __int64, __int64, __int64, int) = nullptr;
	int* CacheParticleEffect( unsigned int* pEffectIndex, const char* szName )
	{
		if ( !FunctionsCS2::FnCacheParticleEffect )
			return 0;
		return FunctionsCS2::FnCacheParticleEffect( this, pEffectIndex, szName, 8, 0ll, 0ll, 0ll, 0 );
		//return ReturnAddressSpoofer::SpoofCall(ReturnAddressSpoofGadgets::m_pClientGadet, MEM::fnCacheParticleEffect, this, pEffectIndex, szName, 8, 0ll, 0ll, 0ll, 0);
	}

	// This function does not create a particle effect, it simply changes it's attributes
	// inline bool(__fastcall* fnCreateParticleEffect)(CGameParticleManager*, unsigned int, int, void*, int) = NULL;
	bool CreateParticleEffect( unsigned int nEffectIndex, int nUnknown, void* pData, int nUnknown1 )
	{
		if ( !FunctionsCS2::FnCreateParticleEffect )
			return false;
		return FunctionsCS2::FnCreateParticleEffect( this, nEffectIndex, nUnknown, pData, nUnknown1 );
	}

	// inline bool(__fastcall* fnUnknownParticleFunction)(CGameParticleManager*, int, unsigned int, const CStrongHandle<CParticleSnapshot>*) = NULL;
	bool UnknownFunction( int nEffectIndex, unsigned int nUnknown, const CStrongHandle<CParticleSnapshot>* pParticleSnapshot )
	{
		if ( !FunctionsCS2::FnUnknownParticleFunction )
			return false;
		return FunctionsCS2::FnUnknownParticleFunction( this, nEffectIndex, nUnknown, pParticleSnapshot );
	}
	bool UnknownFunction( int nEffectIndex, unsigned int nUnknown, const CStrongHandle<InfoForResourceTypeIParticleSnapshot>* pParticleSnapshot )
	{
		if ( !FunctionsCS2::FnUnknownParticleFunction )
			return false;
		return FunctionsCS2::FnUnknownParticleFunction( this, nEffectIndex, nUnknown, pParticleSnapshot );
	}
};
