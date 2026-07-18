#pragma once
#include <Core/Memory/Memory.hpp>
#include <Core/Sdk/Datatypes/Color.hpp>
#include <Core/Sdk/Datatypes/Vector.hpp>
#include <Core/Sdk/EntityHandle.hpp>
#include <Core/Utils/PatternManager/PatternManager.hpp>


enum class ObjectInfoIdList : int32_t
{
	PLAYER_T = 104,
	PLAYER_CT = 113,
	ARM = 38,
};

struct CMaterialParameter
{
	Vector4D_t vecValue; // 0x0
	void* textureValue; // 0x10
	MEM_PAD( 0x10 ); // 0x18
	const char* szParameterName; // 0x28
	const char* szTextValue; // 0x30
	int64_t intValue; // 0x38
};

class CMaterial2
{
public:
	virtual const char* GetName( ) = 0;
	virtual const char* GetShareName( ) = 0;

public:
	auto FindParameter( const char* szName ) -> CMaterialParameter*
	{
		using fn_t = CMaterialParameter * ( __fastcall* )( void* _this, const char* name );
		static fn_t fn = reinterpret_cast< fn_t >( GetPatternManager( )->GetPattern( FNV1A::HashConst( "CMaterial2::FindParameter" ) ).GetAddress( ) );
		return fn( this, szName );
	}

	auto UpdateParameter( ) -> void
	{
		using fn_t = void( __fastcall* )( void* _this );
		static fn_t fn = reinterpret_cast< fn_t >( GetPatternManager( )->GetPattern( FNV1A::HashConst( "CMaterial2::UpdateParameter" ) ).GetAddress( ) );
		return fn( this );
	}
};

class CSceneAnimatableObject
{
private:
	MEM_PAD( 0xB8 );
public:
	CBaseHandle m_hEntity; // C_BaseEntity
};

class CMaterialObjectInfo
{
private:
	MEM_PAD( 0xB0 );

public:
	int nId;
};

class CMeshDrawPrimitive
{
private:
	MEM_PAD( 0x18 );
public:
	CSceneAnimatableObject* m_pSceneAnimatableObject;
	CMaterial2* m_pMaterial;
	CMaterial2* m_pMaterialCopy;
private:
	MEM_PAD( 0x10 );
public:
	CMaterialObjectInfo* m_pObjectInfo;
private:
	MEM_PAD( 0x8 );
public:
	Color_t m_Color;
};