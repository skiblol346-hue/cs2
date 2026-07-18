#include "Common.hpp"
#include "Config/Variables.hpp"
#include "Engine/Convar/Convar.hpp"
#include "Hooks/List/Hook_CreateMovePredict.hpp"
#include "Hooks/List/Hook_FrameStageNotify.hpp"
#include "Hooks/List/Hook_IsRelativeMouseMode.hpp"
#include "Hooks/List/Hook_OverrideView.hpp"
#include "Interfaces/InterfaceManager.hpp"
#include "Interfaces/List/CCSGOInput.hpp"
#include "Interfaces/List/CGameEntitySystem.hpp"
#include "StellarixClient.hpp"
#include "Utils/InputManager/InputManager.hpp"
#include "Utils/Logger/Logger.hpp"
#include <Core/Interfaces/List/IEngineClient.hpp>
#include <Core/Interfaces/List/IEngineCVar.hpp>
#include <Core/Interfaces/List/IResourceSystem.hpp>
#include <Core/Interfaces/List/IMaterialSystem.hpp>
#include <Core/Sdk/Datatypes/CAggregateSceneObject.hpp>
#include <Core/Sdk/Datatypes/CGameEvents.hpp>
#include <Core/Sdk/Datatypes/CUserCmd.hpp>
#include <Core/Sdk/EntityCache/EntityCache.hpp>
#include <Core/Sdk/EntityData.hpp>
#include <Core/Sdk/Globals.hpp>
#include <Features/Features.hpp>
#include <ImGui/imgui.h>
#include <Launcher.hpp>
#include <LoadScreen/LoadScreen.hpp>
#include <StellarixUI.hpp>
#include <Features/Misc/ModelChanger/ModelChanger.hpp>


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );


static CStellarixClient g_CStellarixClient{ };

auto CStellarixClient::OnInit( ) -> bool
{
	if ( !GetStellarixUI( )->OnInit( ) )
		return false;
	L_PRINT( LOG_INFO ) << _xor( "Stellarix UI Initialized" );

	if ( !GetEntityCache( )->OnInit( ) )
		return false;
	L_PRINT( LOG_INFO ) << _xor( "Entity Cache Initialized" );

	if ( !GetFeatures( )->OnInit( ) )
		return false;
	L_PRINT( LOG_INFO ) << _xor( "Features Initialized" );

	if ( !GetGameEventListener( )->OnInit( ) )
		return false;
	L_PRINT( LOG_INFO ) << _xor( "GameEventListener Initialized" );


	CheatData.szLocalPlayerName = std::string( CONVAR::name->value.sz );


	m_bInitialized = true;
	return m_bInitialized;
}

auto CStellarixClient::OnDestroy( ) -> void
{
	if ( !m_bInitialized )
		return;
	m_bInitialized = false;

	GetStellarixUI( )->OnDestroy( );
	GetEntityCache( )->OnDestroy( );
	GetFeatures( )->OnDestroy( );
}

auto CStellarixClient::WindowProcess( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) -> long
{
	if ( !m_bInitialized )
		return false;

	// Storage Keys States
	GetInputManager( )->OnWindowProcess( hWnd, uMsg, wParam, lParam );

	// Check If Menu is Open
	if ( GetInputManager( )->IsKeyReleased( C_GET( unsigned int, Vars.nOpenMenuKey ) ) )
	{
		GetStellarixUI( )->SwitchMenuState( );
		IsRelativeMouseMode_o.unsafe_call<void*>( GetInterfaceManager( )->GetInputSystem( ), GetStellarixUI( )->bMainWindowOpened ? false : GetStellarixUI( )->bMainActive );
	}

	if ( GetInputManager( )->IsKeyReleased( C_GET( unsigned int, Vars.nCheatUnloadKey ) ) )
	{
		GetCheatLauncher( )->OnDestroy( );
		return true;
	}

	// handle ImGui's window messages and block game's input if menu is opened
	return ImGui_ImplWin32_WndProcHandler( hWnd, uMsg, wParam, lParam ) || GetStellarixUI( )->bMainWindowOpened;
}


