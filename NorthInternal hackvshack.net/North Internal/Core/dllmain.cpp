#include <windows.h>
#include <thread>
#include <cstdio>
#include <psapi.h>
#include "../Core/Hooks.hpp"
#include "../Features/Menu/Menu.hpp"
#include "../Features/SkinChanger/SkinChanger.hpp"
#include "../SDK/EntityList.hpp"
#include "../SDK/Offsets.hpp"
#include "../Core/PatternScanner.hpp"

static constexpr DWORD OFFSET_REFRESH_INTERVAL_MS = 5 * 60 * 1000;

void MainThread(HMODULE hModule) {
    while (!GetModuleHandleA("client.dll"))
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::this_thread::sleep_for(std::chrono::milliseconds(3000));

    Offsets::Initialize();
    Hooks::Initialize();

    Functions::fnSetModel = PatternScanner::FindPattern(
        "client.dll",
        "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B F9 48 8B F2"
    );

    std::thread entityUpdateThread([]() {
        while (!(GetAsyncKeyState(VK_END) & 0x8000)) {
            g_EntityList_Logic.Update();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        });

    std::thread offsetRefreshThread([]() {
        while (!(GetAsyncKeyState(VK_END) & 0x8000)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(OFFSET_REFRESH_INTERVAL_MS));
            if (!(GetAsyncKeyState(VK_END) & 0x8000))
                Offsets::Refresh();
        }
        });

    while (!(GetAsyncKeyState(VK_END) & 0x8000)) {
        if (GetAsyncKeyState(VK_INSERT) & 1)
            g_Menu.Toggle();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (entityUpdateThread.joinable())  entityUpdateThread.join();
    if (offsetRefreshThread.joinable()) offsetRefreshThread.join();

    Hooks::Release();
    FreeLibraryAndExitThread(hModule, 0);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        HANDLE hThread = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(MainThread), hModule, 0, nullptr);
        if (hThread) CloseHandle(hThread);
    }
    return TRUE;
}
