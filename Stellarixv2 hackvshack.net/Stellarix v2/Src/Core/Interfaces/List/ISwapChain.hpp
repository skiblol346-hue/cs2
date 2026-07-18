#pragma once
#include <Core/Memory/Memory.hpp>

struct IDXGISwapChain;
class ISwapChainDx11
{
    MEM_PAD( 0x170 );
    IDXGISwapChain* pDXGISwapChain;

public:
    ISwapChainDx11( ) : pDXGISwapChain( nullptr )
    {
    }

    explicit ISwapChainDx11( IDXGISwapChain* swapChain ) : pDXGISwapChain( swapChain )
    {
    }
};