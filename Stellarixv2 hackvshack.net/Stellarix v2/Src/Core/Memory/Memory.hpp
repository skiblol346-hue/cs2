#pragma once
#include <windows.h>
#include <stdint.h>
#include <vector>
#include <Core/Common.hpp>
#include <Core/Utils/Pe64.hpp>


#pragma region memory_definitions
#pragma warning(push)
#pragma warning(disable: 6255) // '_alloca' indicates failure by raising a stack overflow exception. consider using '_malloca' instead
#define MEM_STACKALLOC(SIZE) _malloca(SIZE)
#pragma warning(pop)
#define MEM_STACKFREE(MEMORY) static_cast<void>(0)

#define MEM_PAD(SIZE)										\
private:												\
	char CS_CONCATENATE(pad_0, __COUNTER__)[SIZE]; \
public:
#pragma endregion



namespace MODULES
{
	inline void* pClientDLL = nullptr;
	inline void* pEngine2DLL = nullptr;
	inline void* pSchemaSystemDLL = nullptr;
	inline void* pInputSystemDLL = nullptr;
	inline void* pSDL3DLL = nullptr;
	inline void* pTier0DLL = nullptr;
	inline void* pNavSystemDLL = nullptr;
	inline void* pRenderSystemDLL = nullptr;
	inline void* pLocalizeDLL = nullptr;
	inline void* pDbgHelpDLL = nullptr;
	inline void* pGameOverlayRenderer64DLL = nullptr;
	inline void* pParticlesDLL = nullptr;
	inline void* pSceneSystemDLL = nullptr;
	inline void* pMaterialSystem2DLL = nullptr;
	inline void* pMatchmakingDLL = nullptr;
	inline void* pResourceSystemDLL = nullptr;
	inline void* pSoundSystemDLL = nullptr;
	inline void* pAnimationSystemDLL = nullptr;
}



class CMemoryManager
{
public:
	CMemoryManager( ) = default;
	~CMemoryManager( ) = default;

public:
	auto OnInit( ) -> bool;
	auto OnDestroy( ) -> void;

public:
	auto GetModuleBaseHandle( const wchar_t* wszModuleName ) -> void*;
	auto ResolveRelativeAddress( std::uint8_t* nAddressBytes, std::uint32_t nRVAOffset, std::uint32_t nRIPOffset ) -> std::uint8_t*;

public:
	auto FindPattern( const wchar_t* wszModuleName, const char* szPattern ) -> std::uint8_t*;
	auto FindPattern( const wchar_t* wszModuleName, const char* szBytePattern, const char* szByteMask ) -> std::uint8_t*;
	auto FindPatternEx( const std::uint8_t* pRegionStart, const std::size_t nRegionSize, const std::uint8_t* arrByteBuffer, const std::size_t nByteCount, const char* szByteMask = nullptr ) -> std::uint8_t*;
	auto FindPatternAllOccurrencesEx( const std::uint8_t* pRegionStart, const std::size_t nRegionSize, const std::uint8_t* arrByteBuffer, const std::size_t nByteCount, const char* szByteMask = nullptr ) -> std::vector<std::uint8_t*>;

public:
	auto PatternToBytes( const char* szPattern, std::uint8_t* pOutByteBuffer, char* szOutMaskBuffer ) -> std::size_t;
	auto BytesToPattern( const std::uint8_t* pByteBuffer, const std::size_t nByteCount, char* szOutBuffer ) -> std::size_t;

public:
	auto HeapAlloc( const std::size_t nSize ) -> void*;
	auto HeapFree( void* pMemory ) -> void;

public:
	auto GetExportAddress( const void* hModuleBase, const char* szProcedureName ) -> void*;
	auto GetModuleBaseFileName( const void* hModuleBase, const bool bGetFullPath = false ) -> const wchar_t*;

public:
	template <typename T = std::uint8_t>
	[[nodiscard]] T* GetAbsoluteAddress( T* pRelativeAddress, int nPreOffset = 0x0, int nPostOffset = 0x0 )
	{
		pRelativeAddress += nPreOffset;
		pRelativeAddress += sizeof( std::int32_t ) + *reinterpret_cast< std::int32_t* >( pRelativeAddress );
		pRelativeAddress += nPostOffset;
		return pRelativeAddress;
	}

public:
	template <typename ReturnType, typename... Args>
	inline constexpr ReturnType CallFunction( uintptr_t FunctionAddress, Args... Arguements )
	{
		auto function = reinterpret_cast< ReturnType( * )( Args... ) >( FunctionAddress );
		return function( std::forward<Args>( Arguements )... );
	}

	template <typename T, std::size_t nIndex, class CBaseClass, typename... Args_t>
	static __forceinline T CallVFunc( CBaseClass* thisptr, Args_t... argList )
	{
		using VirtualFn_t = T( __thiscall* )( const void*, decltype( argList )... );
		return ( *reinterpret_cast< VirtualFn_t* const* >( reinterpret_cast< std::uintptr_t >( thisptr ) ) )[ nIndex ]( thisptr, argList... );
	}

	template <typename T = void*>
	static __forceinline T GetVFunc( const void* thisptr, std::size_t nIndex )
	{
		return ( *static_cast< T* const* >( thisptr ) )[ nIndex ];
	}

private:
	bool m_bInitialized = false;
};

auto GetMemoryManager( ) -> CMemoryManager*;