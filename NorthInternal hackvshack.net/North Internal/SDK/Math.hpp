#include <windows.h>
#include <thread>
#include "../Core/Hooks.hpp"

void Setup(HMODULE hModule) {
    Hooks::Initialize();

    while (!(GetAsyncKeyState(VK_END) & 0x8000)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    Hooks::Release();
    FreeLibraryAndExitThread(hModule, 0);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        const auto hThread = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(Setup), hModule, 0, nullptr);
        if (hThread) CloseHandle(hThread);
    }
    return TRUE;
}