#include "Hook_CreateSwapChain.hpp"
#include <Core/Utils/Logger/Logger.hpp>
#include <Core/Interfaces/InterfaceManager.hpp>
#include <Launcher.hpp>

auto __stdcall Hook_CreateSwapChain( IDXGIFactory* pFactory, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain ) -> HRESULT
{
	if ( !GetCheatLauncher( )->CheatIsAlive( ) )
		return CreateSwapChain_o.fastcall<HRESULT>( pFactory, pDevice, pDesc, ppSwapChain );

	GetInterfaceManager( )->DestroyRenderTarget( );
	L_PRINT( LOG_INFO ) << _xor( "render target view has been destroyed" );

	return CreateSwapChain_o.fastcall<HRESULT>( pFactory, pDevice, pDesc, ppSwapChain );
}
