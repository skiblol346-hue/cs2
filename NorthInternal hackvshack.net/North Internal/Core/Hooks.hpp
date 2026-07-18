#pragma once
#include <d3d11.h>
#include <dxgi.h>
#include "../../Dependencies/ImGui/imgui.h"
#include "../../Dependencies/ImGui/imgui_impl_win32.h"
#include "../../Dependencies/ImGui/imgui_impl_dx11.h"
#include "../../Dependencies/minhook/MinHook.h"

namespace Hooks {
    inline HWND m_hwnd = nullptr;
    inline bool m_bInitialized = false;
    inline ID3D11RenderTargetView* m_mainRenderTargetView = nullptr;
    typedef HRESULT(__stdcall* Present_t)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
    inline Present_t oPresent = nullptr;
    inline WNDPROC oWndProc = nullptr;

    inline void* oFrameStageNotify = nullptr;
    void __fastcall FrameStageNotify(void* rcx, int stage);

    HRESULT __stdcall Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
    LRESULT __stdcall WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    void Initialize();
    void Release();
}