static std::string getTimeFormatted( const std::string& format )
{
	auto now = std::chrono::system_clock::now( );
	auto time = std::chrono::system_clock::to_time_t( now );
	std::tm timeinfo;
	localtime_s( &timeinfo, &time );
	std::stringstream ss;
	ss << std::put_time( &timeinfo, format.c_str( ) );
	return ss.str( );
}

static float flGlobalTimer = 0.f;
static float flLastTimeUpdateFps = 0.f;
static float flLastTimeUpdateTime = 0.f;
auto CStellarixClient::UpdateWatermarkData( ImGuiIO& io ) -> void
{
	flGlobalTimer += io.DeltaTime;
	if ( flGlobalTimer - flLastTimeUpdateFps >= flFpsDeltaUpdate )
	{
		CheatData.iFps = io.Framerate;
		flLastTimeUpdateFps = flGlobalTimer;
	}
	if ( flGlobalTimer - flLastTimeUpdateTime >= flTimeDeltaUpdate )
	{
		std::string szNewTime = getTimeFormatted( "%H:%M" );
		static std::string szLastTime;
		if ( flTimeDeltaUpdate != 60 )
		{
			if ( szNewTime != szLastTime && !szLastTime.empty( ) )
				flTimeDeltaUpdate = 60;
		}

		szLastTime = szNewTime;
		CheatData.szCurrentTime = szLastTime;
		flLastTimeUpdateTime = flGlobalTimer;
	}
}


auto CStellarixClient::OnLoadConfig( ) -> void
{
	if ( !m_bInitialized )
		return;

	if ( C_GET( float, Vars.flViewModelFov ) == CFG_UNDEFINE_INT && CONVAR::viewmodel_fov )
		C_GET( float, Vars.flViewModelFov ) = CONVAR::viewmodel_fov->value.fl;
	if ( C_GET( float, Vars.flViewModelOffsetX ) == CFG_UNDEFINE_INT && CONVAR::viewmodel_offset_x )
		C_GET( float, Vars.flViewModelOffsetX ) = CONVAR::viewmodel_offset_x->value.fl;
	if ( C_GET( float, Vars.flViewModelOffsetY ) == CFG_UNDEFINE_INT && CONVAR::viewmodel_offset_y )
		C_GET( float, Vars.flViewModelOffsetY ) = CONVAR::viewmodel_offset_y->value.fl;
	if ( C_GET( float, Vars.flViewModelOffsetZ ) == CFG_UNDEFINE_INT && CONVAR::viewmodel_offset_z )
		C_GET( float, Vars.flViewModelOffsetZ ) = CONVAR::viewmodel_offset_z->value.fl;

	if ( CONVAR::r_fullscreen_gamma )
		CONVAR::r_fullscreen_gamma->value.fl = C_GET( float, Vars.flFullScreenGamma );
}

auto CStellarixClient::OnLevelInitHook( const char* szNewMap ) -> void
{
	L_PRINT( LOG_INFO ) << "LevelInit: [ " << szNewMap << " ]";

	GetInterfaceManager( )->OnLevelInitHook( );
	GetEntityCache( )->OnLevelInitHook( );
	GetFeatures( )->OnLevelInitHook( szNewMap );
}

auto CStellarixClient::OnLevelShutdownHook( ) -> void
{
	GetInterfaceManager( )->OnLevelShutdownHook( );
	GetEntityCache( )->OnLevelShutdownHook( );
	GetFeatures( )->OnLevelShutdownHook( );
}

auto CStellarixClient::OnAddEntityHook( CEntityInstance* pInstance, CBaseHandle hHandle ) -> void
{
	if ( !pInstance || !hHandle.IsValid( ) ) return;

	//L_PRINT( LOG_INFO ) << "OnAddEntity: [ " << pInstance->GetEntityClassName( ) << ", " << hHandle.ToInt( ) << " ]";
	GetEntityCache( )->OnAddEntityHook( pInstance, hHandle );
}

