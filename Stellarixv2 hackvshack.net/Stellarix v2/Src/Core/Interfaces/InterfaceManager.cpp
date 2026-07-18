#include "InterfaceManager.hpp"
#include <Core/Memory/Memory.hpp>
#include <Core/Utils/Logger/Logger.hpp>
#include <Core/Utils/PatternManager/PatternManager.hpp>

// List Interfaces
#include "List/IGlobalVars.hpp"
#include "List/ISwapChain.hpp"
#include <Core/Engine/FuncrionCS2.hpp>
#include <Core/Sdk/Datatypes/UltBuffer.hpp>

#pragma region interfaces_get

using InstantiateInterfaceFn_t = void* ( * )( );
class CInterfaceRegister
{
public:
	InstantiateInterfaceFn_t fnCreate;
	const char* szName;
	CInterfaceRegister* pNext;
};

static const CInterfaceRegister* GetRegisterList( void* hModule )
{
	std::uint8_t* pCreateInterface = reinterpret_cast< std::uint8_t* >( GetMemoryManager( )->GetExportAddress( hModule, _xor( "CreateInterface" ) ) );

	if ( !pCreateInterface )
	{
		L_PRINT( LOG_ERROR ) << _xor( "failed to get \"CreateInterface\" address" );
		return nullptr;
	}

	return *reinterpret_cast< CInterfaceRegister** >( GetMemoryManager( )->ResolveRelativeAddress( pCreateInterface, 0x3, 0x7 ) );
}

template <typename T = void*>
T* Capture( const CInterfaceRegister* pModuleRegister, const char* szInterfaceName )
{
	for ( const CInterfaceRegister* pRegister = pModuleRegister; pRegister != nullptr; pRegister = pRegister->pNext )
	{
		if ( const std::size_t nInterfaceNameLength = CRT::StringLength( szInterfaceName );
			 // found needed interface
			 CRT::StringCompareN( szInterfaceName, pRegister->szName, nInterfaceNameLength ) == 0 &&
			 // and we've given full name with hardcoded digits
			 ( CRT::StringLength( pRegister->szName ) == nInterfaceNameLength ||
			   // or it contains digits after name
			   CRT::StringToInteger<int>( pRegister->szName + nInterfaceNameLength, nullptr, 10 ) > 0 ) )
		{
			// capture our interface
			void* pInterface = pRegister->fnCreate( );

#ifdef _DEBUG
			// log interface address
			L_PRINT( LOG_INFO ) << _xor( "captured \"" ) << pRegister->szName << _xor( "\" interface at address: " ) << LOGGING::AddFlags( LOG_MODE_INT_SHOWBASE | LOG_MODE_INT_FORMAT_HEX ) << reinterpret_cast< std::uintptr_t >( pInterface );
#else
			L_PRINT( LOG_INFO ) << _xor( "captured \"" ) << pRegister->szName << _xor( "\" interface" );
#endif

			return static_cast< T* >( pInterface );
		}
	}

	L_PRINT( LOG_ERROR ) << _xor( "failed to find interface \"" ) << szInterfaceName << _xor( "\"" );
	return nullptr;
}

#pragma endregion

static CInterfaceManager g_CInterfaceManager{ };

auto CInterfaceManager::OnInit( ) -> bool
{
	bool bSuccess = true;

	bSuccess &= ( GetMemAlloc( ) != nullptr );
	bSuccess &= ( GetSchemaSystem( ) != nullptr );
	bSuccess &= ( GetSwapChain( ) != nullptr );
	bSuccess &= ( GetInputSystem( ) != nullptr );
	bSuccess &= ( GetGameEntitySystem( ) != nullptr );
	bSuccess &= ( GetGlobalVars( ) != nullptr );
	bSuccess &= ( GetSource2Client( ) != nullptr );
	bSuccess &= ( GetEngineClient( ) != nullptr );
	bSuccess &= ( GetResourceSystem( ) != nullptr );
	bSuccess &= ( GetEngineCVar( ) != nullptr );
	bSuccess &= ( GetGameEventManager( ) != nullptr );
	bSuccess &= ( GetGameParticleManager( ) != nullptr );
	bSuccess &= ( GetParticleSystemManager( ) != nullptr );
	bSuccess &= ( GetNetworkClientService( ) != nullptr );
	bSuccess &= ( GetCIPrediction( ) != nullptr );

	m_bInitialized = bSuccess;
	return m_bInitialized;
}

