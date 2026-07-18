#include "injector.hpp"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>

#include <cerrno>
#include <conio.h>
#include <cstdlib>
#include <cwchar>
#include <iostream>
#include <string_view>
#include <xorstr.hpp>

namespace {

void print_usage() {
    std::wcerr << xorstr_(L"usage: ttapp.exe [--process cs2.exe] [--dll path\\to\\ttapp.dll] [--timeout-ms N] [--logs]\n");
}

bool enable_virtual_terminal() {
    const HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    if (console == nullptr || console == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD mode = 0;
    if (!GetConsoleMode(console, &mode)) {
        return false;
    }

    if ((mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0) {
        return true;
    }

    return SetConsoleMode(console, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;
}

void print_brand() {
    constexpr std::wstring_view brand_art =
LR"(  __    __                           
_/  |__/  |______  ______ ______     
\   __\   __\__  \ \____ \\____ \    
 |  |  |  |  / __ \|  |_> >  |_> >   
 |__|  |__| (____  /   __/|   __/ /\ 
                 \/|__|   |__|    \/ )";

    if (enable_virtual_terminal()) {
        std::wcout << L"\x1b[38;2;0;255;51m" << brand_art << L"\x1b[0m\n\n";
        return;
    }

    const HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO console_info{};
    const bool has_console_attrs =
        console != nullptr &&
        console != INVALID_HANDLE_VALUE &&
        GetConsoleScreenBufferInfo(console, &console_info);

    if (has_console_attrs) {
        SetConsoleTextAttribute(console, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    }

    std::wcout << brand_art;

    if (has_console_attrs) {
        SetConsoleTextAttribute(console, console_info.wAttributes);
    }

    std::wcout << L"\n\n";
}

WORD progress_color(ttapp::injector::LogLevel level) {
    switch (level) {
    case ttapp::injector::LogLevel::info:
        return FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    case ttapp::injector::LogLevel::success:
        return FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    case ttapp::injector::LogLevel::warning:
        return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    case ttapp::injector::LogLevel::error:
        return FOREGROUND_RED | FOREGROUND_INTENSITY;
    }

    return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
}

std::wstring_view progress_prefix(ttapp::injector::LogLevel level) {
    switch (level) {
    case ttapp::injector::LogLevel::info:
        return L"[*]";
    case ttapp::injector::LogLevel::success:
        return L"[+]";
    case ttapp::injector::LogLevel::warning:
        return L"[!]";
    case ttapp::injector::LogLevel::error:
        return L"[x]";
    }

    return L"[*]";
}

void print_progress(ttapp::injector::LogLevel level, std::wstring_view message) {
    const HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO console_info{};
    const bool has_console_attrs =
        console != nullptr &&
        console != INVALID_HANDLE_VALUE &&
        GetConsoleScreenBufferInfo(console, &console_info);

    if (has_console_attrs) {
        SetConsoleTextAttribute(console, progress_color(level));
    }

    std::wcout << progress_prefix(level);

    if (has_console_attrs) {
        SetConsoleTextAttribute(console, console_info.wAttributes);
    }

    std::wcout << L' ' << message << L'\n';
}

void pause_before_exit() {
    std::wcout << xorstr_(L"Press any key to continue . . .");
    std::wcout.flush();
    _getwch();
    std::wcout << L'\n';
}

bool parse_timeout(std::wstring_view value, std::chrono::milliseconds& timeout) {
    if (value.empty()) {
        return false;
    }

    const std::wstring text(value);
    wchar_t* end = nullptr;
    errno = 0;
    const long long parsed = std::wcstoll(text.c_str(), &end, 10);

    if (errno == ERANGE || end != text.c_str() + text.size() || parsed < 0) {
        return false;
    }

    timeout = std::chrono::milliseconds(parsed);
    return true;
}

bool parse_args(int argc, wchar_t** argv, ttapp::injector::InjectorOptions& options) {
    for (int i = 1; i < argc; ++i) {
        const std::wstring_view arg(argv[i]);

        if (arg == xorstr_(L"--help") || arg == xorstr_(L"-h") || arg == xorstr_(L"/?")) {
            print_usage();
            return false;
        }

        if (arg == xorstr_(L"--process")) {
            if (++i >= argc || argv[i][0] == L'\0') {
                std::wcerr << xorstr_(L"missing value for --process\n");
                return false;
            }

            options.process_name = argv[i];
            continue;
        }

        if (arg == xorstr_(L"--dll")) {
            if (++i >= argc || argv[i][0] == L'\0') {
                std::wcerr << xorstr_(L"missing value for --dll\n");
                return false;
            }

            options.dll_path = argv[i];
            continue;
        }

        if (arg == xorstr_(L"--timeout-ms")) {
            if (++i >= argc) {
                std::wcerr << xorstr_(L"missing value for --timeout-ms\n");
                return false;
            }

            if (!parse_timeout(argv[i], options.timeout)) {
                std::wcerr << xorstr_(L"invalid value for --timeout-ms\n");
                return false;
            }

            options.wait_forever = false;
            continue;
        }

        if (arg == xorstr_(L"--logs")) {
            options.enable_logs = true;
            continue;
        }

        std::wcerr << xorstr_(L"unknown argument: ") << arg << L"\n";
        return false;
    }

    return true;
}

} // namespace

int wmain(int argc, wchar_t** argv) {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleTitleW(xorstr_(L"ttapp"));
    print_brand();

    ttapp::injector::InjectorOptions options;
    options.progress_log = print_progress;

    if (!parse_args(argc, argv, options)) {
        pause_before_exit();
        return static_cast<int>(ttapp::injector::ExitCode::invalid_input);
    }

    if (options.process_name.empty()) {
        options.process_name = xorstr_(L"cs2.exe");
    }

    const auto result = ttapp::injector::inject(options);
    const auto code = static_cast<int>(result.code);

    if (result.code == ttapp::injector::ExitCode::success) {
        print_progress(ttapp::injector::LogLevel::success, result.message);
    } else {
        print_progress(ttapp::injector::LogLevel::error, result.message);
    }

    pause_before_exit();
    return code;
}
