#include "platform/ClientCore.hpp"

#include "features/matchmaking/AutoAccept.hpp"
#include "runtime/Runtime.hpp"

#include <atomic>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace {

std::atomic_bool g_auto_accept_pending = false;
DWORD64 g_auto_accept_not_before_ms = 0;
DWORD64 g_last_auto_accept_ms = 0;
using SetPlayerReadyFn = bool(__fastcall*)(void*, const char*);
SetPlayerReadyFn g_set_player_ready = nullptr;
constexpr DWORD64 k_auto_accept_cooldown_ms = 1500;
constexpr DWORD64 k_auto_accept_panorama_delay_ms = 1500;

struct PanoTextSwap {
	const char* from;
	const char* to;
};

static constexpr PanoTextSwap k_auto_accept_text_swaps[] = {
	{ "#match_ready_title", "Match found" },
	{ "#match_ready_accept", "Auto accepting" },
};

void auto_accept_file_log(const char* format, ...)
{
	if (!logger::m_save_to_file || logger::m_log_file_path[0] == '\0')
		return;

	char log_path[MAX_PATH] = {};
	strcpy_s(log_path, logger::m_log_file_path);
	char* last_slash = std::strrchr(log_path, '\\');
	if (last_slash)
		strcpy_s(last_slash + 1, MAX_PATH - static_cast<size_t>((last_slash + 1) - log_path), "auto_accept_log.txt");
	else
		strcpy_s(log_path, "auto_accept_log.txt");

	FILE* file = nullptr;
	if (fopen_s(&file, log_path, "a") != 0 || !file)
		return;

	fprintf(file, "[%llu] ", static_cast<unsigned long long>(GetTickCount64()));
	if (format) {
		va_list args;
		va_start(args, format);
		vfprintf(file, format, args);
		va_end(args);
	}
	fprintf(file, "\n");
	fclose(file);
}
std::vector<int> ida_to_bytes(const char* pattern)
{
	std::vector<int> bytes{};
	if (!pattern)
		return bytes;

	char* start = const_cast<char*>(pattern);
	char* end = const_cast<char*>(pattern) + std::strlen(pattern);

	for (char* current = start; current < end; ++current) {
		if (*current == '?') {
			++current;

			if (*current == '?')
				++current;

			bytes.push_back(-1);
		}
		else {
			bytes.push_back(std::strtoul(current, &current, 16));
		}
	}

	return bytes;
}

uint8_t* module_base(const char* module_name)
{
	if (!module_name)
		return nullptr;

	return reinterpret_cast<uint8_t*>(GetModuleHandleA(module_name));
}

size_t module_size(uint8_t* base)
{
	if (!base)
		return 0;

	auto* dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(base);
	if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
		return 0;

	auto* nt_header = reinterpret_cast<PIMAGE_NT_HEADERS>(base + dos_header->e_lfanew);
	if (nt_header->Signature != IMAGE_NT_SIGNATURE)
		return 0;

	return nt_header->OptionalHeader.SizeOfImage;
}

bool is_in_module(uint8_t* base, size_t size, const void* ptr)
{
	const auto address = reinterpret_cast<uintptr_t>(ptr);
	const auto module = reinterpret_cast<uintptr_t>(base);
	return address >= module && address < module + size;
}

std::vector<uint8_t*> scan_pattern_all(const char* module_name, const char* pattern)
{
	std::vector<uint8_t*> matches{};
	uint8_t* base = module_base(module_name);
	const size_t size = module_size(base);
	std::vector<int> bytes = ida_to_bytes(pattern);

	if (!base || size == 0 || bytes.empty() || bytes.size() > size)
		return matches;

	for (size_t i = 0; i <= size - bytes.size(); ++i) {
		bool found = true;
		for (size_t j = 0; j < bytes.size(); ++j) {
			if (bytes[j] == -1)
				continue;

			if (base[i + j] != static_cast<uint8_t>(bytes[j])) {
				found = false;
				break;
			}
		}

		if (found)
			matches.push_back(base + i);
	}

	return matches;
}

const char* resolve_rip_string(uint8_t* instruction, int imm_offset, int next_instruction_offset)
{
	if (!instruction)
		return nullptr;

	uint8_t* base = module_base(ttapp::client::modules().m_modules.client_dll.get_name());
	const size_t size = module_size(base);
	if (!base || size == 0)
		return nullptr;

	const int32_t relative = *reinterpret_cast<int32_t*>(instruction + imm_offset);
	const auto string_address = instruction + next_instruction_offset + relative;
	if (!is_in_module(base, size, string_address))
		return nullptr;

	return reinterpret_cast<const char*>(string_address);
}