auto CStellarixClient::OnRemoveEntityHook( CEntityInstance* pInstance, CBaseHandle hHandle ) -> void
{
	if ( !pInstance || !hHandle.IsValid( ) ) return;

	//L_PRINT( LOG_INFO ) << "OnRemoveEntity: [ " << pInstance->GetEntityClassName( ) << ", " << hHandle.ToInt( ) << " ]";
	GetEntityCache( )->OnRemoveEntityHook( pInstance, hHandle );
}

auto CStellarixClient::OnPresentHook( ) -> void
{
	if ( !m_bInitialized )
		return;

	ImGuiIO& io = ImGui::GetIO( );
	ImGuiStyle& style = ImGui::GetStyle( );
	//CheatState.imScreenSize = io.DisplaySize;
	//io.FontGlobalScale = 2.f;

	GetLoadScreen( )->OnPresentHook( io, style, ImGui::GetForegroundDrawList( ) );
	if ( !GetLoadScreen( )->IsActive( ) )
	{
		UpdateWatermarkData( io );
		GetStellarixUI( )->OnPresentHook( io, style );
		GetFeatures( )->OnPresentHook( );
	}
}

auto CStellarixClient::OnFrameStageNotifyHook( void* pSource2Client, int nFrameStage ) -> void
{
	if ( !m_bInitialized )
		return FrameStageNotify_o.unsafe_fastcall<void>( pSource2Client, nFrameStage );

	GetEntityCache( )->OnFrameStageNotifyHook( nFrameStage );
	GetFeatures( )->OnFrameStageNotifyHook( nFrameStage );

	FrameStageNotify_o.unsafe_fastcall<void>( pSource2Client, nFrameStage );


	if ( nFrameStage == FRAME_RENDER_START && false )
	{
		static Color_t colBackup;
		static int iBackupCount;

		if ( C_GET( Color_t, Vars.colWorldColor ) != colBackup )
		{
			ResourceArray_t arr{};
			arr.nCount = 0;
			arr.aResources = nullptr;

			// @TODO: 0x74616D76    -  BA 76 6D 61 74 48 8B C8 E8   (materialsystem2.dll) 13.05.25
			// @TODO: 0x74736F7076  -  49 BF ? ? ? ? ? ? ? ? 41 BE  (materialsystem2.dll) 05.11.25
			GetInterfaceManager( )->GetResourceSystem( )->EnumMaterials( 0x74736F7076, &arr, 2 );

			if ( arr.nCount && arr.aResources )
			{
				colBackup = C_GET( Color_t, Vars.colWorldColor );

				for ( int i = 0; i < arr.nCount; i++ )
				{
					auto pmat = arr.aResources[ i ];
					if ( !pmat )
						continue;

					auto mat = *pmat;
					if ( !mat )
						continue;

					if ( !mat->GetName( ) )
						continue;

					std::string name = std::string( mat->GetName( ) );
					if ( name.find( "char" ) != std::string::npos )
						continue;

					if ( name.find( "weapon" ) != std::string::npos )
						continue;

					if ( name.find( "loadout" ) != std::string::npos )
						continue;

					if ( name.find( "gun" ) != std::string::npos )
						continue;

					if ( name.find( "inventory" ) != std::string::npos )
						continue;

					if ( name.find( "skin" ) != std::string::npos )
						continue;

					if ( name.find( "sky" ) != std::string::npos )
						continue;

					if ( name.find( "sun" ) != std::string::npos )
						continue;

					if ( name.find( "cloud" ) != std::string::npos )
						continue;

					/*if ( name.find( "materials/" ) != std::string::npos )
						continue;*/

					if ( name.find( "models/" ) != std::string::npos )
						continue;

					if ( name.find( "prop" ) != std::string::npos )
						continue;

					auto paramvColorTint = mat->FindParameter( "g_vColorTint" );
					if ( paramvColorTint )
					{
						paramvColorTint->vecValue = Vector4D_t{ C_GET( Color_t, Vars.colWorldColor ).r / 255.f, C_GET( Color_t, Vars.colWorldColor ).g / 255.f, C_GET( Color_t, Vars.colWorldColor ).b / 255.f, C_GET( Color_t, Vars.colWorldColor ).a / 255.f };
						mat->UpdateParameter( );
					}
				}
			}
		}
	}
}

