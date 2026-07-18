#pragma once

#include <chrono>
#include <filesystem>
#include <string>
#include <string_view>

namespace ttapp::injector {

enum class LogLevel {
    info,
    success,
    warning,
    error,
};

using ProgressLogCallback = void (*)(LogLevel level, std::wstring_view message);

struct InjectorOptions {
    std::wstring process_name;
    std::filesystem::path dll_path;
    bool wait_forever = true;
    std::chrono::milliseconds timeout{ 0 };
    std::chrono::milliseconds poll_interval{ 100 };
    bool enable_logs = false;
    ProgressLogCallback progress_log = nullptr;
};

enum class ExitCode : int {
    success = 0,
    invalid_input = 1,
    process_not_found = 2,
    attach_failed = 3,
    map_failed = 4,
};

struct InjectionResult {
    ExitCode code = ExitCode::invalid_input;
    std::wstring message;
};

std::filesystem::path default_dll_path();
InjectionResult inject(const InjectorOptions& options);

} // namespace ttapp::injector
