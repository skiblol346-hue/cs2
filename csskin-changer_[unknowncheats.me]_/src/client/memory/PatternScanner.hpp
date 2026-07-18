#pragma once

#include <cstdint>
#include <initializer_list>
#include <memory>
#include <string>
#include <Windows.h>
#include <vector>

inline bool valid_ptr(const void* p) {
	return reinterpret_cast<std::uintptr_t>(p) >= 0x10000;
}
inline bool valid_ptr(std::uintptr_t p) {
	return p >= 0x10000;
}

struct PatternCandidate {
    const char* bytes;
};

struct PatternFailure {
    std::string module_name;
    std::string signature_name;
    std::vector<std::string> candidates;
};

class PatternScanner {
    std::vector<PatternFailure> m_failures;

public:
    std::vector<int> ida_to_bytes(const char*);
    unsigned char* scan(const char*, const char*);
    unsigned char* scan(const char*, const char*, int);
    unsigned char* scan(const char*, const char*, std::initializer_list<PatternCandidate>);
    unsigned char* scan(const char*, const char*, std::initializer_list<PatternCandidate>, int);
    unsigned char* scan_absolute(const char*, const char*);
    unsigned char* scan_absolute(const char*, const char*, int);
    unsigned char* scan_absolute(const char*, const char*, int, int);
    unsigned char* scan_absolute(const char*, const char*, std::initializer_list<PatternCandidate>);
    unsigned char* scan_absolute(const char*, const char*, std::initializer_list<PatternCandidate>, int);
    unsigned char* scan_absolute(const char*, const char*, std::initializer_list<PatternCandidate>, int, int);
    const std::vector<PatternFailure>& failures() const;
    void clear_failures();
    void log_failures() const;
    unsigned __int64 export_fn(unsigned __int64 base, const char* procedure_name);
    unsigned char* get_absolute_address(unsigned char*);
    unsigned char* get_absolute_address(unsigned char*, int);
    unsigned char* get_absolute_address(unsigned char*, int, int);
    unsigned char* resolve_relative_address(unsigned char*, int, int);
    unsigned char* resolve_relative_address(unsigned char*, int);
};
