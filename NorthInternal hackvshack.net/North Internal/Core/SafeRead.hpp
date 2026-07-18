#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstdint>
#include <cstring>

namespace SafeRead {

    static constexpr uintptr_t ADDR_MIN = 0x10000;
    static constexpr uintptr_t ADDR_MAX = 0x7FFFFFFEFFFF;

    inline bool IsValidPtr(uintptr_t addr) {
        return addr >= ADDR_MIN && addr <= ADDR_MAX;
    }

    template <typename T>
    inline bool Read(uintptr_t addr, T& out) {
        __try {
            if (!IsValidPtr(addr)) return false;
            memcpy(&out, reinterpret_cast<const void*>(addr), sizeof(T));
            return true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }
    }

    inline bool ReadPtr(uintptr_t addr, uintptr_t& out) {
        return Read<uintptr_t>(addr, out);
    }

    inline bool ReadInt(uintptr_t addr, int& out) {
        return Read<int>(addr, out);
    }

    inline bool ReadFloat(uintptr_t addr, float& out) {
        return Read<float>(addr, out);
    }

    inline bool ReadBool(uintptr_t addr, bool& out) {
        return Read<bool>(addr, out);
    }

    inline bool ReadByte(uintptr_t addr, uint8_t& out) {
        return Read<uint8_t>(addr, out);
    }

    inline bool ReadString(uintptr_t ptrAddr, char* out, int maxLen) {
        __try {
            if (!IsValidPtr(ptrAddr)) return false;
            uintptr_t strPtr = *reinterpret_cast<const uintptr_t*>(ptrAddr);
            if (!IsValidPtr(strPtr)) return false;
            for (int i = 0; i < maxLen - 1; i++) {
                out[i] = *reinterpret_cast<const char*>(strPtr + i);
                if (out[i] == '\0') return i > 0;
            }
            out[maxLen - 1] = '\0';
            return true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }
    }

}
