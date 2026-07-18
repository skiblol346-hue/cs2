#include "Renderer.hpp"
#include <Assets/Fonts/Inter/InterBold.hpp>
#include <Assets/Fonts/Verdana/Verdana.hpp>
#include <Assets/Fonts/WeaponIcons/WeaponIcons.hpp>
#include <Assets/Images/WatermarkUserBytes.hpp>
#include <Core/Interfaces/InterfaceManager.hpp>
#include <Core/Memory/Memory.hpp>
#include <Core/StellarixClient.hpp>
#include <cstddef>
#include <ImGui/imgui_freetype.h>
#include <ImGui/imgui_impl_dx11.h>
#include <ImGui/imgui_impl_win32.h>
#include <Launcher.hpp>

#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <Assets/Fonts/Awesome5/Awesome5.hpp>
#include <Assets/Fonts/Awesome5/Awesome5_Name.hpp>
#include <Assets/Images/UserTestCat.hpp>
#include <Core/Utils/CDX11ImageHelper/CDX11ImageHelper.hpp>
#include <Core/Sdk/Datatypes/Matrix.hpp>
#include <Core/Sdk/Globals.hpp>
#include <Assets/Images/Ghost.hpp>
#include <Core/Engine/FuncrionCS2.hpp>


static CRenderer g_CRenderer{ };

static BOOL CALLBACK EnumWindowsCallback( HWND handle, LPARAM lParam )
{
	const auto MainWindow = [handle]( )
	{
		return GetWindow( handle, GW_OWNER ) == nullptr &&
			IsWindowVisible( handle ) && handle != GetConsoleWindow( );
	};

	DWORD nPID = 0;
	GetWindowThreadProcessId( handle, &nPID );

	if ( GetCurrentProcessId( ) != nPID || !MainWindow( ) )
		return TRUE;

	*reinterpret_cast< HWND* >( lParam ) = handle;
	return FALSE;
}

auto WindowProcess( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) -> long
{
	if ( GetStellarixClient( )->WindowProcess( hWnd, uMsg, wParam, lParam ) )
		return 1L;

	return ::CallWindowProcW( GetRenderer( )->pOldWndProc, hWnd, uMsg, wParam, lParam );
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

// thread-safe draw data mutex
static SRWLOCK drawLock = {};

static void* __cdecl ImGuiAllocWrapper( const std::size_t nSize, [[maybe_unused]] void* pUserData = nullptr )
{
	return GetMemoryManager( )->HeapAlloc( nSize );
}

static void __cdecl ImGuiFreeWrapper( void* pMemory, [[maybe_unused]] void* pUserData = nullptr ) noexcept
{
	GetMemoryManager( )->HeapFree( pMemory );
}


auto CRenderer::OnInit( ) -> bool
{
	if ( m_bInitialized )
		return true;

	while ( !hWindow )
	{
		EnumWindows( ::EnumWindowsCallback, reinterpret_cast< LPARAM >( &hWindow ) );
		::Sleep( 200U );
	}

	// change window message handle to our
	pOldWndProc = reinterpret_cast< WNDPROC >( SetWindowLongPtrW( hWindow, GWLP_WNDPROC, reinterpret_cast< LONG_PTR >( WindowProcess ) ) );
	if ( !pOldWndProc )
		return false;

	ImGui::SetAllocatorFunctions( ImGuiAllocWrapper, ImGuiFreeWrapper );
	ImGui::SetCurrentContext( ImGui::CreateContext( ) );
	if ( !ImGui_ImplWin32_Init( hWindow ) )
		return false;
	if ( !ImGui_ImplDX11_Init( GetInterfaceManager( )->pDevice, GetInterfaceManager( )->pDeviceContext ) )
		return false;

	m_bInitialized = InitFonts( ) && InitShaders( );
	return m_bInitialized;
}

auto CRenderer::OnDestroy( ) -> void
{
	if ( !m_bInitialized )
		return;
	m_bInitialized = false;

	ImGui_ImplDX11_Shutdown( );
	ImGui_ImplWin32_Shutdown( );
	ImGui::DestroyContext( );
}

auto CRenderer::RenderDrawData( ImDrawData* pDrawData ) -> void
{
	if ( ::TryAcquireSRWLockExclusive( &drawLock ) )
	{
		*pDrawListRender = *pDrawListSafe;
		::ReleaseSRWLockExclusive( &drawLock );
	}

	if ( pDrawListRender->CmdBuffer.empty( ) )
		return;

	// remove trailing command if unused
	// @note: equivalent to  pDrawList->_PopUnusedDrawCmd()
	if ( const ImDrawCmd& lastCommand = pDrawListRender->CmdBuffer.back( ); lastCommand.ElemCount == 0 && lastCommand.UserCallback == nullptr )
	{
		pDrawListRender->CmdBuffer.pop_back( );
		if ( pDrawListRender->CmdBuffer.empty( ) )
			return;
	}

	ImGuiContext* pContext = ImGui::GetCurrentContext( );
	ImGuiViewportP* pViewport = pContext->Viewports[ 0 ];
	ImVector<ImDrawList*>* vecDrawLists = pViewport->DrawDataBuilder.Layers[ 0 ];
	vecDrawLists->push_front( pDrawListRender ); // this one being most background

	pDrawData->CmdLists.push_front( pDrawListRender );
	pDrawData->CmdListsCount = vecDrawLists->Size;
	pDrawData->TotalVtxCount += pDrawListRender->VtxBuffer.Size;
	pDrawData->TotalIdxCount += pDrawListRender->IdxBuffer.Size;
}

static ImDrawListFlags imDrawListOldFlags;
auto CRenderer::NewFrame( ) -> void
{
	ImGui_ImplDX11_NewFrame( );
	ImGui_ImplWin32_NewFrame( );
	ImGui::NewFrame( );

	imDrawListOldFlags = ImGui::GetBackgroundDrawList( )->Flags;
	ImGui::GetBackgroundDrawList( )->Flags = ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines | ImDrawListFlags_AntiAliasedFill;
}

auto CRenderer::Render( ) -> void
{
	ImGui::GetBackgroundDrawList( )->Flags = imDrawListOldFlags;

	ImGui::Render( );
	//RenderDrawData( ImGui::GetDrawData( ) );
	ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData( ) );
}

