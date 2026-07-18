#pragma once
#include <d3d11.h>
#include <memory>
#include <atomic>
// TODO: don`t include (interfaces).hpp

class IMemAlloc;
class CSchemaSystem;
class ISwapChainDx11;
class IInputSystem;
class CGameEntitySystem;
class IGlobalVars;
class ISource2Client;
class IEngineClient;
class CUtlBuffer;
class CResourceSystem;
class IEngineCVar;
class CGameEventManager;
class CGameParticleManager;
class IParticleSystemMgr;
class INetworkClientService;
class CIPrediction;


class CInterfaceManager
{
public:
	CInterfaceManager( ) = default;
	~CInterfaceManager( ) = default;

public:
	auto OnInit( ) -> bool;
	auto OnDestroy( ) -> void;

public:
	auto OnLevelInitHook( ) -> void;
	auto OnLevelShutdownHook( ) -> void;


public:
	auto GetMemAlloc( ) -> IMemAlloc*;
	auto GetSchemaSystem( ) -> CSchemaSystem*;
	auto GetSwapChain( ) -> ISwapChainDx11*;
	auto GetInputSystem( ) -> IInputSystem*;
	auto GetGameEntitySystem( ) -> CGameEntitySystem*;
	auto GetGlobalVars( ) -> IGlobalVars*;
	auto GetSource2Client( ) -> ISource2Client*;
	auto GetEngineClient( ) -> IEngineClient*;
	auto GetResourceSystem( ) -> CResourceSystem*;
	auto GetEngineCVar( ) -> IEngineCVar*;
	auto GetGameEventManager( ) -> CGameEventManager*;
	auto GetGameParticleManager( ) -> CGameParticleManager*;
	auto GetParticleSystemManager( ) -> IParticleSystemMgr*;
	auto GetNetworkClientService( ) -> INetworkClientService*;
	auto GetCIPrediction( ) -> CIPrediction*;

public:
	auto CreateRenderTarget( ) -> void;
	auto DestroyRenderTarget( ) -> void;

public:
	ID3D11Device* pDevice = nullptr;
	ID3D11DeviceContext* pDeviceContext = nullptr;
	ID3D11RenderTargetView* pRenderTargetView = nullptr;

private:
	IMemAlloc* m_pMemAlloc = nullptr;
	CSchemaSystem* m_pSchemaSystem = nullptr;
	ISwapChainDx11* m_pSwapChain = nullptr;
	IInputSystem* m_pInputSystem = nullptr;
	CGameEntitySystem* m_pGameEntitySystem = nullptr;
	std::atomic<IGlobalVars*> m_pGlobalVars = nullptr;
	ISource2Client* m_pSource2Client = nullptr;
	IEngineClient* m_pEngineClient = nullptr;
	CResourceSystem* m_pResourceSystem = nullptr;
	IEngineCVar* m_pEngineCVar = nullptr;
	CGameEventManager* m_pGameEventManager = nullptr;
	CGameParticleManager* m_pGameParticleManager = nullptr;
	IParticleSystemMgr* m_pParticleSystemMgr = nullptr;
	INetworkClientService* m_pNetworkClientService = nullptr;
	CIPrediction* m_pCIPrediction = nullptr;

private:
	bool m_bInitialized = false;
};

auto GetInterfaceManager( ) -> CInterfaceManager*;