auto CStellarixClient::OnCreateMovePredictHook( CCSGOInput* pInput, int nSlot, CUserCmd* pUserCmd ) -> void
{
	CreateMovePredict_o.unsafe_fastcall<void>( pInput, nSlot, pUserCmd );
	if ( !m_bInitialized )
		return;

	if ( auto pMutableBase = pUserCmd->csgoUserCmd.mutable_base( ); pMutableBase )
	{
		auto CmdViewAngles = pMutableBase->viewangles( );
		LocalPlayerData.angViewAngle = QAngle_t(
			CmdViewAngles.has_x( ) ? CmdViewAngles.x( ) : 0.f,
			CmdViewAngles.has_y( ) ? CmdViewAngles.y( ) : 0.f,
			CmdViewAngles.has_z( ) ? CmdViewAngles.z( ) : 0.f
		);
	}

	CheatData.bIsInGame = GetInterfaceManager( )->GetEngineClient( )->IsInGame( ) && GetInterfaceManager( )->GetEngineClient( )->IsConnected( );
	if ( CheatData.bIsInGame )
	{
		CheatData.flGVCurrentTime = GetInterfaceManager( )->GetGlobalVars( ) ? GetInterfaceManager( )->GetGlobalVars( )->flCurrentTime : 0;
		CheatData.flGVFrameTime = GetInterfaceManager( )->GetGlobalVars( ) ? GetInterfaceManager( )->GetGlobalVars( )->flFrameTime : 0.1f;

		CCSPlayerController* pLocalController = CCSPlayerController::GetLocalPlayerController( );
		C_CSPlayerPawn* pLocalPawn = C_CSPlayerPawn::GetPawnFromController( pLocalController );
		if ( pLocalPawn )
		{
			LocalPlayerData.vecEyePosition = pLocalPawn->GetEyePosition( );
			LocalPlayerData.vecOrigin = pLocalPawn->GetOrigin( );
			LocalPlayerData.iHealth = pLocalPawn->m_iHealth( );
		}

		GetFeatures( )->OnCreateMovePredictHook( pInput, pUserCmd );
	}
}

auto CStellarixClient::OnOverrideViewHook( void* pClientModeCSNormal, CViewSetup* pViewSetup ) -> void
{
	if ( !m_bInitialized )
		return OverrideView_o.unsafe_fastcall<void>( pClientModeCSNormal, pViewSetup );

	GetFeatures( )->OnPreOverrideViewHook( pViewSetup );
	OverrideView_o.unsafe_fastcall<void>( pClientModeCSNormal, pViewSetup );
	GetFeatures( )->OnPostOverrideViewHook( pViewSetup );
}

auto CStellarixClient::OnDrawArrayHook( void* pAnimatableSceneObjectDesc, void* pDx11, CMeshDrawPrimitive* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk ) -> void
{
	if ( !m_bInitialized )
		return;

	GetFeatures( )->OnDrawArrayHook( pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk );
}

auto CStellarixClient::OnRenderLightingHook( CAggregateSceneObjectLighting* pCAggregateSceneObjectLighting ) -> void
{
	if ( !m_bInitialized )
		return;

	if ( pCAggregateSceneObjectLighting )
	{
		Color_t colLighting = C_GET( Color_t, Vars.colLightingColor );
		float flLightingIntensity = C_GET( float, Vars.flLightingIntensity );

		pCAggregateSceneObjectLighting->flRed = colLighting.Base<COLOR_R>( ) * flLightingIntensity;
		pCAggregateSceneObjectLighting->flGreen = colLighting.Base<COLOR_G>( ) * flLightingIntensity;
		pCAggregateSceneObjectLighting->flBlue = colLighting.Base<COLOR_B>( ) * flLightingIntensity;
	}
}


