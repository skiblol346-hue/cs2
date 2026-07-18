#pragma once
#include <windows.h>
#include <dxgi.h>
#include <SafetyHook/safetyhook.hpp>


inline SafetyHookInline CreateSwapChain_o{};
auto WINAPI Hook_CreateSwapChain( IDXGIFactory* pFactory, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain )->HRESULT;
