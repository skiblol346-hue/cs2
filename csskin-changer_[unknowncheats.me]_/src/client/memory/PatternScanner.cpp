#include "platform/ClientCore.hpp"
#include "memory/PatternScanner.hpp"

namespace {

uint8_t* scan_one(PatternScanner& scanner, const char* module_name, const char* pattern)
{
    if (!module_name || !pattern)
        return nullptr;

    void* module_handle = WINCALL(GetModuleHandle)(module_name);
    if (module_handle == nullptr)
        return nullptr;

    PIMAGE_DOS_HEADER dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(module_handle);
    PIMAGE_NT_HEADERS nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<uint8_t*>(module_handle) + dos_header->e_lfanew);

    auto size_of_image = nt_headers->OptionalHeader.SizeOfImage;
    auto pattern_bytes = scanner.ida_to_bytes(pattern);
    auto scan_bytes = reinterpret_cast<uint8_t*>(module_handle);

    auto pattern_size = pattern_bytes.size();
    auto pattern_data = pattern_bytes.data();

    if (pattern_size == 0 || pattern_size > size_of_image)
        return nullptr;

    for (unsigned int i = 0; i <= size_of_image - pattern_size; i++) {
        bool found = true;

        for (unsigned int j = 0; j < pattern_size; ++j) {
            if (pattern_data[j] == -1)
                continue;

            if (scan_bytes[i + j] != pattern_data[j]) {
                found = false;
                break;
            }
        }

        if (found)
            return &scan_bytes[i];
    }

    return nullptr;
}

} // namespace

std::vector<int> PatternScanner::ida_to_bytes(const char* pattern)
{
    std::vector<int> bytes = std::vector<int>{};
    char* start = const_cast<char*>(pattern);
    char* end = const_cast<char*>(pattern) + strlen(pattern);

    for (char* current = start; current < end; ++current) {
        if (*current == '?') {
            ++current;

            if (*current == '?')
                ++current;

            bytes.push_back(-1);
        }
        else {
            bytes.push_back(strtoul(current, &current, 16));
        }
    }

    return bytes;
}

uint8_t* PatternScanner::scan(const char* module_name, const char* pattern) {
    return scan(module_name, xorstr_("<unnamed>"), { PatternCandidate{ pattern } });
}

uint8_t* PatternScanner::scan(const char* module_name, const char* pattern, int offset)
{
    uint8_t* address = scan(module_name, pattern);
    if (address == nullptr)
        return nullptr;

    return address + offset;
}

uint8_t* PatternScanner::scan(const char* module_name, const char* signature_name, std::initializer_list<PatternCandidate> candidates) {
    for (const PatternCandidate& candidate : candidates) {
        uint8_t* address = scan_one(*this, module_name, candidate.bytes);
        if (address)
            return address;
    }

    PatternFailure failure{};
    failure.module_name = module_name ? module_name : xorstr_("<null>");
    failure.signature_name = signature_name ? signature_name : xorstr_("<unnamed>");

    for (const PatternCandidate& candidate : candidates)
        failure.candidates.push_back(candidate.bytes ? candidate.bytes : xorstr_("<null>"));

    m_failures.push_back(failure);

    LOG_ERROR(xorstr_("[patterns] failed: %s in %s; tried %llu candidate(s)"),
        failure.signature_name.c_str(),
        failure.module_name.c_str(),
        static_cast<unsigned long long>(failure.candidates.size()));

    for (std::size_t i = 0; i < failure.candidates.size(); ++i)
        LOG_ERROR(xorstr_("[patterns] candidate[%llu]: %s"), static_cast<unsigned long long>(i), failure.candidates[i].c_str());

    return nullptr;
}

uint8_t* PatternScanner::scan(const char* module_name, const char* signature_name, std::initializer_list<PatternCandidate> candidates, int offset)
{
    uint8_t* address = scan(module_name, signature_name, candidates);
    if (address == nullptr)
        return nullptr;

    return address + offset;
}

uint8_t* PatternScanner::scan_absolute(const char* module_name, const char* pattern) {
    uint8_t* address = scan(module_name, pattern);
    if (address == nullptr)
        return nullptr;

    return get_absolute_address(address);
}

uint8_t* PatternScanner::scan_absolute(const char* module_name, const char* pattern, int pre_offset) {
    uint8_t* address = scan(module_name, pattern);
    if (address == nullptr)
        return nullptr;

    return get_absolute_address(address, pre_offset);
}

uint8_t* PatternScanner::scan_absolute(const char* module_name, const char* pattern, int pre_offset, int post_offset) {
    uint8_t* address = scan(module_name, pattern);
    if (address == nullptr)
        return nullptr;

    return get_absolute_address(address, pre_offset, post_offset);
}

