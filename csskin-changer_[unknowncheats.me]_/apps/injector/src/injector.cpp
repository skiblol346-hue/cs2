#include "injector.hpp"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <BlackBone/Config.h>
#include <BlackBone/Process/Process.h>
#include <BlackBone/Misc/Utils.h>

#include <Windows.h>
#include <TlHelp32.h>

#include <algorithm>
#include <cstdint>
#include <cwchar>
#include <fstream>
#include <limits>
#include <thread>
#include <vector>
#include <random>

#include <xorstr.hpp>

namespace ttapp::injector {
namespace {

const std::filesystem::path k_log_flag_directory = L"C:\\ttapp";
const std::filesystem::path k_log_flag_path = k_log_flag_directory / L"ttapp_logs_enabled";

class unique_handle {
public:
    unique_handle() = default;
    explicit unique_handle(HANDLE handle) : handle_(handle) {}

    unique_handle(const unique_handle&) = delete;
    unique_handle& operator=(const unique_handle&) = delete;

    unique_handle(unique_handle&& other) noexcept : handle_(other.handle_) {
        other.handle_ = nullptr;
    }

    unique_handle& operator=(unique_handle&& other) noexcept {
        if (this != &other) {
            reset();
            handle_ = other.handle_;
            other.handle_ = nullptr;
        }

        return *this;
    }

    ~unique_handle() {
        reset();
    }

    [[nodiscard]] HANDLE get() const {
        return handle_;
    }

    [[nodiscard]] bool valid() const {
        return handle_ != nullptr && handle_ != INVALID_HANDLE_VALUE;
    }

    void reset(HANDLE handle = nullptr) {
        if (valid()) {
            CloseHandle(handle_);
        }

        handle_ = handle;
    }

private:
    HANDLE handle_ = nullptr;
};

void emit(const InjectorOptions& options, LogLevel level, std::wstring_view message) {
    if (options.progress_log) {
        options.progress_log(level, message);
    }
}

std::wstring widen_ascii(const std::string& value) {
    return std::wstring(value.begin(), value.end());
}

std::filesystem::path executable_path() {
    std::vector<wchar_t> buffer(MAX_PATH);

    for (;;) {
        const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (length == 0) {
            return {};
        }

        if (length < buffer.size() - 1) {
            return std::filesystem::path(std::wstring(buffer.data(), length));
        }

        if (buffer.size() >= 32768) {
            return {};
        }

        buffer.resize(buffer.size() * 2);
    }
}

bool enable_debug_privilege() {
    HANDLE raw_token = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &raw_token)) {
        return false;
    }

    unique_handle token(raw_token);

    LUID luid{};
    if (!LookupPrivilegeValueW(nullptr, xorstr_(L"SeDebugPrivilege"), &luid)) {
        return false;
    }

    TOKEN_PRIVILEGES privileges{};
    privileges.PrivilegeCount = 1;
    privileges.Privileges[0].Luid = luid;
    privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(token.get(), FALSE, &privileges, static_cast<DWORD>(sizeof(privileges)), nullptr, nullptr)) {
        return false;
    }

    return GetLastError() != ERROR_NOT_ALL_ASSIGNED;
}

std::vector<std::uint8_t> read_file(const std::filesystem::path& path, std::wstring& error) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        error = L"Failed to open DLL: " + path.wstring();
        return {};
    }

    const std::streamoff file_size = file.tellg();
    if (file_size <= 0) {
        error = L"DLL is empty: " + path.wstring();
        return {};
    }

    if (static_cast<unsigned long long>(file_size) > (std::numeric_limits<std::size_t>::max)()) {
        error = L"DLL is too large to read: " + path.wstring();
        return {};
    }

    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(file_size));
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(bytes.data()), file_size);

    if (!file) {
        error = L"Failed to read DLL: " + path.wstring();
        return {};
    }

    return bytes;
}

