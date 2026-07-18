#pragma once
#include <windows.h>
#include <numeric>
#include <deque>
#include <ImGui/imgui.h>

class CEntityInstance;
class CBaseHandle;

class CMeshDrawPrimitive;
class CViewSetup;
class CCSGOInput;
class CAggregateSceneObjectLighting;
class CGameEventHelper;
class CGameEvent;
class CUserCmd;


class CStellarixClient
{
public:
	CStellarixClient( ) = default;
	~CStellarixClient( ) = default;

public:
	auto OnInit( ) -> bool;
	auto OnDestroy( ) -> void;


public:
	auto WindowProcess( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) -> long;

public:
	auto UpdateWatermarkData( ImGuiIO& io ) -> void;

public:
	auto OnLoadConfig( ) -> void;

public:
	auto OnLevelInitHook( const char* szNewMap ) -> void;
	auto OnLevelShutdownHook( ) -> void;
	auto OnAddEntityHook( CEntityInstance* pInstance, CBaseHandle hHandle ) -> void;
	auto OnRemoveEntityHook( CEntityInstance* pInstance, CBaseHandle hHandle ) -> void;
	auto OnPresentHook( ) -> void;
	auto OnFrameStageNotifyHook( void* pSource2Client, int nFrameStage ) -> void;
	auto OnCreateMovePredictHook( CCSGOInput* pInput, int nSlot, CUserCmd* pUserCmd ) -> void;
	auto OnOverrideViewHook( void* pClientModeCSNormal, CViewSetup* pViewSetup ) -> void;
	auto OnDrawArrayHook( void* pAnimatableSceneObjectDesc, void* pDx11, CMeshDrawPrimitive* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk ) -> void;
	auto OnRenderLightingHook( CAggregateSceneObjectLighting* pCAggregateSceneObjectLighting ) -> void;

public:
	auto OnPlayerHurtEvent( CGameEventHelper gameEventHelper, CGameEvent* event ) -> void;
	auto OnPlayerDeathEvent( CGameEventHelper gameEventHelper, CGameEvent* event ) -> void;
	auto OnBulletImpactEvent( CGameEventHelper gameEventHelper, CGameEvent* event ) -> void;
	auto OnPlayerSpawnEvent( CGameEventHelper gameEventHelper, CGameEvent* event ) -> void;
	auto OnPlayerBeginNewMatchEvent( CGameEventHelper gameEventHelper, CGameEvent* event ) -> void;
	auto OnGameEndEvent( CGameEventHelper gameEventHelper, CGameEvent* event ) -> void;

private:
	const float flFpsDeltaUpdate = 1.7;
	float flTimeDeltaUpdate = 1.f;

private:
	bool m_bInitialized = false;
};

auto GetStellarixClient( ) -> CStellarixClient*;