#pragma once
#include <Windows.h>
#include <Psapi.h>
#include <string>
#include <vector>
#include <cstdint>

class PatternScanner {
private:
    struct Pattern {
        std::vector<int> bytes;
        std::string mask;
    };

    static Pattern ParsePattern(const char* pattern);
    static bool CompareBytes(const uint8_t* data, const Pattern& pattern);
    static uintptr_t ScanRegion(uintptr_t start, size_t size, const Pattern& pattern);

public:
    static uintptr_t FindPattern(const char* moduleName, const char* pattern);
    static uintptr_t FindPattern(uintptr_t start, size_t size, const char* pattern);
    static uintptr_t ResolveRelativeAddress(uintptr_t address, int offset, int instructionSize);
};