uint8_t* PatternScanner::scan_absolute(const char* module_name, const char* signature_name, std::initializer_list<PatternCandidate> candidates) {
    uint8_t* address = scan(module_name, signature_name, candidates);
    if (address == nullptr)
        return nullptr;

    return get_absolute_address(address);
}

uint8_t* PatternScanner::scan_absolute(const char* module_name, const char* signature_name, std::initializer_list<PatternCandidate> candidates, int pre_offset) {
    uint8_t* address = scan(module_name, signature_name, candidates);
    if (address == nullptr)
        return nullptr;

    return get_absolute_address(address, pre_offset);
}

uint8_t* PatternScanner::scan_absolute(const char* module_name, const char* signature_name, std::initializer_list<PatternCandidate> candidates, int pre_offset, int post_offset) {
    uint8_t* address = scan(module_name, signature_name, candidates);
    if (address == nullptr)
        return nullptr;

    return get_absolute_address(address, pre_offset, post_offset);
}

const std::vector<PatternFailure>& PatternScanner::failures() const {
    return m_failures;
}

void PatternScanner::clear_failures() {
    m_failures.clear();
}

void PatternScanner::log_failures() const {
    if (m_failures.empty())
        return;

    LOG_ERROR(xorstr_("[patterns] %llu recorded pattern failure(s)"), static_cast<unsigned long long>(m_failures.size()));

    for (const PatternFailure& failure : m_failures)
        LOG_ERROR(xorstr_("[patterns] recorded: %s in %s; tried %llu candidate(s)"),
            failure.signature_name.c_str(),
            failure.module_name.c_str(),
            static_cast<unsigned long long>(failure.candidates.size()));
}

unsigned __int64 PatternScanner::export_fn(unsigned __int64 base, const char* procedure_name)
{
    const auto dos_headers = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
    if (dos_headers->e_magic != IMAGE_DOS_SIGNATURE)
        return 0;

    const auto nt_headers = reinterpret_cast<const IMAGE_NT_HEADERS64*>(base + dos_headers->e_lfanew);
    if (nt_headers->Signature != IMAGE_NT_SIGNATURE)
        return 0;

    const std::uintptr_t export_directory_size = nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
    const std::uintptr_t export_directory_address = nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

    if (export_directory_size == 0u || export_directory_address == 0u)
        return 0;

    const auto exports = reinterpret_cast<const IMAGE_EXPORT_DIRECTORY*>(base + export_directory_address);
    const auto names_rva = reinterpret_cast<const std::uint32_t*>(base + exports->AddressOfNames);
    const auto name_ordinals_rva = reinterpret_cast<const std::uint16_t*>(base + exports->AddressOfNameOrdinals);
    const auto functions_rva = reinterpret_cast<const std::uint32_t*>(base + exports->AddressOfFunctions);

    std::size_t right = exports->NumberOfNames, left = 0u;
    while (right != left)
    {
        const std::size_t middle = left + ((right - left) >> 1u);
        const int result = strcmp(procedure_name, reinterpret_cast<const char*>(base + names_rva[middle]));

        if (result == 0)
        {
            const std::uint32_t function_rva = functions_rva[name_ordinals_rva[middle]];

            if (function_rva >= export_directory_address && function_rva - export_directory_address < export_directory_size)
                break;

            return base + function_rva;
        }

        if (result > 0)
            left = middle + 1;
        else
            right = middle;
    }
    return 0;
}

uint8_t* PatternScanner::get_absolute_address(unsigned char* address, int pre_offset, int post_offset) {
    address += pre_offset;
    address += *reinterpret_cast<std::int32_t*>(address) + 0x4;
    address += post_offset;

    return address;
}

uint8_t* PatternScanner::get_absolute_address(unsigned char* address, int pre_offset) {
    return get_absolute_address(address, pre_offset, 0);
}

uint8_t* PatternScanner::get_absolute_address(unsigned char* address) {
    return get_absolute_address(address, 0, 0);
}

uint8_t* PatternScanner::resolve_relative_address(unsigned char* address, int rva_offset, int rip_offset) {
    int32_t RVA = *reinterpret_cast<int32_t*>(address + rva_offset);
    uint64_t RIP = reinterpret_cast<uint64_t>(address) + rip_offset;

    return reinterpret_cast<uint8_t*>(RIP + RVA);
}

uint8_t* PatternScanner::resolve_relative_address(unsigned char* address, int rva_offset) {
    return resolve_relative_address(address, rva_offset, 0);
}
