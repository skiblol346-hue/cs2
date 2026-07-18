#include "CDX11ImageHelper.hpp"
#include <stb_image.h>
#include <d3d11.h>
#include <Core/Interfaces/InterfaceManager.hpp>


static CDX11ImageHelper g_CDX11ImageHelper{};

auto CDX11ImageHelper::LoadPNGTextureFromMemory( const void* pBuffer, size_t BufferSize, int* w, int* h ) -> ID3D11ShaderResourceView*
{
	ID3D11ShaderResourceView* pD3D11ShaderResourceView = nullptr;

	auto pstbImage = stbi_load_from_memory( reinterpret_cast< const unsigned char* >( pBuffer ), static_cast< int >( BufferSize ), w, h, nullptr, 4 );
	if ( !pstbImage )
		return nullptr;

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory( &desc, sizeof( desc ) );

	desc.Width = *w;
	desc.Height = *h;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;

	ID3D11Texture2D* pTexture = NULL;
	D3D11_SUBRESOURCE_DATA subResource;

	subResource.pSysMem = pstbImage;
	subResource.SysMemPitch = desc.Width * 4;
	subResource.SysMemSlicePitch = 0;

	GetInterfaceManager( )->pDevice->CreateTexture2D( &desc, &subResource, &pTexture );

	// Create texture view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory( &srvDesc, sizeof( srvDesc ) );

	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;

	if ( pTexture )
	{
		GetInterfaceManager( )->pDevice->CreateShaderResourceView( pTexture, &srvDesc, &pD3D11ShaderResourceView );
		pTexture->Release( );
	}

	stbi_image_free( pstbImage );

	return pD3D11ShaderResourceView;
}

auto CDX11ImageHelper::LoadTextureFromMemory( const void* pBuffer, size_t BufferSize, int* w, int* h ) -> ID3D11ShaderResourceView*
{
	ID3D11ShaderResourceView* pD3D11ShaderResourceView = nullptr;

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory( &desc, sizeof( desc ) );

	desc.Width = *w;
	desc.Height = *h;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;

	ID3D11Texture2D* pTexture = NULL;
	D3D11_SUBRESOURCE_DATA subResource;

	subResource.pSysMem = pBuffer;
	subResource.SysMemPitch = desc.Width * 4;
	subResource.SysMemSlicePitch = 0;

	GetInterfaceManager( )->pDevice->CreateTexture2D( &desc, &subResource, &pTexture );

	// Create texture view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory( &srvDesc, sizeof( srvDesc ) );

	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;

	if ( pTexture )
	{
		GetInterfaceManager( )->pDevice->CreateShaderResourceView( pTexture, &srvDesc, &pD3D11ShaderResourceView );

		pTexture->Release( );
	}

	return pD3D11ShaderResourceView;
}

auto GetDX11ImageHelper( ) -> CDX11ImageHelper*
{
	return &g_CDX11ImageHelper;
}
