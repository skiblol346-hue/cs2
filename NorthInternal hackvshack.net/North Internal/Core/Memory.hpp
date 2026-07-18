#pragma once
#include <windows.h>
#include <vector>
#include <string>
#include <iostream>
#include <Psapi.h>

namespace Memory {
    inline uintptr_t FindPattern(const char* moduleName, const char* pattern) {
        auto moduleHandle = GetModuleHandleA(moduleName);
        if (!moduleHandle) return 0;

        MODULEINFO moduleInfo;
        GetModuleInformation(GetCurrentProcess(), moduleHandle, &moduleInfo, sizeof(MODULEINFO));

        uintptr_t start = (uintptr_t)moduleHandle;
        uintptr_t end = start + moduleInfo.SizeOfImage;

        auto patternToBytes = [](const char* pattern) {
            std::vector<int> bytes;
            char* start = const_cast<char*>(pattern);
            char* end = const_cast<char*>(pattern) + strlen(pattern);

            for (char* current = start; current < end; ++current) {
                if (*current == '?') {
                    ++current;
                    if (*current == '?') ++current;
                    bytes.push_back(-1);
                }
                else {
                    bytes.push_back(strtoul(current, &current, 16));
                }
            }
            return bytes;
            };

        auto patternBytes = patternToBytes(pattern);
        size_t s = patternBytes.size();
        int* d = patternBytes.data();

        for (uintptr_t i = start; i < end - s; ++i) {
            bool found = true;
            for (size_t j = 0; j < s; ++j) {
                if (d[j] != -1 && *(uint8_t*)(i + j) != d[j]) {
                    found = false;
                    break;
                }
            }
            if (found) return i;
        }
        return 0;
    }

    inline uintptr_t ResolveJump(uintptr_t address) {
        if (!address) return 0;
        return address + *(int32_t*)(address + 3) + 7;
    }
}