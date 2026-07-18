#include "platform/ClientCore.hpp"
#include "runtime/Runtime.hpp"
#include "game/interfaces/i_mem_alloc.hpp"

#include <exception>
#include <memory>

namespace {

DWORD WINAPI client_thread(void* parameter) {
    const auto module = static_cast<HMODULE>(parameter);
    std::unique_ptr<ttapp::client::Runtime> runtime;

    try {
        runtime = std::make_unique<ttapp::client::Runtime>(module);
        if (runtime->initialize()) {
            runtime->run();
        }

        runtime->shutdown();
    } catch (const std::exception& error) {
        if (runtime && runtime->is_logger_initialized())
            LOG_ERROR(xorstr_("[runtime] unhandled exception: %s"), error.what());
        else
            OutputDebugStringA(error.what());

        if (runtime)
            runtime->shutdown();
    } catch (...) {
        if (runtime && runtime->is_logger_initialized())
            LOG_ERROR(xorstr_("[runtime] unknown exception"));
        else
            OutputDebugStringA(xorstr_("Unknown exception occurred"));

        if (runtime)
            runtime->shutdown();
    }

    FreeLibraryAndExitThread(module, 0);
}

} // namespace

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    if (reason != DLL_PROCESS_ATTACH) {
        return TRUE;
    }

    InitMemAlloc();
    DisableThreadLibraryCalls(module);

    HANDLE thread = CreateThread(nullptr, 0, client_thread, module, 0, nullptr);
    if (thread == nullptr || thread == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    CloseHandle(thread);
    return TRUE;
}
