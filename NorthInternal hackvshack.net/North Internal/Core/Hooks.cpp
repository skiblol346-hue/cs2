#include "Hooks.hpp"
#include "../Features/Menu/Menu.hpp"
#include "../Features/ESP/General.hpp"
#include "../Features/ESP/Flags.hpp"
#include "../Features/SkinChanger/SkinChanger.hpp"
#include "../Features/LegitBot/LegitBot.hpp"
#include "../Features/View/View.hpp"
#include "../SDK/EntityList.hpp"
#include "../SDK/Offsets.hpp"
#include "../Core/PatternScanner.hpp"

#pragma comment(lib, "d3d11.lib")

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT __stdcall Hooks::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (g_Menu.bOpened && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;
    return CallWindowProc(oWndProc, hWnd, msg, wParam, lParam);
}

void __fastcall Hooks::FrameStageNotify(void* rcx, int stage) {
    OutputDebugStringA("[Hook] FSN called\n");
    if (stage == 7) {
        g_SkinChanger.ApplySkins();
    }
    using fn_t = void(__fastcall*)(void*, int);
    auto orig = reinterpret_cast<fn_t>(oFrameStageNotify);
    if (orig) orig(rcx, stage);
}

HRESULT __stdcall Hooks::Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
    __try {
        ID3D11Device* pDevice = nullptr;
        ID3D11DeviceContext* pContext = nullptr;

        if (FAILED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice)))
            return oPresent(pSwapChain, SyncInterval, Flags);

        pDevice->GetImmediateContext(&pContext);

        if (!m_bInitialized) {
            DXGI_SWAP_CHAIN_DESC sd;
            pSwapChain->GetDesc(&sd);
            m_hwnd = sd.OutputWindow;

            ID3D11Texture2D* pBackBuffer = nullptr;
            pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
            if (pBackBuffer) {
                pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_mainRenderTargetView);
                pBackBuffer->Release();
            }

            ImGui::CreateContext();
            ImGui_ImplWin32_Init(m_hwnd);
            ImGui_ImplDX11_Init(pDevice, pContext);

            g_Menu.LoadFonts();

            oWndProc = (WNDPROC)SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
            m_bInitialized = true;
        }

        uintptr_t clientBase = (uintptr_t)GetModuleHandleA("client.dll");
        uintptr_t localPawn = 0;
        if (clientBase && Offsets::dwLocalPlayerPawn) {
            __try {
                localPawn = *(uintptr_t*)(clientBase + Offsets::dwLocalPlayerPawn);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {}
        }

        LegitBot_Aim::Run(clientBase, localPawn);
        LegitBot_AutoFire::Run(clientBase, localPawn);

        g_View.Run(clientBase, localPawn);

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::GetIO().MouseDrawCursor = g_Menu.bOpened;

        g_ESP.RenderGeneral();
        g_Flags.RenderFlags();
        LegitBot_Aim::Draw(localPawn);
        g_Menu.Render();

        ImGui::Render();

        pContext->OMSetRenderTargets(1, &m_mainRenderTargetView, nullptr);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        pContext->Release();
        pDevice->Release();

        return oPresent(pSwapChain, SyncInterval, Flags);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return oPresent(pSwapChain, SyncInterval, Flags);
    }
}

void Hooks::Initialize() {
    if (MH_Initialize() != MH_OK) return;

    uintptr_t frameStageNotify = PatternScanner::FindPattern("client.dll",
        "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B D9 E8 ? ? ? ? 48 8B E8");
    if (frameStageNotify) {
        MH_CreateHook((void*)frameStageNotify, &FrameStageNotify, &oFrameStageNotify);
        MH_EnableHook((void*)frameStageNotify);
    }

    ID3D11Device* pDevice = nullptr;
    ID3D11DeviceContext* pContext = nullptr;
    IDXGISwapChain* pSwapChain = nullptr;

    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 1;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = GetForegroundWindow();
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    if (SUCCEEDED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, &featureLevel, 1, D3D11_SDK_VERSION, &scd, &pSwapChain, &pDevice, nullptr, &pContext))) {
        void** vmt = *(void***)pSwapChain;

        MH_CreateHook(vmt[8], &Present, reinterpret_cast<void**>(&oPresent));
        MH_EnableHook(vmt[8]);

        pSwapChain->Release();
        pDevice->Release();
        pContext->Release();
    }
}

void Hooks::Release() {
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();

    if (oWndProc) {
        SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, (LONG_PTR)oWndProc);
    }

    if (m_mainRenderTargetView) {
        m_mainRenderTargetView->Release();
        m_mainRenderTargetView = nullptr;
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}