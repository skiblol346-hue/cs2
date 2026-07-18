#pragma once
#include <windows.h>
#include <dxgi.h>
#include <cstdint>
#include <SafetyHook/safetyhook.hpp>


inline SafetyHookInline ResizeBuffers_o{};
auto __fastcall Hook_ResizeBuffers( IDXGISwapChain* pSwapChain, std::uint32_t nBufferCount, std::uint32_t nWidth, std::uint32_t nHeight, DXGI_FORMAT newFormat, std::uint32_t nFlags )->HRESULT;