bool has_expected_nearby_string_ref(uint8_t* address)
{
	if (!address)
		return false;

	constexpr size_t scan_length = 0x300;
	for (size_t i = 0; i + 7 < scan_length; ++i) {
		uint8_t* instruction = address + i;
		const char* value = nullptr;

		if (instruction[0] == 0x48 && instruction[1] == 0x8D && instruction[2] == 0x15)
			value = resolve_rip_string(instruction, 3, 7);
		else if (instruction[0] == 0x48 && instruction[1] == 0x8D && instruction[2] == 0x0D)
			value = resolve_rip_string(instruction, 3, 7);
		else if (instruction[0] == 0x48 && instruction[1] == 0x8B && instruction[2] == 0x0D)
			value = resolve_rip_string(instruction, 3, 7);
		else if (instruction[0] == 0x48 && instruction[1] == 0x8B && instruction[2] == 0x15)
			value = resolve_rip_string(instruction, 3, 7);

		if (!value)
			continue;

		if (std::strcmp(value, "deferred") == 0) {
			LOG_INFO(xorstr_("[auto_accept] SetPlayerReady validation found string xref: deferred"));
			auto_accept_file_log("SetPlayerReady validation found string xref: deferred");
			return true;
		}

		if (std::strncmp(value, "prematch:", 9) == 0) {
			LOG_INFO(xorstr_("[auto_accept] SetPlayerReady validation found string xref: prematch"));
			auto_accept_file_log("SetPlayerReady validation found string xref: prematch");
			return true;
		}
	}

	return false;
}

bool validate_set_player_ready(uint8_t* address)
{
	if (!address) {
		LOG_ERROR(xorstr_("[auto_accept] SetPlayerReady validation failed: null address"));
		auto_accept_file_log("SetPlayerReady validation failed: null address");
		return false;
	}

	constexpr uint8_t expected_prologue[] = { 0x40, 0x53, 0x48, 0x83, 0xEC, 0x20 };
	if (std::memcmp(address, expected_prologue, sizeof(expected_prologue)) != 0) {
		LOG_ERROR(xorstr_("[auto_accept] SetPlayerReady validation failed: prologue mismatch"));
		auto_accept_file_log("SetPlayerReady validation failed: prologue mismatch");
		return false;
	}

	if (!has_expected_nearby_string_ref(address)) {
		LOG_ERROR(xorstr_("[auto_accept] SetPlayerReady validation failed: no nearby deferred/prematch xref"));
		auto_accept_file_log("SetPlayerReady validation failed: no nearby deferred/prematch xref");
		return false;
	}

	LOG_INFO(xorstr_("[auto_accept] SetPlayerReady validation passed"));
	auto_accept_file_log("SetPlayerReady validation passed");
	return true;
}

void queue_auto_accept_after_delay(const char* reason, DWORD64 delay_ms)
{
	if (!ttapp::client::config().misc.m_auto_accept)
		return;

	g_auto_accept_not_before_ms = GetTickCount64() + delay_ms;
	if (!g_auto_accept_pending.exchange(true)) {
		LOG_INFO(xorstr_("[auto_accept] %s"), reason ? reason : "queued");
		auto_accept_file_log(reason ? reason : "queued");
	}
}

bool run_set_player_ready()
{
	if (!g_set_player_ready) {
		LOG_ERROR(xorstr_("[auto_accept] SetPlayerReady unavailable"));
		auto_accept_file_log("SetPlayerReady unavailable");
		return false;
	}

	if (!g_set_player_ready(nullptr, "accept")) {
		LOG_ERROR(xorstr_("[auto_accept] SetPlayerReady returned false"));
		auto_accept_file_log("SetPlayerReady returned false");
		return false;
	}

	LOG_INFO(xorstr_("[auto_accept] SetPlayerReady accept sent"));
	auto_accept_file_log("SetPlayerReady accept sent");
	return true;
}
} // namespace