auto CInterfaceManager::OnDestroy( ) -> void
{
	if ( !m_bInitialized )
		return;
}

auto CInterfaceManager::OnLevelInitHook( ) -> void
{
	GetGlobalVars( );
	GetGameParticleManager( );
}

auto CInterfaceManager::OnLevelShutdownHook( ) -> void
{
	m_pGlobalVars = nullptr;
	m_pGameParticleManager = nullptr;
}

auto CInterfaceManager::GetMemAlloc( ) -> IMemAlloc*
{
	if ( !m_pMemAlloc )
		m_pMemAlloc = *reinterpret_cast< IMemAlloc** >( GetMemoryManager( )->GetExportAddress( MODULES::pTier0DLL, _xor( "g_pMemAlloc" ) ) );

	if ( !m_pMemAlloc )
		L_PRINT( LOG_ERROR ) << _xor( "Failed to get \"m_pMemAlloc\" address" );

	return m_pMemAlloc;
}

auto CInterfaceManager::GetSchemaSystem( ) -> CSchemaSystem*
{
	if ( !m_pSchemaSystem )
	{
		const auto pSchemaSystemRegisterList = GetRegisterList( MODULES::pSchemaSystemDLL );
		if ( pSchemaSystemRegisterList )
			m_pSchemaSystem = Capture<CSchemaSystem>( pSchemaSystemRegisterList, SCHEMA_SYSTEM );
	}

	if ( !m_pSchemaSystem )
		L_PRINT( LOG_ERROR ) << _xor( "Failed to get \"m_pSchemaSystem\" address" );

	return m_pSchemaSystem;
}

