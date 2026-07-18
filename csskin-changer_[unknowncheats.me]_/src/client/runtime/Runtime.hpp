#pragma once

#include <Windows.h>

#include "runtime/Services.hpp"

namespace ttapp::client {

class Runtime {
public:
    explicit Runtime(HMODULE module);

    bool initialize();
    void run();
    void shutdown();

    HMODULE module() const { return module_; }
    bool is_logger_initialized() const { return logger_initialized_; }

    Services services;

private:
    bool fail_initialize(const char* stage);

    HMODULE module_ = nullptr;
    bool initialized_ = false;
    bool logger_initialized_ = false;
    bool renderer_initialized_ = false;
    bool hooks_initialized_ = false;
};

Runtime& runtime();
void set_active_runtime(Runtime* runtime);

} // namespace ttapp::client
