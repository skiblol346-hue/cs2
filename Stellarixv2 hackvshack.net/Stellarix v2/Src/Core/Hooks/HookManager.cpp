#include "HookManager.hpp"
#include <SafetyHook/safetyhook.hpp>
#include <Core/Utils/Logger/Logger.hpp>
#include <Core/Utils/PatternManager/PatternManager.hpp>

// List
#include "List/Hook_Present.hpp"
#include "List/Hook_ResizeBuffers.hpp"
#include "List/Hook_CreateSwapChain.hpp"
#include "List/Hook_MouseInputEnabled.hpp"
#include "List/Hook_IsRelativeMouseMode.hpp"
#include "List/Hook_OnAddEntity.hpp"
#include "List/Hook_OnRemoveEntity.hpp"
#include "List/Hook_OnLevelInit.hpp"
#include "List/Hook_OnLevelShutdown.hpp"
#include "List/Hook_GetMatrixForView.hpp"
#include "List/Hook_FrameStageNotify.hpp"
#include "List/Hook_ShowMessageBox.hpp"
#include "List/Hook_CreateMovePredict.hpp"
#include "List/Hook_RenderLegs.hpp"
#include "List/Hook_RenderCrosshair.hpp"
#include "List/Hook_MarkInterpolationLatchFlagsDirty.hpp"
#include "List/Hook_SetupViewModel.hpp"
#include "List/Hook_DrawTeamIntro.hpp"
#include "List/Hook_PopupEventHandle.hpp"
#include "List/Hook_RenderScopeOverlay.hpp"
#include "List/Hook_RenderLighting.hpp"
#include "List/Hook_DrawAggregateSceneObjectArray.hpp"
#include "List/Hook_ValidateCamera.hpp"
#include "List/Hook_CalculateFov.hpp"
#include "List/Hook_OverrideView.hpp"
#include "List/Hook_DrawArray.hpp"
#include "List/Hook_SetModel.hpp"



static CHookManager g_CHookManager{ };

