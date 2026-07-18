#include "runtime/Runtime.hpp"
#include "runtime/ServiceAccess.hpp"

#include <exception>

#include "game/interfaces/i_mem_alloc.hpp"

namespace ttapp::client {
namespace {

Runtime* active_runtime = nullptr;
constexpr const char* k_log_flag_path = "C:\\ttapp\\ttapp_logs_enabled";

} // namespace

#include <Windows.h>
#include <cstdio>
#include <cstring>

namespace {

bool get_directory_from_path(const char* path, char* out, size_t out_size)
{
    if (!path || !path[0])
        return false;

    strcpy_s(out, out_size, path);
    char* last_slash = strrchr(out, '\\');
    if (!last_slash)
        return false;

    *last_slash = '\0';
    return out[0] != '\0';
}

bool directory_exists(const char* path)
{
    const DWORD attrib = GetFileAttributesA(path);
    return attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY);
}

bool use_log_flag(const char* flag_path)
{
    const DWORD attrib = GetFileAttributesA(flag_path);
    if (attrib == INVALID_FILE_ATTRIBUTES || (attrib & FILE_ATTRIBUTE_DIRECTORY))
        return false;

    char log_directory[MAX_PATH] = {};
    FILE* flag_file = nullptr;
    if (fopen_s(&flag_file, flag_path, "r") == 0 && flag_file) {
        if (fgets(log_directory, sizeof(log_directory), flag_file))
            log_directory[strcspn(log_directory, "\r\n")] = '\0';
        fclose(flag_file);
    }

    if (!directory_exists(log_directory))
        get_directory_from_path(flag_path, log_directory, sizeof(log_directory));

    if (directory_exists(log_directory))
        logger::set_output_directory(log_directory);

    DeleteFileA(flag_path);
    return true;
}

} // namespace

Runtime::Runtime(HMODULE module) : module_(module) {}

bool Runtime::initialize() {
    set_active_runtime(this);
    logger::set_base_directory(module_);

    const bool save_to_file = use_log_flag(k_log_flag_path);

    logger::initialize(save_to_file);
    logger_initialized_ = true;
    LOG_INFO(xorstr_("Logger initialized."));

    services.config_store.setup_values();
    LOG_INFO(xorstr_("Config values setup."));

    if (!services.modules.m_modules.initialize())
        return fail_initialize(xorstr_("modules"));
    LOG_INFO(xorstr_("Modules initialized."));

    if (!services.interfaces.initialize())
        return fail_initialize(xorstr_("interfaces"));
    LOG_INFO(xorstr_("Interfaces initialized."));

    LOG_INFO(xorstr_("Searching..."));
    if (!services.renderer.initialize())
        return fail_initialize(xorstr_("renderer"));
    renderer_initialized_ = true;
    LOG_INFO(xorstr_("Renderer initialized."));

    if (!services.hooks.initialize())
        return fail_initialize(xorstr_("hooks"));
    hooks_initialized_ = true;
    LOG_INFO(xorstr_("Hooks initialized."));

    services.config_store.auto_load_key();
    LOG_INFO(xorstr_("Config key auto-loaded."));

    LOG_INFO(xorstr_("Initializing ItemSchema..."));
    services.item_schema.initialize();
    LOG_INFO(xorstr_("ItemSchema initialized."));

    LOG_INFO(xorstr_("Initializing SkinChanger..."));
    services.skin_changer.initialize();
    LOG_INFO(xorstr_("SkinChanger initialized."));

    services.config_store.auto_load();
    LOG_INFO(xorstr_("Config auto-loaded."));

    initialized_ = true;
    LOG_SUCCESS(xorstr_(u8"ttapp is ready"));
    return true;
}

bool Runtime::fail_initialize(const char* stage) {
    LOG_ERROR(xorstr_("[runtime] initialization failed at stage: %s"), stage);
    services.patterns.log_failures();
    LOG_ERROR(xorstr_("[runtime] unloading cleanly"));
    shutdown();
    return false;
}

void Runtime::run() {
    while (!GetAsyncKeyState(VK_END)) {
        Sleep(100);
    }
}

void Runtime::shutdown() {
    if (hooks_initialized_) {
        services.hooks.destroy();
        hooks_initialized_ = false;
        renderer_initialized_ = false;
    }
    else if (renderer_initialized_) {
        services.renderer.uninitialize();
        renderer_initialized_ = false;
    }

    if (logger_initialized_) {
        logger::shutdown();
        logger_initialized_ = false;
    }

    set_active_runtime(nullptr);
    initialized_ = false;
}

Runtime& runtime() {
    return *active_runtime;
}

Config& config() { return runtime().services.config; }
ConfigStore& config_store() { return runtime().services.config_store; }
ModuleRegistry& modules() { return runtime().services.modules; }
InterfaceRegistry& interfaces() { return runtime().services.interfaces; }
PatternScanner& patterns() { return runtime().services.patterns; }
SchemaRegistry& schema() { return runtime().services.schema; }
ItemSchema& item_schema() { return runtime().services.item_schema; }
QuickStop& quick_stop() { return runtime().services.quick_stop; }
AutoAccept& auto_accept() { return runtime().services.auto_accept; }
SpectatorList& spectator_list() { return runtime().services.spectator_list; }
SkinChanger& skin_changer() { return runtime().services.skin_changer; }
GloveChanger& glove_changer() { return runtime().services.glove_changer; }
AgentChanger& agent_changer() { return runtime().services.agent_changer; }
Dx11Renderer& renderer() { return runtime().services.renderer; }
Menu& menu() { return runtime().services.menu; }
HookManager& hooks() { return runtime().services.hooks; }
GameFrameContext& frame() { return runtime().services.frame; }

void set_active_runtime(Runtime* runtime) {
    active_runtime = runtime;
}

} // namespace ttapp::client