auto CInterfaceManager::GetSwapChain( ) -> ISwapChainDx11*
{
	if ( !m_pSwapChain )
	{
		m_pSwapChain = **reinterpret_cast< ISwapChainDx11*** >( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Interface::ISwapChainDx11" ) ).GetAddress( ) );

		if ( m_pSwapChain )
		{
			if ( FAILED( m_pSwapChain->pDXGISwapChain->GetDevice( __uuidof( ID3D11Device ), reinterpret_cast< void** >( &pDevice ) ) ) )
			{
				L_PRINT( LOG_ERROR ) << _xor( "Failed to get device from m_pSwapChain" );
				m_pSwapChain = nullptr;
			}
			else
				pDevice->GetImmediateContext( &pDeviceContext );
		}
	}

	if ( !m_pSwapChain )
		L_PRINT( LOG_ERROR ) << _xor( "Failed to get \"m_pSwapChain\" address" );

	return m_pSwapChain;
}

auto CInterfaceManager::GetInputSystem( ) -> IInputSystem*
{
	if ( !m_pInputSystem )
	{
		const auto pInputSystemRegisterList = GetRegisterList( MODULES::pInputSystemDLL );
		if ( pInputSystemRegisterList )
			m_pInputSystem = Capture<IInputSystem>( pInputSystemRegisterList, INPUT_SYSTEM_VERSION );
	}

	if ( !m_pInputSystem )
		L_PRINT( LOG_ERROR ) << _xor( "Failed to get \"m_pInputSystem\" address" );

	return m_pInputSystem;
}

auto CInterfaceManager::GetGameEntitySystem( ) -> CGameEntitySystem*
{
	if ( !m_pGameEntitySystem )
		m_pGameEntitySystem = *reinterpret_cast< CGameEntitySystem** >( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Interface::CGameEntitySystem" ) ).GetAddress( ) );

	if ( !m_pGameEntitySystem )
		L_PRINT( LOG_ERROR ) << _xor( "Failed to get \"m_pGameEntitySystem\" address" );

	return m_pGameEntitySystem;
}

auto CInterfaceManager::GetGlobalVars( ) -> IGlobalVars*
{
	if ( !m_pGlobalVars )
	{
		const auto GlobalVarsPattern = GetPatternManager( )->GetPattern( FNV1A::HashConst( "Interface::IGlobalVars" ) );
		m_pGlobalVars.store( *reinterpret_cast< IGlobalVars** >( GetMemoryManager( )->ResolveRelativeAddress( GetMemoryManager( )->FindPattern( CLIENT_DLL, GlobalVarsPattern.szPattern.c_str( ) ), 0x3, 0x7 ) ) );
	}

	if ( !m_pGlobalVars )
		L_PRINT( LOG_ERROR ) << _xor( "Failed to get \"m_pGlobalVars\" address" );

	return m_pGlobalVars.load( );
}

auto CInterfaceManager::GetSource2Client( ) -> ISource2Client*
{
	if ( !m_pSource2Client )
	{
		const auto pClientRegister = GetRegisterList( MODULES::pClientDLL );
		if ( pClientRegister )
			m_pSource2Client = Capture<ISource2Client>( pClientRegister, SOURCE2_CLIENT );
	}

	if ( !m_pSource2Client )
		L_PRINT( LOG_ERROR ) << _xor( "Failed to get \"m_pSource2Client\" address" );

	return m_pSource2Client;
}

auto CInterfaceManager::GetEngineClient( ) -> IEngineClient*
{
	if ( !m_pEngineClient )
	{
		const auto pEngineRegisterList = GetRegisterList( MODULES::pEngine2DLL );
		if ( pEngineRegisterList )
			m_pEngineClient = Capture<IEngineClient>( pEngineRegisterList, SOURCE2_ENGINE_TO_CLIENT );
	}

	if ( !m_pEngineClient )
		L_PRINT( LOG_ERROR ) << _xor( "Failed to get \"m_pEngineClient\" address" );

	return m_pEngineClient;
}

auto CInterfaceManager::GetResourceSystem( ) -> CResourceSystem*
{
	if ( !m_pResourceSystem )
	{
		const auto pResourceSystemRegisterList = GetRegisterList( MODULES::pResourceSystemDLL );
		if ( pResourceSystemRegisterList )
			m_pResourceSystem = Capture<CResourceSystem>( pResourceSystemRegisterList, RESOURCE_SYSTEM );
	}

	if ( !m_pResourceSystem )
		L_PRINT( LOG_ERROR ) << _xor( "Failed to get \"m_pResourceSystem\" address" );

	return m_pResourceSystem;
}

auto CInterfaceManager::GetEngineCVar( ) -> IEngineCVar*
{
	if ( !m_pEngineCVar )
	{
		const auto pTier0RegisterList = GetRegisterList( MODULES::pTier0DLL );
		if ( pTier0RegisterList )
			m_pEngineCVar = Capture<IEngineCVar>( pTier0RegisterList, ENGINE_CVAR );
	}

	if ( !m_pEngineCVar )
		L_PRINT( LOG_ERROR ) << _xor( "Failed to get \"m_pEngineCVar\" address" );


	return m_pEngineCVar;
}

auto CInterfaceManager::GetGameEventManager( ) -> CGameEventManager*
{
	if ( !m_pGameEventManager )
		m_pGameEventManager = *reinterpret_cast< CGameEventManager** >( GetMemoryManager( )->ResolveRelativeAddress( GetMemoryManager( )->GetVFunc<std::uint8_t*>( GetSource2Client( ), 14U ) + 0x3E, 0x3, 0x7 ) );

	if ( !m_pGameEventManager )
		L_PRINT( LOG_ERROR ) << _xor( "Failed to get \"m_pGameEventManager\" address" );


	return m_pGameEventManager;
}

auto CInterfaceManager::GetGameParticleManager( ) -> CGameParticleManager*
{
	if ( !m_pGameParticleManager )
	{
		const auto GameParticleManagerPattern = GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::GetGameParticleSystem" ) );
		m_pGameParticleManager = reinterpret_cast< decltype( FunctionsCS2::FnGetGameParticleSystem ) >( GetMemoryManager( )->FindPattern( CLIENT_DLL, GameParticleManagerPattern.szPattern.c_str( ) ) )( );
	}

	if ( !m_pGameParticleManager )
		L_PRINT( LOG_ERROR ) << _xor( "Failed to get \"m_pGameParticleManager\" address" );


	return m_pGameParticleManager;
}

auto CInterfaceManager::GetParticleSystemManager( ) -> IParticleSystemMgr*
{
	if ( !m_pParticleSystemMgr )
	{
		const auto pParticlesRegisterList = GetRegisterList( MODULES::pParticlesDLL );
		if ( pParticlesRegisterList )
			m_pParticleSystemMgr = Capture<IParticleSystemMgr>( pParticlesRegisterList, PARTICLE_SYSTEM_MANAGER );
	}

	if ( !m_pParticleSystemMgr )
		L_PRINT( LOG_ERROR ) << _xor( "Failed to get \"m_pParticleSystemMgr\" address" );

	return m_pParticleSystemMgr;
}

auto CInterfaceManager::GetNetworkClientService( ) -> INetworkClientService*
{
	if ( !m_pNetworkClientService )
	{
		const auto pEngineRegisterList = GetRegisterList( MODULES::pEngine2DLL );
		if ( pEngineRegisterList )
			m_pNetworkClientService = Capture<INetworkClientService>( pEngineRegisterList, NETWORK_CLIENT_SERVICE );
	}

	if ( !m_pNetworkClientService )
		L_PRINT( LOG_ERROR ) << _xor( "Failed to get \"m_pNetworkClientService\" address" );

	return m_pNetworkClientService;
}

auto CInterfaceManager::GetCIPrediction( ) -> CIPrediction*
{
	if ( !m_pCIPrediction )
	{
		const auto pClientRegister = GetRegisterList( MODULES::pClientDLL );
		if ( pClientRegister )
			m_pCIPrediction = Capture<CIPrediction>( pClientRegister, C_PREDICTION );
	}

	if ( !m_pCIPrediction )
		L_PRINT( LOG_ERROR ) << _xor( "Failed to get \"m_pCIPrediction\" address" );

	return m_pCIPrediction;
}


auto CInterfaceManager::CreateRenderTarget( ) -> void
{
	if ( FAILED( GetSwapChain( )->pDXGISwapChain->GetDevice( __uuidof( ID3D11Device ), ( void** )&pDevice ) ) )
	{
		L_PRINT( LOG_ERROR ) << _xor( "Failed to get device from swapchain" );
	}
	else
		// we successfully got device, so we can get immediate context
		pDevice->GetImmediateContext( &pDeviceContext );

	// @note: i dont use this anywhere else so lambda is fine
	static const auto GetCorrectDXGIFormat = [ ]( DXGI_FORMAT eCurrentFormat )
	{
		switch ( eCurrentFormat )
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		}

		return eCurrentFormat;
	};

	DXGI_SWAP_CHAIN_DESC sd;
	GetSwapChain( )->pDXGISwapChain->GetDesc( &sd );

	ID3D11Texture2D* pBackBuffer = nullptr;
	if ( SUCCEEDED( GetSwapChain( )->pDXGISwapChain->GetBuffer( 0, IID_PPV_ARGS( &pBackBuffer ) ) ) )
	{
		if ( pBackBuffer )
		{
			D3D11_RENDER_TARGET_VIEW_DESC desc{};
			desc.Format = static_cast< DXGI_FORMAT >( GetCorrectDXGIFormat( sd.BufferDesc.Format ) );
			desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			if ( FAILED( pDevice->CreateRenderTargetView( pBackBuffer, &desc, &pRenderTargetView ) ) )
			{
				L_PRINT( LOG_WARNING ) << _xor( "failed to create render target view with D3D11_RTV_DIMENSION_TEXTURE2D..." );
				L_PRINT( LOG_INFO ) << _xor( "retrying to create render target view with D3D11_RTV_DIMENSION_TEXTURE2DMS..." );
				desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
				if ( FAILED( pDevice->CreateRenderTargetView( pBackBuffer, &desc, &pRenderTargetView ) ) )
				{
					L_PRINT( LOG_WARNING ) << _xor( "failed to create render target view with D3D11_RTV_DIMENSION_TEXTURE2D..." );
					L_PRINT( LOG_INFO ) << _xor( "retrying..." );
					if ( FAILED( pDevice->CreateRenderTargetView( pBackBuffer, NULL, &pRenderTargetView ) ) )
					{
						L_PRINT( LOG_ERROR ) << _xor( "failed to create render target view" );
					}
				}
			}
			pBackBuffer->Release( );
			pBackBuffer = nullptr;
		}
	}
}

auto CInterfaceManager::DestroyRenderTarget( ) -> void
{
	if ( pRenderTargetView )
	{
		pRenderTargetView->Release( );
		pRenderTargetView = nullptr;
	}
}

auto GetInterfaceManager( ) -> CInterfaceManager*
{
	return &g_CInterfaceManager;
}