auto CStellarixClient::OnPlayerHurtEvent( CGameEventHelper gameEventHelper, CGameEvent* event ) -> void
{
	if ( !m_bInitialized )
		return;

	if ( gameEventHelper.GetDamage( ) <= 0 )
		return;

	auto pPlayerController = gameEventHelper.GetPlayerController( );
	auto pAttackerController = gameEventHelper.GetAttackerController( );
	if ( !pPlayerController || !pAttackerController )
		return;

	auto pPlayerPawn = C_CSPlayerPawn::GetPawnFromController( pPlayerController );
	auto pAttackerPawn = C_CSPlayerPawn::GetPawnFromController( pAttackerController );
	if ( !pPlayerPawn || !pAttackerPawn )
		return;

	CCSPlayerController* pLocalController = CCSPlayerController::GetLocalPlayerController( );
	C_CSPlayerPawn* pLocalPawn = C_CSPlayerPawn::GetPawnFromController( pLocalController );
	if ( !pLocalController || !pLocalPawn )
		return;

	GetFeatures( )->OnPlayerHurtEvent( gameEventHelper, event, pPlayerController, pAttackerController, pPlayerPawn, pAttackerPawn, pLocalController, pLocalPawn );
}

auto CStellarixClient::OnPlayerDeathEvent( CGameEventHelper gameEventHelper, CGameEvent* event ) -> void
{
	if ( !m_bInitialized )
		return;

	auto pPlayerController = gameEventHelper.GetPlayerController( );
	auto pAttackerController = gameEventHelper.GetAttackerController( );
	if ( !pPlayerController || !pAttackerController )
		return;

	auto pPlayerPawn = GetInterfaceManager( )->GetGameEntitySystem( )->Get<C_CSPlayerPawn>( pPlayerController->m_hPawn( ) );
	auto pAttackerPawn = GetInterfaceManager( )->GetGameEntitySystem( )->Get<C_CSPlayerPawn>( pAttackerController->m_hPawn( ) );
	if ( !pPlayerPawn || !pAttackerPawn )
		return;
}

auto CStellarixClient::OnBulletImpactEvent( CGameEventHelper gameEventHelper, CGameEvent* event ) -> void
{
	if ( !m_bInitialized )
		return;

	auto pPlayerController = gameEventHelper.GetPlayerController( );
	if ( !pPlayerController || !pPlayerController->m_bIsLocalPlayerController( ) )
		return;

	auto pPlayerPawn = C_CSPlayerPawn::GetPawnFromController( pPlayerController );
	if ( !pPlayerPawn )
		return;

	GetFeatures( )->OnBulletImpactEvent( gameEventHelper, event, pPlayerController, pPlayerPawn );
}

auto CStellarixClient::OnPlayerSpawnEvent( CGameEventHelper gameEventHelper, CGameEvent* event ) -> void
{
	if ( !m_bInitialized )
		return;

	auto pPlayerController = gameEventHelper.GetPlayerController( );
	if ( !pPlayerController || !pPlayerController->m_bIsLocalPlayerController( ) )
		return;

	auto pPlayerPawn = GetInterfaceManager( )->GetGameEntitySystem( )->Get<C_CSPlayerPawn>( pPlayerController->m_hPawn( ) );
	if ( !pPlayerPawn )
		return;

	GetModelChanger( )->ChangeModel( pPlayerPawn );
	GetModelChanger( )->OnChangeModelInMenu( );
}

auto CStellarixClient::OnPlayerBeginNewMatchEvent( CGameEventHelper gameEventHelper, CGameEvent* event ) -> void
{
	if ( !m_bInitialized )
		return;

	GetEntityCache( )->OnPlayerBeginNewMatchEvent( );
}

auto CStellarixClient::OnGameEndEvent( CGameEventHelper gameEventHelper, CGameEvent* event ) -> void
{
	if ( !m_bInitialized )
		return;

	GetEntityCache( )->OnGameEndEvent( );
}

auto GetStellarixClient( ) -> CStellarixClient*
{
	return &g_CStellarixClient;
}
