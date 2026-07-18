#include "PatternScanner.hpp"

PatternScanner::Pattern PatternScanner::ParsePattern(const char* pattern) {
    Pattern result;
    const char* current = pattern;

    while (*current) {
        if (*current == ' ') {
            current++;
            continue;
        }

        if (*current == '?') {
            result.bytes.push_back(-1);
            result.mask += '?';
            current++;
            if (*current == '?') current++;
        }
        else {
            char byte[3] = { current[0], current[1], 0 };
            result.bytes.push_back(strtol(byte, nullptr, 16));
            result.mask += 'x';
            current += 2;
        }
    }

    return result;
}

bool PatternScanner::CompareBytes(const uint8_t* data, const Pattern& pattern) {
    __try {
        for (size_t i = 0; i < pattern.bytes.size(); i++) {
            if (pattern.mask[i] == 'x' && data[i] != pattern.bytes[i]) {
                return false;
            }
        }
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

uintptr_t PatternScanner::ScanRegion(uintptr_t start, size_t size, const Pattern& pattern) {
    __try {
        const uint8_t* data = reinterpret_cast<const uint8_t*>(start);
        const size_t patternSize = pattern.bytes.size();

        if (size <= patternSize) return 0;

        for (size_t i = 0; i < size - patternSize; i++) {
            if (CompareBytes(data + i, pattern)) {
                return start + i;
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return 0;
    }

    return 0;
}

uintptr_t PatternScanner::FindPattern(const char* moduleName, const char* patternStr) {
    HMODULE module = GetModuleHandleA(moduleName);
    if (!module) return 0;

    MODULEINFO moduleInfo;
    if (!GetModuleInformation(GetCurrentProcess(), module, &moduleInfo, sizeof(MODULEINFO))) return 0;

    Pattern pattern = ParsePattern(patternStr);
    uintptr_t moduleBase = reinterpret_cast<uintptr_t>(module);
    uintptr_t moduleEnd = moduleBase + moduleInfo.SizeOfImage;

    MEMORY_BASIC_INFORMATION mbi;
    uintptr_t address = moduleBase;

    while (address < moduleEnd) {
        if (VirtualQuery(reinterpret_cast<LPCVOID>(address), &mbi, sizeof(mbi)) == 0) break;

        if (mbi.State == MEM_COMMIT &&
            mbi.Type == MEM_IMAGE &&
            (mbi.Protect == PAGE_EXECUTE_READ ||
                mbi.Protect == PAGE_EXECUTE_READWRITE ||
                mbi.Protect == PAGE_READONLY ||
                mbi.Protect == PAGE_READWRITE)) {

            uintptr_t result = ScanRegion(address, mbi.RegionSize, pattern);
            if (result) return result;
        }

        address += mbi.RegionSize;
    }

    return 0;
}

uintptr_t PatternScanner::FindPattern(uintptr_t start, size_t size, const char* patternStr) {
    Pattern pattern = ParsePattern(patternStr);
    return ScanRegion(start, size, pattern);
}

uintptr_t PatternScanner::ResolveRelativeAddress(uintptr_t address, int offset, int instructionSize) {
    if (!address) return 0;
    __try {
        int32_t relativeOffset = *reinterpret_cast<int32_t*>(address + offset);
        return address + relativeOffset + instructionSize;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return 0;
    }
}