void AutoAccept::initialize()
{
	const char* client_dll = ttapp::client::modules().m_modules.client_dll.get_name();
	uint8_t* base = module_base(client_dll);
	const auto matches = scan_pattern_all(client_dll,
		xorstr_("40 53 48 83 EC 20 48 8B DA 48 8D 15 ? ? ? ? 48 8B CB FF 15 ? ? ? ? 85 C0 75 14 BA 02 00"));

	if (matches.size() != 1) {
		auto_accept_file_log("SetPlayerReady pattern match count=%d expected=1 unused=(%d,%d,%d)",
			static_cast<int>(matches.size()), 0, 0, 0);
		LOG_ERROR(xorstr_("[auto_accept] SetPlayerReady pattern match count: %llu"), static_cast<unsigned long long>(matches.size()));
		return;
	}

	auto_accept_file_log("SetPlayerReady=0x%p rva=0x%llX",
		reinterpret_cast<void*>(matches[0]),
		static_cast<unsigned long long>(reinterpret_cast<uintptr_t>(matches[0]) - reinterpret_cast<uintptr_t>(base)));
	LOG_INFO(xorstr_("[auto_accept] SetPlayerReady=0x%p rva=0x%llX"),
		reinterpret_cast<void*>(matches[0]),
		static_cast<unsigned long long>(reinterpret_cast<uintptr_t>(matches[0]) - reinterpret_cast<uintptr_t>(base)));
	if (!validate_set_player_ready(matches[0])) {
		LOG_ERROR(xorstr_("[auto_accept] SetPlayerReady sanity checks failed"));
		return;
	}

	g_set_player_ready = reinterpret_cast<SetPlayerReadyFn>(matches[0]);
	LOG_INFO(xorstr_("[auto_accept] SetPlayerReady ready"));
}

const char* AutoAccept::resolve_panorama_text(const char* text)
{
	if (!text || !*text || !ttapp::client::config().misc.m_auto_accept_status_text)
		return text;

	for (const auto& swap : k_auto_accept_text_swaps) {
		if (std::strcmp(text, swap.from) == 0)
			return swap.to;
	}

	return text;
}

void AutoAccept::queue_if_match_ready(const char* text)
{
	if (!text || std::strcmp(text, "#match_ready_accept") != 0)
		return;

	if (!ttapp::client::config().misc.m_auto_accept)
		return;

	if (!g_auto_accept_pending.exchange(true)) {
		g_auto_accept_not_before_ms = GetTickCount64() + k_auto_accept_panorama_delay_ms;
		LOG_INFO(xorstr_("[auto_accept] match ready detected"));
		auto_accept_file_log("match ready detected");
	}
}

void AutoAccept::process_queue()
{
	if (!g_auto_accept_pending.load())
		return;

	if (!ttapp::client::config().misc.m_auto_accept) {
		g_auto_accept_pending = false;
		return;
	}

	const DWORD64 now = GetTickCount64();
	if (now < g_auto_accept_not_before_ms)
		return;

	if (now - g_last_auto_accept_ms < k_auto_accept_cooldown_ms)
		return;

	g_last_auto_accept_ms = now;
	g_auto_accept_pending = false;

	if (!run_set_player_ready()) {
		LOG_ERROR(xorstr_("[auto_accept] failed to call SetPlayerReady"));
		return;
	}
}

void AutoAccept::reset()
{
	g_auto_accept_pending = false;
	g_auto_accept_not_before_ms = 0;
	g_last_auto_accept_ms = 0;
}
void AutoAccept::on_match_found()
{
	if (ttapp::client::config().misc.m_auto_accept) {
		LOG_INFO(xorstr_("[auto_accept] match found handler fired"));
		auto_accept_file_log("match found handler fired");
		if (run_set_player_ready()) {
			g_last_auto_accept_ms = GetTickCount64();
			g_auto_accept_pending = false;
			LOG_INFO(xorstr_("[auto_accept] primary SetPlayerReady accept sent"));
			auto_accept_file_log("primary SetPlayerReady accept sent");
		}
		else {
			LOG_ERROR(xorstr_("[auto_accept] match found handler accept failed"));
		}
	}
}

void AutoAccept::on_panorama_event(const char* event_name)
{
	if (event_name && std::strcmp(event_name, "popup_accept_match_found") == 0) {
		LOG_INFO(xorstr_("[auto_accept] panorama event popup_accept_match_found"));
		auto_accept_file_log("panorama event popup_accept_match_found");
		queue_auto_accept_after_delay("panorama event delayed accept queued", k_auto_accept_panorama_delay_ms);
	}
}
