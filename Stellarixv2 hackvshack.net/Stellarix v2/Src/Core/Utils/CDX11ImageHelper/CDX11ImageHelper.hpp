#pragma once

struct ID3D11ShaderResourceView;

class CDX11ImageHelper final
{
public:
	auto LoadPNGTextureFromMemory( const void* pBuffer, size_t BufferSize, int* w, int* h ) -> ID3D11ShaderResourceView*;
	auto LoadTextureFromMemory( const void* pBuffer, size_t BufferSize, int* w, int* h ) -> ID3D11ShaderResourceView*;
};

auto GetDX11ImageHelper( ) -> CDX11ImageHelper*;
