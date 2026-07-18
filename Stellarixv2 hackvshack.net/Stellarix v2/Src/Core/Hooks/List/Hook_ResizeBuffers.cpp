#include "Hook_ResizeBuffers.hpp"
#include <ImGui/imgui_impl_dx11.h>
#include <Core/Interfaces/InterfaceManager.hpp>
#include <Launcher.hpp>


auto __fastcall Hook_ResizeBuffers( IDXGISwapChain* pSwapChain, std::uint32_t nBufferCount, std::uint32_t nWidth, std::uint32_t nHeight, DXGI_FORMAT newFormat, std::uint32_t nFlags ) -> HRESULT
{
	if ( !GetCheatLauncher( )->CheatIsAlive( ) )
		return ResizeBuffers_o.fastcall<HRESULT>( pSwapChain, nBufferCount, nWidth, nHeight, newFormat, nFlags );

	if ( GetInterfaceManager( )->pRenderTargetView )
	{
		GetInterfaceManager( )->pDeviceContext->OMSetRenderTargets( 0, NULL, NULL );
		GetInterfaceManager( )->pRenderTargetView->Release( );
		GetInterfaceManager( )->pRenderTargetView = nullptr;
	}

	ImGui_ImplDX11_InvalidateDeviceObjects( );

	auto hResult = ResizeBuffers_o.fastcall<HRESULT>( pSwapChain, nBufferCount, nWidth, nHeight, newFormat, nFlags );
	if ( SUCCEEDED( hResult ) )
	{
		GetInterfaceManager( )->CreateRenderTarget( );
		ImGui_ImplDX11_CreateDeviceObjects( );
	}

	return hResult;
}
