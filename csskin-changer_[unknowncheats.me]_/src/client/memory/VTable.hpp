#pragma once

#include <cstddef>
#include <type_traits>

#define VFUNC(_class, ret_type, index, ...) vmt::call_virtual<ret_type>(_class, index, __VA_ARGS__)

namespace vmt {
    template <typename T = void*>
    inline T get_v_method(void* class_, std::size_t index) {
        static_assert(std::is_same_v<T, void*> || std::is_function_v<std::remove_pointer_t<T>>,
            "get_v_method<T> expects void* or a function pointer type");

        if (!class_)
            return T{};

        void** table = *static_cast<void***>(class_);
        if (!table)
            return T{};

        return reinterpret_cast<T>(table[index]);
    }

    template <typename T, typename... Args>
    inline T call_virtual(void* class_, std::size_t index, Args... args) {
        using fn_t = T(__thiscall*)(void*, Args...);
        static_assert(std::is_function_v<std::remove_pointer_t<fn_t>>,
            "call_virtual requires a function pointer vtable entry");

        auto func = get_v_method<fn_t>(class_, index);

        return func(class_, args...);
    }
}
