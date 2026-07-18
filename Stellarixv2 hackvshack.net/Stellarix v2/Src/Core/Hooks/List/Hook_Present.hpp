#pragma once
#include <windows.h>
#include <dxgi.h>
#include <SafetyHook/safetyhook.hpp>

inline SafetyHookInline Present_o{};
auto WINAPI Hook_Present( IDXGISwapChain* pSwapChain, UINT uSyncInterval, UINT uFlags )->HRESULT;