auto CRenderer::InitFonts( ) -> bool
{
	ImFontConfig imVerdanaConfig;
	imVerdanaConfig.FontDataOwnedByAtlas = false;
	imVerdanaConfig.FontLoaderFlags = ImGuiFreeTypeLoaderFlags_Bold;

	ImFontConfig imInterBOLDConfig;
	imInterBOLDConfig.FontLoaderFlags = ImGuiFreeTypeLoaderFlags_ForceAutoHint;
	imInterBOLDConfig.FontDataOwnedByAtlas = false;

	ImFontConfig imWeaponIconsConfig;
	imWeaponIconsConfig.FontLoaderFlags = ImGuiFreeTypeLoaderFlags_ForceAutoHint;
	imWeaponIconsConfig.FontDataOwnedByAtlas = false;


	const ImWchar Awesome5IconsRanges[ ] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	ImFontConfig imAwesome5IconsConfig;
	imAwesome5IconsConfig.MergeMode = true;
	imAwesome5IconsConfig.PixelSnapH = true;
	imAwesome5IconsConfig.FontDataOwnedByAtlas = false;


	ImGuiIO& io = ImGui::GetIO( );
	io.Fonts->AddFontDefault( );


	StellarixFonts::fontVerdana = io.Fonts->AddFontFromMemoryTTF( ( void* )VerdanaFontData, sizeof( VerdanaFontData ), 200.f, &imVerdanaConfig, io.Fonts->GetGlyphRangesCyrillic( ) );
	StellarixFonts::fontInterBOLD = io.Fonts->AddFontFromMemoryTTF( ( void* )InterBoldFontData, sizeof( InterBoldFontData ), 200.f, &imInterBOLDConfig, io.Fonts->GetGlyphRangesCyrillic( ) );
	StellarixFonts::fontWeaponIcons = io.Fonts->AddFontFromMemoryTTF( ( void* )WeaponIconsFontData, sizeof( WeaponIconsFontData ), 200.f, &imWeaponIconsConfig, io.Fonts->GetGlyphRangesDefault( ) );
	StellarixFonts::fontAwesome5 = io.Fonts->AddFontFromMemoryTTF( ( void* )AwesomeSolid_900FontData, sizeof( AwesomeSolid_900FontData ), 200.f, &imAwesome5IconsConfig, Awesome5IconsRanges );


	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	return io.Fonts->Build( );
}

