#include "Hook_Present.hpp"
#include <Core/Interfaces/InterfaceManager.hpp>
#include <Core/Interfaces/List/ISwapChain.hpp>
#include <Core/Renderer/Renderer.hpp>
#include <Core/StellarixClient.hpp>
#include <Launcher.hpp>


auto __stdcall Hook_Present( IDXGISwapChain* pSwapChain, UINT uSyncInterval, UINT uFlags ) -> HRESULT
{
	if ( !GetCheatLauncher( )->CheatIsAlive( ) )
		return Present_o.stdcall<HRESULT>( pSwapChain, uSyncInterval, uFlags );

	// Recreate if it's not valid
	if ( !GetInterfaceManager( )->pRenderTargetView )
		GetInterfaceManager( )->CreateRenderTarget( );

	// Set our render target
	if ( GetInterfaceManager( )->pRenderTargetView )
		GetInterfaceManager( )->pDeviceContext->OMSetRenderTargets( 1, &GetInterfaceManager( )->pRenderTargetView, nullptr );

	GetRenderer( )->NewFrame( );
	GetStellarixClient( )->OnPresentHook( );
	GetRenderer( )->Render( );

	return Present_o.stdcall<HRESULT>( pSwapChain, uSyncInterval, uFlags );
}