auto CHookManager::OnInit( ) -> bool
{
	Present_o = safetyhook::create_inline( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::Present" ) ).GetAddress( ), reinterpret_cast< void* >( Hook_Present ) );
	ResizeBuffers_o = safetyhook::create_inline( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::ResizeBuffers" ) ).GetAddress( ), reinterpret_cast< void* >( Hook_ResizeBuffers ) );
	CreateSwapChain_o = safetyhook::create_inline( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::CreateSwapChain" ) ).GetAddress( ), reinterpret_cast< void* >( Hook_CreateSwapChain ) );
	MouseInputEnabled_o = safetyhook::create_inline( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::MouseInputEnabled" ) ).GetAddress( ), reinterpret_cast< void* >( Hook_MouseInputEnabled ) );
	IsRelativeMouseMode_o = safetyhook::create_inline( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::IsRelativeMouseMode" ) ).GetAddress( ), reinterpret_cast< void* >( Hook_IsRelativeMouseMode ) );
	
	//DrawAggregateSceneObjectArray_o = safetyhook::create_inline( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::DrawAggregateSceneObjectArray" ) ).GetAddress( ), reinterpret_cast< void* >( Hook_DrawAggregateSceneObjectArray ) );

	OnAddEntity_o = safetyhook::create_inline( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::OnAddEntity" ) ).GetAddress( ), reinterpret_cast< void* >( Hook_OnAddEntity ) );
	OnRemoveEntity_o = safetyhook::create_inline( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::OnRemoveEntity" ) ).GetAddress( ), reinterpret_cast< void* >( Hook_OnRemoveEntity ) );
	OnLevelInit_o = safetyhook::create_inline( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::OnLevelInit" ) ).GetAddress( ), reinterpret_cast< void* >( Hook_OnLevelInit ) );
	OnLevelShutdown_o = safetyhook::create_inline( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::OnLevelShutdown" ) ).GetAddress( ), reinterpret_cast< void* >( Hook_OnLevelShutdown ) );
	
	GetMatrixForView_o = safetyhook::create_inline( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::GetMatrixForView" ) ).GetAddress( ), reinterpret_cast< void* >( Hook_GetMatrixForView ) );
	FrameStageNotify_o = safetyhook::create_inline( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::FrameStageNotify" ) ).GetAddress( ), reinterpret_cast< void* >( Hook_FrameStageNotify ) );
	CreateMovePredict_o = safetyhook::create_inline( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::CreateMovePredict" ) ).GetAddress( ), reinterpret_cast< void* >( Hook_CreateMovePredict ) );
	ValidateCamera_o = safetyhook::create_inline( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::ValidateCamera" ) ).GetAddress( ), reinterpret_cast< void* >( Hook_ValidateCamera ) );
	DrawArray_o = safetyhook::create_inline( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::DrawArray" ) ).GetAddress( ), reinterpret_cast< void* >( Hook_DrawArray ) );
	SetModel_o = safetyhook::create_inline( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::SetModel" ) ).GetAddress( ), reinterpret_cast< void* >( Hook_SetModel ) );
	
	RenderLegs_o = safetyhook::create_inline( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::RenderLegs" ) ).GetAddress( ), reinterpret_cast< void* >( Hook_RenderLegs ) );
	RenderCrosshair_o = safetyhook::create_inline( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::RenderCrosshair" ) ).GetAddress( ), reinterpret_cast< void* >( Hook_RenderCrosshair ) );
	SetupViewModel_o = safetyhook::create_inline( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::SetupViewModel" ) ).GetAddress( ), reinterpret_cast< void* >( Hook_SetupViewModel ) );
	CalculateFov_o = safetyhook::create_inline( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::CalculateFov" ) ).GetAddress( ), reinterpret_cast< void* >( Hook_CalculateFov ) );
	OverrideView_o = safetyhook::create_inline( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::OverrideView" ) ).GetAddress( ), reinterpret_cast< void* >( Hook_OverrideView ) );
	
	MarkInterpolationLatchFlagsDirty_o = safetyhook::create_inline( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::MarkInterpolationLatchFlagsDirty" ) ).GetAddress( ), reinterpret_cast< void* >( Hook_MarkInterpolationLatchFlagsDirty ) );
	DrawTeamIntro_o = safetyhook::create_inline( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::DrawTeamIntro" ) ).GetAddress( ), reinterpret_cast< void* >( Hook_DrawTeamIntro ) );
	PopupEventHandle_o = safetyhook::create_inline( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::PopupEventHandle" ) ).GetAddress( ), reinterpret_cast< void* >( Hook_PopupEventHandle ) );
	RenderScopeOverlay_o = safetyhook::create_inline( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::RenderScopeOverlay" ) ).GetAddress( ), reinterpret_cast< void* >( Hook_RenderScopeOverlay ) );
	
	RenderLighting_o = safetyhook::create_inline( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::RenderLighting" ) ).GetAddress( ), reinterpret_cast< void* >( Hook_RenderLighting ) );

	ShowMessageBox_o = safetyhook::create_inline( GetPatternManager( )->GetPattern( FNV1A::HashConst( "Hook::ShowMessageBox" ) ).GetAddress( ), reinterpret_cast< void* >( Hook_ShowMessageBox ) );

	m_bInitialized = true;
	return m_bInitialized;
}

auto CHookManager::OnDestroy( ) -> void
{
	if ( !m_bInitialized )
		return;
	m_bInitialized = false;

	Present_o.reset( );
	ResizeBuffers_o.reset( );
	CreateSwapChain_o.reset( );
	MouseInputEnabled_o.reset( );
	IsRelativeMouseMode_o.reset( );

	//DrawAggregateSceneObjectArray_o.reset( );

	OnAddEntity_o.reset( );
	OnRemoveEntity_o.reset( );
	OnLevelInit_o.reset( );
	OnLevelShutdown_o.reset( );

	GetMatrixForView_o.reset( );
	FrameStageNotify_o.reset( );
	CreateMovePredict_o.reset( );
	ValidateCamera_o.reset( );
	DrawArray_o.reset( );
	SetModel_o.reset( );

	RenderLegs_o.reset( );
	RenderCrosshair_o.reset( );
	SetupViewModel_o.reset( );
	CalculateFov_o.reset( );
	OverrideView_o.reset( );

	MarkInterpolationLatchFlagsDirty_o.reset( );
	DrawTeamIntro_o.reset( );
	PopupEventHandle_o.reset( );
	RenderScopeOverlay_o.reset( );

	RenderLighting_o.reset( );

	ShowMessageBox_o.reset( );
}

auto GetHookManager( ) -> CHookManager*
{
	return &g_CHookManager;
}