auto CRenderer::InitShaders( ) -> bool
{
	int w, h;

	if ( StellarixShaders::shaderWatermarkUser == nullptr )
		StellarixShaders::shaderWatermarkUser = GetDX11ImageHelper( )->LoadPNGTextureFromMemory( WatermarkUserBytes, sizeof( WatermarkUserBytes ), &w, &h );

	if ( StellarixShaders::shaderUserTestCat == nullptr )
		StellarixShaders::shaderUserTestCat = GetDX11ImageHelper( )->LoadPNGTextureFromMemory( UserTestCatBytes, sizeof( UserTestCatBytes ), &w, &h );

	if ( StellarixShaders::shaderGhost == nullptr )
		StellarixShaders::shaderGhost = GetDX11ImageHelper( )->LoadPNGTextureFromMemory( GhostBytes, sizeof( GhostBytes ), &w, &h );

	return true;
}

auto CRenderer::WorldToScreen( const Vector_t& vecOrigin, ImVec2* pVecScreen ) -> bool
{
	ViewMatrix_t viewMatrix = CheatData.viewMatrix;
	const float flWidth = viewMatrix[ 3 ][ 0 ] * vecOrigin.x + viewMatrix[ 3 ][ 1 ] * vecOrigin.y + viewMatrix[ 3 ][ 2 ] * vecOrigin.z + viewMatrix[ 3 ][ 3 ];
	// check is point can't fit on screen, because it's behind us
	if ( flWidth < 0.001f )
		return false;

	// compute the scene coordinates of a point in 3D
	const float flInverse = 1.0f / flWidth;
	pVecScreen->x = ( viewMatrix[ 0 ][ 0 ] * vecOrigin.x + viewMatrix[ 0 ][ 1 ] * vecOrigin.y + viewMatrix[ 0 ][ 2 ] * vecOrigin.z + viewMatrix[ 0 ][ 3 ] ) * flInverse;
	pVecScreen->y = ( viewMatrix[ 1 ][ 0 ] * vecOrigin.x + viewMatrix[ 1 ][ 1 ] * vecOrigin.y + viewMatrix[ 1 ][ 2 ] * vecOrigin.z + viewMatrix[ 1 ][ 3 ] ) * flInverse;

	// screen transform
	// get the screen position in pixels of given point
	const ImVec2 vecDisplaySize = ImGui::GetIO( ).DisplaySize;
	pVecScreen->x = ( vecDisplaySize.x * 0.5f ) + ( pVecScreen->x * vecDisplaySize.x ) * 0.5f;
	pVecScreen->y = ( vecDisplaySize.y * 0.5f ) - ( pVecScreen->y * vecDisplaySize.y ) * 0.5f;

	return true;
}

/*__forceinline auto CRenderer::WorldToScreen( const Vector_t& vecOrigin, ImVec2* pVecScreen ) -> bool
{
	ViewMatrix_t viewMatrix = CheatData.viewMatrix;
	const float flWidth = viewMatrix[ 3 ][ 0 ] * vecOrigin.x + viewMatrix[ 3 ][ 1 ] * vecOrigin.y + viewMatrix[ 3 ][ 2 ] * vecOrigin.z + viewMatrix[ 3 ][ 3 ];

	// check is point can't fit on screen, because it's behind us
	if ( flWidth < 0.001f )
		return false;

	// compute the scene coordinates of a point in 3D
	const float flInverse = 1.0f / flWidth;
	pVecScreen->x = ( viewMatrix[ 0 ][ 0 ] * vecOrigin.x + viewMatrix[ 0 ][ 1 ] * vecOrigin.y + viewMatrix[ 0 ][ 2 ] * vecOrigin.z + viewMatrix[ 0 ][ 3 ] ) * flInverse;
	pVecScreen->y = ( viewMatrix[ 1 ][ 0 ] * vecOrigin.x + viewMatrix[ 1 ][ 1 ] * vecOrigin.y + viewMatrix[ 1 ][ 2 ] * vecOrigin.z + viewMatrix[ 1 ][ 3 ] ) * flInverse;

	// screen transform
	// get the screen position in pixels of given point
	const ImVec2 vecDisplaySize = ImGui::GetIO( ).DisplaySize;
	pVecScreen->x = ( vecDisplaySize.x * 0.5f ) + ( pVecScreen->x * vecDisplaySize.x ) * 0.5f;
	pVecScreen->y = ( vecDisplaySize.y * 0.5f ) - ( pVecScreen->y * vecDisplaySize.y ) * 0.5f;

	return true;
}*/

auto GetRenderer( ) -> CRenderer*
{
	return &g_CRenderer;
}