bool validate_pe_image(const std::vector<std::uint8_t>& bytes, std::wstring& error) {
    if (bytes.size() < sizeof(IMAGE_DOS_HEADER)) {
        error = L"DLL is too small to contain a DOS header";
        return false;
    }

    const auto* dos_header = reinterpret_cast<const IMAGE_DOS_HEADER*>(bytes.data());
    if (dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
        error = L"DLL does not start with an MZ header";
        return false;
    }

    if (dos_header->e_lfanew < 0) {
        error = L"DLL has an invalid PE header offset";
        return false;
    }

    const auto nt_offset = static_cast<std::size_t>(dos_header->e_lfanew);
    if (nt_offset > bytes.size() || bytes.size() - nt_offset < sizeof(IMAGE_NT_HEADERS64)) {
        error = L"DLL PE header is outside the file";
        return false;
    }

    const auto* nt_headers = reinterpret_cast<const IMAGE_NT_HEADERS64*>(bytes.data() + nt_offset);
    if (nt_headers->Signature != IMAGE_NT_SIGNATURE) {
        error = L"DLL does not contain a valid PE signature";
        return false;
    }

    if (nt_headers->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64) {
        error = L"DLL is not an x64 image";
        return false;
    }

    if (nt_headers->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        error = L"DLL does not contain a PE32+ optional header";
        return false;
    }

    return true;
}

DWORD find_process_id_once(const std::wstring& process_name) {
    unique_handle snapshot(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
    if (!snapshot.valid()) {
        return 0;
    }

    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);

    if (!Process32FirstW(snapshot.get(), &entry)) {
        return 0;
    }

    do {
        if (_wcsicmp(entry.szExeFile, process_name.c_str()) == 0) {
            return entry.th32ProcessID;
        }
    } while (Process32NextW(snapshot.get(), &entry));

    return 0;
}

DWORD wait_for_process(const InjectorOptions& options) {
    const auto start = std::chrono::steady_clock::now();

    for (;;) {
        const DWORD process_id = find_process_id_once(options.process_name);
        if (process_id != 0) {
            return process_id;
        }

        if (!options.wait_forever) {
            const auto elapsed = std::chrono::steady_clock::now() - start;
            if (elapsed >= options.timeout) {
                return 0;
            }

            const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(options.timeout - elapsed);
            std::this_thread::sleep_for((std::min)(options.poll_interval, remaining));
            continue;
        }

        std::this_thread::sleep_for(options.poll_interval);
    }
}

std::wstring describe_status(NTSTATUS status) {
    auto description = blackbone::Utils::GetErrorDescription(status);
    if (description.empty()) {
        description = L"NTSTATUS 0x" + std::to_wstring(static_cast<unsigned long>(status));
    }

    return description;
}

void write_log_flag(const std::filesystem::path& log_directory) {
    std::error_code fs_error;
    std::filesystem::create_directories(k_log_flag_directory, fs_error);
    if (fs_error) {
        return;
    }

    std::ofstream flag_file(k_log_flag_path);
    if (flag_file) {
        flag_file << log_directory.string();
    }
}

void remove_old_logs(const std::filesystem::path& log_directory) {
    const wchar_t* log_files[] = {
        L"ttapp_log.txt",
        L"quick_stop_log.txt",
        L"spectator_list_log.txt",
        L"auto_accept_log.txt",
    };

    std::error_code fs_error;
    for (const wchar_t* log_file : log_files) {
        std::filesystem::remove(log_directory / log_file, fs_error);
        fs_error.clear();
    }
}

} // namespace

std::filesystem::path default_dll_path() {
    const auto exe_path = executable_path();
    if (exe_path.empty()) {
        return {};
    }

    return exe_path.parent_path() / xorstr_(L"ttapp.dll");
}

InjectionResult inject(const InjectorOptions& options) {
    auto dll_path = options.dll_path.empty() ? default_dll_path() : options.dll_path;
    if (dll_path.empty()) {
        return { ExitCode::invalid_input, xorstr_(L"Failed to resolve injector executable path") };
    }

    std::error_code fs_error;
    dll_path = std::filesystem::absolute(dll_path, fs_error);
    if (fs_error) {
        return { ExitCode::invalid_input, xorstr_(L"Failed to resolve DLL path: ") + widen_ascii(fs_error.message()) };
    }

    emit(options, LogLevel::info, xorstr_(L"Resolved DLL path: ") + dll_path.wstring());

    if (!std::filesystem::exists(dll_path, fs_error) || fs_error) {
        return { ExitCode::invalid_input, xorstr_(L"DLL not found: ") + dll_path.wstring() };
    }

    if (!std::filesystem::is_regular_file(dll_path, fs_error) || fs_error) {
        return { ExitCode::invalid_input, xorstr_(L"DLL path is not a file: ") + dll_path.wstring() };
    }

    emit(options, LogLevel::success, xorstr_(L"DLL file found"));

    std::wstring error;
    auto bytes = read_file(dll_path, error);
    if (bytes.empty()) {
        return { ExitCode::invalid_input, error };
    }

    emit(options, LogLevel::success, xorstr_(L"DLL read completed: ") + std::to_wstring(static_cast<unsigned long long>(bytes.size())) + xorstr_(L" bytes"));

    if (!validate_pe_image(bytes, error)) {
        return { ExitCode::invalid_input, error };
    }

    emit(options, LogLevel::success, xorstr_(L"PE image validated"));

    if (options.enable_logs) {
        const auto exe_path = executable_path();
        if (!exe_path.empty()) {
            const auto log_directory = exe_path.parent_path();
            remove_old_logs(log_directory);
            write_log_flag(log_directory);
            emit(options, LogLevel::info, xorstr_(L"Client file logging enabled"));
        } else {
            emit(options, LogLevel::warning, xorstr_(L"Client file logging requested, but executable path could not be resolved"));
        }
    } else {
        DeleteFileW(k_log_flag_path.c_str());
        emit(options, LogLevel::info, xorstr_(L"Client file logging disabled"));
    }

    const bool debug_privilege_enabled = enable_debug_privilege();
    if (debug_privilege_enabled) {
        emit(options, LogLevel::success, xorstr_(L"SeDebugPrivilege enabled"));
    } else {
        emit(options, LogLevel::warning, xorstr_(L"SeDebugPrivilege was not enabled"));
    }

    emit(options, LogLevel::info, xorstr_(L"Searching for target process: ") + options.process_name);
    const DWORD process_id = wait_for_process(options);
    if (process_id == 0) {
        return { ExitCode::process_not_found, xorstr_(L"Target process not found: ") + options.process_name };
    }

    emit(options, LogLevel::success, xorstr_(L"Target process found: ") + options.process_name + xorstr_(L" (PID ") + std::to_wstring(static_cast<unsigned long>(process_id)) + xorstr_(L")"));

    // Randomized delay before attachment (2000ms to 5000ms) to evade timing-based heuristics
    {
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> dist(2000, 5000);
        std::this_thread::sleep_for(std::chrono::milliseconds(dist(rng)));
    }

    blackbone::Process process;
    emit(options, LogLevel::info, xorstr_(L"Attaching to process"));
    const NTSTATUS attach_status = process.Attach(process_id);
    if (!NT_SUCCESS(attach_status)) {
        return { ExitCode::attach_failed, xorstr_(L"BlackBone attach failed: ") + describe_status(attach_status) };
    }

    emit(options, LogLevel::success, xorstr_(L"Process attach succeeded"));
    emit(options, LogLevel::info, xorstr_(L"Mapping DLL image"));

    auto image = process.mmap().MapImage(bytes.size(), bytes.data(), false, blackbone::NoFlags);
    if (!image) {
        auto message = xorstr_(L"BlackBone map failed: ") + describe_status(image.status);
        if (!debug_privilege_enabled) {
            message += xorstr_(L" (SeDebugPrivilege was not enabled)");
        }

        return { ExitCode::map_failed, message };
    }

    emit(options, LogLevel::success, xorstr_(L"DLL image mapped"));

    std::wstring message = xorstr_(L"Injected ") + dll_path.wstring() + xorstr_(L" into ") + options.process_name;
    if (!debug_privilege_enabled) {
        message += xorstr_(L" (warning: SeDebugPrivilege was not enabled)");
    }

    return { ExitCode::success, message };
}

} // namespace ttapp::injector
