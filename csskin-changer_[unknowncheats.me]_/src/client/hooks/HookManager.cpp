#include "platform/ClientCore.hpp"

#include "runtime/Runtime.hpp"
#include "render/Dx11Renderer.hpp"
#include "ui/Menu.hpp"
#include "features/hud/SpectatorList.hpp"
#include "features/matchmaking/AutoAccept.hpp"
#include "features/movement/QuickStop.hpp"
#include "features/skins/AgentChanger.hpp"
#include "features/skins/GloveChanger.hpp"
#include "features/skins/SkinChanger.hpp"
#include "features/skins/SkinEventHandler.hpp"
#include "game/interfaces/i_csgo_input.hpp"
#include "game/interfaces/i_entity_system.hpp"
#include "game/interfaces/i_game_event.hpp"
#include "game/interfaces/i_mem_alloc.hpp"
#include "game/items/ItemSchema.hpp"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <initializer_list>

namespace {

bool g_minhook_initialized = false;

enum { LOOSE_VAR_COLOR4 = 9 };

struct CompositeMaterialInputLooseVariable_t {
	char* m_strName;
	uint8_t pad_008[0x38];
	int32_t m_nVariableType;
	uint8_t pad_044[0x48];
	uint32_t m_cValueColor4;
	uint8_t pad_090[0x288 - 0x90];
};

static_assert(offsetof(CompositeMaterialInputLooseVariable_t, m_nVariableType) == 0x40);
static_assert(offsetof(CompositeMaterialInputLooseVariable_t, m_cValueColor4) == 0x8C);
static_assert(sizeof(CompositeMaterialInputLooseVariable_t) == 0x288);

using append_loose_variable_fn = void(__fastcall*)(void*, const CompositeMaterialInputLooseVariable_t*);
append_loose_variable_fn g_append_loose_variable = nullptr;

uint8_t color_byte(float value)
{
	value = std::clamp(value, 0.0f, 1.0f);
	return static_cast<uint8_t>(value * 255.0f + 0.5f);
}

uint32_t pack_rgba(const float color[4])
{
	return static_cast<uint32_t>(color_byte(color[0]))
		| (static_cast<uint32_t>(color_byte(color[1])) << 8)
		| (static_cast<uint32_t>(color_byte(color[2])) << 16)
		| (static_cast<uint32_t>(color_byte(color[3])) << 24);
}

char* game_strdup(const char* text)
{
	const size_t size = strlen(text) + 1;
	auto* copy = static_cast<char*>(GameAlloc(size));
	if (!copy)
		return nullptr;

	memcpy(copy, text, size);
	return copy;
}

void append_color_variable(void* vec, const char* name, uint32_t rgba)
{
	if (!g_append_loose_variable)
		return;

	CompositeMaterialInputLooseVariable_t variable{};
	variable.m_strName = game_strdup(name);
	if (!variable.m_strName)
		return;

	variable.m_nVariableType = LOOSE_VAR_COLOR4;
	variable.m_cValueColor4 = rgba;
	g_append_loose_variable(vec, &variable);
}

void append_color4_variables(void* vec, const float colors[4][4])
{
	for (int i = 0; i < 4; ++i) {
		char name[16];
		sprintf_s(name, "g_vColor%d", i);
		append_color_variable(vec, name, pack_rgba(colors[i]));
	}
}

bool is_knife_definition(uint16_t def_index)
{
	return def_index == WEAPON_KNIFE || def_index == WEAPON_KNIFE_T || (def_index >= 500 && def_index <= 526);
}

struct material_entity_probe_t {
	c_econ_entity* entity = nullptr;
	c_econ_item_view* item = nullptr;
	uint16_t def_index = 0;
	int paint_kit_id = 0;
};

bool try_probe_material_entity(void* context, material_entity_probe_t& probe)
{
	__try {
		if (!valid_ptr(context))
			return false;

		auto* entity = reinterpret_cast<c_econ_entity*>(context);
		auto* attribute_manager = entity->m_attribute_manager();
		if (!valid_ptr(attribute_manager))
			return false;

		auto* item = attribute_manager->m_item();
		if (!valid_ptr(item))
			return false;

		probe.entity = entity;
		probe.item = item;
		probe.def_index = item->m_definition_index();
		probe.paint_kit_id = entity->m_paint_kit();
		return true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
}

int cached_paint_kit_id(uint16_t def_index, int selected_index)
{
	auto& schema = ttapp::client::item_schema();
	auto kits_it = schema.item_paint_kits.find(def_index);
	if (kits_it == schema.item_paint_kits.end()
		|| selected_index < 0
		|| selected_index >= static_cast<int>(kits_it->second.size()))
		return 0;

	return kits_it->second[selected_index].id;
}

bool try_append_weapon_colors(void* vec, const material_entity_probe_t& probe)
{
	const int config_index = Config::skin_changer_t::get_config_index(probe.def_index);
	if (config_index == 0)
		return false;

	auto& skin = ttapp::client::config().skin_changer.weapon_skins[config_index];
	if (!skin.custom_colors_enabled || skin.paint_kit == 0)
		return false;

	const int paint_kit_id = cached_paint_kit_id(probe.def_index, skin.paint_kit);
	if (paint_kit_id == 0 || probe.paint_kit_id != paint_kit_id)
		return false;

	append_color4_variables(vec, skin.custom_colors);
	return true;
}

bool try_append_knife_colors(void* vec, const material_entity_probe_t& probe)
{
	auto& cfg = ttapp::client::config().knife_changer;
	if (!cfg.m_custom_colors_enabled || cfg.m_knife == 0 || cfg.m_paint_kit == 0)
		return false;
	if (!is_knife_definition(probe.def_index))
		return false;

	auto& schema = ttapp::client::item_schema();
	if (cfg.m_knife < 0 || cfg.m_knife >= static_cast<int>(schema.knives.size()))
		return false;

	const uint16_t selected_knife = schema.knives[cfg.m_knife].definition_index;
	if (probe.def_index != selected_knife)
		return false;

	const int paint_kit_id = cached_paint_kit_id(selected_knife, cfg.m_paint_kit);
	if (paint_kit_id == 0 || probe.paint_kit_id != paint_kit_id)
		return false;

	append_color4_variables(vec, cfg.m_custom_colors);
	return true;
}

void on_build_material_colors_unsafe(void* vec, void* context)
{
	if (!vec)
		return;

	material_entity_probe_t probe{};
	const bool entity_probe_ok = try_probe_material_entity(context, probe);
	if (entity_probe_ok) {
		if (Config::skin_changer_t::get_config_index(probe.def_index) != 0
			&& ttapp::client::config().skin_changer.m_enabled
			&& try_append_weapon_colors(vec, probe))
			return;

		if (try_append_knife_colors(vec, probe))
			return;
	}
}

void __fastcall on_build_material_colors(void* vec, void* context)
{
	__try {
		on_build_material_colors_unsafe(vec, context);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		return;
	}
}

bool rel32_delta(uint8_t* source, const void* destination, int32_t& out)
{
	const auto delta = reinterpret_cast<intptr_t>(destination) - reinterpret_cast<intptr_t>(source + 5);
	if (delta < INT32_MIN || delta > INT32_MAX)
		return false;

	out = static_cast<int32_t>(delta);
	return true;
}

void emit_u32(std::vector<uint8_t>& code, uint32_t value)
{
	for (int i = 0; i < 4; ++i)
		code.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xFF));
}

void emit_u64(std::vector<uint8_t>& code, uint64_t value)
{
	for (int i = 0; i < 8; ++i)
		code.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xFF));
}

void emit_bytes(std::vector<uint8_t>& code, std::initializer_list<uint8_t> bytes)
{
	code.insert(code.end(), bytes.begin(), bytes.end());
}

void* allocate_near(uint8_t* target, size_t size)
{
	SYSTEM_INFO info{};
	GetSystemInfo(&info);

	const uintptr_t granularity = info.dwAllocationGranularity;
	const uintptr_t target_addr = reinterpret_cast<uintptr_t>(target);
	const uintptr_t max_app = reinterpret_cast<uintptr_t>(info.lpMaximumApplicationAddress);
	const uintptr_t range = 0x7FFF0000ull;
	const uintptr_t min_addr = target_addr > range ? target_addr - range : granularity;
	const uintptr_t max_addr = std::min(target_addr + range, max_app - size);
	const uintptr_t base = target_addr & ~(granularity - 1);

	for (uintptr_t offset = 0; offset <= range; offset += granularity) {
		if (base >= offset) {
			const uintptr_t candidate = (base - offset) & ~(granularity - 1);
			if (candidate >= min_addr) {
				if (void* result = VirtualAlloc(reinterpret_cast<void*>(candidate), size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE))
					return result;
			}
		}

		const uintptr_t candidate = (base + offset) & ~(granularity - 1);
		if (candidate <= max_addr) {
			if (void* result = VirtualAlloc(reinterpret_cast<void*>(candidate), size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE))
				return result;
		}
	}

	return nullptr;
}

bool write_rel_jump(uint8_t* source, void* destination)
{
	int32_t delta = 0;
	if (!rel32_delta(source, destination, delta))
		return false;

	DWORD old_protect = 0;
	if (!VirtualProtect(source, 5, PAGE_EXECUTE_READWRITE, &old_protect))
		return false;

	source[0] = 0xE9;
	memcpy(source + 1, &delta, sizeof(delta));
	FlushInstructionCache(GetCurrentProcess(), source, 5);

	DWORD unused = 0;
	VirtualProtect(source, 5, old_protect, &unused);
	return true;
}

class MaterialColorMidHook {
public:
	bool install(uint8_t* target)
	{
		if (m_target)
			return true;
		if (!target)
			return false;
		if (target[0] != 0x48 || target[1] != 0x8D || target[2] != 0x4C || target[3] != 0x24)
			return false;

		memcpy(m_original, target, sizeof(m_original));

		m_stub = static_cast<uint8_t*>(allocate_near(target, 256));
		if (!m_stub)
			return false;

		std::vector<uint8_t> code;
		code.reserve(192);

		emit_bytes(code, { 0x9C, 0x50, 0x51, 0x52, 0x41, 0x50, 0x41, 0x51, 0x41, 0x52, 0x41, 0x53, 0x41, 0x54 });
		emit_bytes(code, { 0x49, 0x89, 0xE4 });
		emit_bytes(code, { 0x48, 0x81, 0xEC });
		emit_u32(code, 0xA0);
		emit_bytes(code, { 0x48, 0x83, 0xE4, 0xF0 });

		emit_bytes(code, { 0xF3, 0x0F, 0x7F, 0x44, 0x24, 0x20 });
		emit_bytes(code, { 0xF3, 0x0F, 0x7F, 0x4C, 0x24, 0x30 });
		emit_bytes(code, { 0xF3, 0x0F, 0x7F, 0x54, 0x24, 0x40 });
		emit_bytes(code, { 0xF3, 0x0F, 0x7F, 0x5C, 0x24, 0x50 });
		emit_bytes(code, { 0xF3, 0x0F, 0x7F, 0x64, 0x24, 0x60 });
		emit_bytes(code, { 0xF3, 0x0F, 0x7F, 0x6C, 0x24, 0x70 });

		emit_bytes(code, { 0x4C, 0x89, 0xC1 });
		emit_bytes(code, { 0x48, 0x89, 0xDA });
		emit_bytes(code, { 0x48, 0xB8 });
		emit_u64(code, reinterpret_cast<uint64_t>(&on_build_material_colors));
		emit_bytes(code, { 0xFF, 0xD0 });

		emit_bytes(code, { 0xF3, 0x0F, 0x6F, 0x44, 0x24, 0x20 });
		emit_bytes(code, { 0xF3, 0x0F, 0x6F, 0x4C, 0x24, 0x30 });
		emit_bytes(code, { 0xF3, 0x0F, 0x6F, 0x54, 0x24, 0x40 });
		emit_bytes(code, { 0xF3, 0x0F, 0x6F, 0x5C, 0x24, 0x50 });
		emit_bytes(code, { 0xF3, 0x0F, 0x6F, 0x64, 0x24, 0x60 });
		emit_bytes(code, { 0xF3, 0x0F, 0x6F, 0x6C, 0x24, 0x70 });

		emit_bytes(code, { 0x4C, 0x89, 0xE4 });
		emit_bytes(code, { 0x41, 0x5C, 0x41, 0x5B, 0x41, 0x5A, 0x41, 0x59, 0x41, 0x58, 0x5A, 0x59, 0x58, 0x9D });
		code.insert(code.end(), m_original, m_original + sizeof(m_original));

		int32_t return_delta = 0;
		if (!rel32_delta(m_stub + code.size(), target + 5, return_delta)) {
			VirtualFree(m_stub, 0, MEM_RELEASE);
			m_stub = nullptr;
			return false;
		}
		code.push_back(0xE9);
		emit_u32(code, static_cast<uint32_t>(return_delta));

		if (code.size() > 256) {
			VirtualFree(m_stub, 0, MEM_RELEASE);
			m_stub = nullptr;
			return false;
		}

		memcpy(m_stub, code.data(), code.size());
		FlushInstructionCache(GetCurrentProcess(), m_stub, code.size());

		if (!write_rel_jump(target, m_stub)) {
			VirtualFree(m_stub, 0, MEM_RELEASE);
			m_stub = nullptr;
			return false;
		}

		m_target = target;
		return true;
	}

	void uninstall()
	{
		if (m_target) {
			DWORD old_protect = 0;
			if (VirtualProtect(m_target, sizeof(m_original), PAGE_EXECUTE_READWRITE, &old_protect)) {
				memcpy(m_target, m_original, sizeof(m_original));
				FlushInstructionCache(GetCurrentProcess(), m_target, sizeof(m_original));
				DWORD unused = 0;
				VirtualProtect(m_target, sizeof(m_original), old_protect, &unused);
			}
			m_target = nullptr;
		}

		if (m_stub) {
			VirtualFree(m_stub, 0, MEM_RELEASE);
			m_stub = nullptr;
		}
	}

private:
	uint8_t* m_target = nullptr;
	uint8_t* m_stub = nullptr;
	uint8_t m_original[5] = {};
};

MaterialColorMidHook g_material_color_mid_hook;

bool install_hook(const char* name, Hook& hook, void* target, void* detour)
{
	if (!target) {
		LOG_ERROR(xorstr_("[hooks] missing target: %s"), name);
		return false;
	}

	if (!hook.hook(target, detour)) {
		LOG_ERROR(xorstr_("[hooks] install failed: %s"), name);
		return false;
	}

	return true;
}

bool fail_hook_initialize(const char* name)
{
	LOG_ERROR(xorstr_("[hooks] initialization failed: %s"), name);
	ttapp::client::hooks().destroy();
	return false;
}

void install_optional_panorama_hooks()
{
	const char* panorama_dll = ttapp::client::modules().m_modules.panorama_dll.get_name();
	if (!panorama_dll)
		return;

	auto* construct_string = ttapp::client::patterns().scan(panorama_dll, xorstr_("hooks.panorama.construct_string"), {
		{ xorstr_("48 89 5C 24 ? 48 89 6C 24 ? 57 41 56 41 57 48 83 EC ? 4D 8B D0") },
	});
	if (install_hook(xorstr_("hooks.panorama.construct_string"), hooks::panorama_construct_string::m_construct_string,
		construct_string, hooks::panorama_construct_string::hk_construct_string)) {
		LOG_INFO(xorstr_("[auto_accept] construct string hook installed"));
	}

	auto* get_string = ttapp::client::patterns().scan(panorama_dll, xorstr_("hooks.panorama.get_string"), {
		{ xorstr_("48 8B 51 10 48 8D 05 ? ? ? ? 48 85 D2 48 0F 45 C2 C3") },
	});
	if (install_hook(xorstr_("hooks.panorama.get_string"), hooks::panorama_get_string::m_get_string,
		get_string, hooks::panorama_get_string::hk_get_string)) {
		LOG_INFO(xorstr_("[auto_accept] get string hook installed"));
	}
}

void install_optional_match_found_handler()
{
	const char* client_dll = ttapp::client::modules().m_modules.client_dll.get_name();
	auto* target = ttapp::client::patterns().scan(client_dll, xorstr_("hooks.auto_accept.match_found_handler"), {
		{ xorstr_("48 83 EC 28 48 8B 0D ? ? ? ? 48 85 C9 74 ? 48 8B 01 48 89 7C 24 20 FF 50 78") },
	});

	if (install_hook(xorstr_("hooks.auto_accept.match_found_handler"), hooks::match_found_handler::m_match_found_handler,
		target, hooks::match_found_handler::hk_match_found_handler)) {
		LOG_INFO(xorstr_("[auto_accept] match found handler hook installed"));
	}
}

void install_optional_panorama_event_hook()
{
	const char* client_dll = ttapp::client::modules().m_modules.client_dll.get_name();
	auto* target = ttapp::client::patterns().scan(client_dll, xorstr_("hooks.auto_accept.panorama_event"), {
		{ xorstr_("40 56 57 41 57 48 83 EC 40 48 8B 3D ? ? ? ? 4D 85 C0 48 8B 35 ? ? ? ? 4C 8B FA 49 0F 45 F8") },
	});

	if (install_hook(xorstr_("hooks.auto_accept.panorama_event"), hooks::panorama_event::m_panorama_event,
		target, hooks::panorama_event::hk_panorama_event)) {
		LOG_INFO(xorstr_("[auto_accept] panorama event hook installed"));
	}
}

void install_optional_material_color_hook(const char* client_dll)
{
	if (!client_dll)
		return;

	auto* append_call = ttapp::client::patterns().scan(client_dll, xorstr_("hooks.material_colors.append_loose_variable"), {
		{ xorstr_("E8 ? ? ? ? 0F 28 B4 24 ? ? ? ? 4C 39 A5") },
	});
	if (!append_call)
		return;

	g_append_loose_variable = reinterpret_cast<append_loose_variable_fn>(
		ttapp::client::patterns().resolve_relative_address(append_call, 1, 5));
	if (!g_append_loose_variable) {
		LOG_ERROR(xorstr_("[material_colors] append function resolve failed"));
		return;
	}

	auto* build_material = ttapp::client::patterns().scan(client_dll, xorstr_("hooks.material_colors.build_material"), {
		{ xorstr_("48 8D 15 ? ? ? ? 48 8D 4C 24 ? E8 ? ? ? ? 48 8B D0 48 8D 8B") },
	});
	if (!build_material)
		return;

	if (g_material_color_mid_hook.install(build_material + 7))
		LOG_INFO(xorstr_("[material_colors] build material mid-hook installed"));
	else
		LOG_ERROR(xorstr_("[material_colors] build material mid-hook install failed"));
}
} // namespace
bool HookManager::initialize() {
	MH_STATUS initialize_status = MH_Initialize();
	if (initialize_status != MH_OK && initialize_status != MH_ERROR_ALREADY_INITIALIZED) {
		LOG_ERROR(xorstr_("[hooks] MH_Initialize failed: %d"), initialize_status);
		return false;
	}
	g_minhook_initialized = true;

	ttapp::client::auto_accept().initialize();

	if (!install_hook(xorstr_("create_move"), hooks::create_move::m_create_move, vmt::get_v_method(ttapp::client::interfaces().m_csgo_input, 5), hooks::create_move::hk_create_move))
		return fail_hook_initialize(xorstr_("create_move"));
	if (!install_hook(xorstr_("mouse_input_enabled"), hooks::mouse_input_enabled::m_mouse_input_enabled, vmt::get_v_method(ttapp::client::interfaces().m_csgo_input, 23), hooks::mouse_input_enabled::hk_mouse_input_enabled))
		return fail_hook_initialize(xorstr_("mouse_input_enabled"));
	if (!install_hook(xorstr_("enable_cursor"), hooks::enable_cursor::m_enable_cursor, vmt::get_v_method(ttapp::client::interfaces().m_input_system, 76), hooks::enable_cursor::hk_enable_cursor))
		return fail_hook_initialize(xorstr_("enable_cursor"));

	const char* client_dll = ttapp::client::modules().m_modules.client_dll.get_name();

	if (!install_hook(xorstr_("frame_stage_notify"), hooks::frame_stage_notify::m_frame_stage_notify,
		ttapp::client::patterns().scan(client_dll, xorstr_("hooks.frame_stage_notify"), {
			{ xorstr_("48 89 5C 24 ? 48 89 6C 24 ? 57 48 83 EC 40 48 8B F9 33 ED") },
		}),
		hooks::frame_stage_notify::hk_frame_stage_notify))
		return fail_hook_initialize(xorstr_("frame_stage_notify"));

	{
		i_game_event::get_name = reinterpret_cast<i_game_event::GetNameFn>(ttapp::client::patterns().scan(client_dll, xorstr_("hooks.game_event.get_name"), {
			{ xorstr_("8B 41 14 0F BA E0 1E 73 05 48 8D 41 18 C3") },
		}));
		i_game_event::get_string = reinterpret_cast<i_game_event::GetStringFn>(ttapp::client::patterns().scan(client_dll, xorstr_("hooks.game_event.get_string"), {
			{ xorstr_("48 83 EC 38 8B 02 48 83 C1 58 89 44 24 20 8B 42 04 89 44 24 24 48 8B 42 08 48 8D 54 24 20 48 89 44 24 28 E8 ? ? ? ? 48 83 C4 38 C3 CC CC CC 33 C9") },
		}));
		i_game_event::set_string = reinterpret_cast<i_game_event::SetStringFn>(ttapp::client::patterns().scan(client_dll, xorstr_("hooks.game_event.set_string"), {
			{ xorstr_("48 83 EC 38 8B 02 48 83 C1 58 89 44 24 20 41 B1 1A") },
		}));
		i_game_event::get_player_controller = reinterpret_cast<i_game_event::GetPlayerControllerFn>(ttapp::client::patterns().scan(client_dll, xorstr_("hooks.game_event.get_player_controller"), {
			{ xorstr_("48 83 EC 38 8B 02 4C 8D 44 24 20") },
		}));

		if (!i_game_event::get_name || !i_game_event::get_string || !i_game_event::set_string || !i_game_event::get_player_controller)
			return fail_hook_initialize(xorstr_("game_event_functions"));
	}

	if (!install_hook(xorstr_("fire_event_client_side"), hooks::fire_event_client_side::m_fire_event_client_side,
		ttapp::client::patterns().scan(client_dll, xorstr_("hooks.fire_event_client_side"), {
			{ xorstr_("40 53 41 54 41 56 48 83 EC ? 4C 8B F2") },
		}),
		hooks::fire_event_client_side::hk_fire_event_client_side))
		return fail_hook_initialize(xorstr_("fire_event_client_side"));

	if (!install_hook(xorstr_("level_init"), hooks::level_init::m_level_init,
		ttapp::client::patterns().scan(client_dll, xorstr_("hooks.level_init"), {
			{ xorstr_("40 55 56 41 56 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 0D") },
		}),
		hooks::level_init::hk_level_init))
		return fail_hook_initialize(xorstr_("level_init"));

	if (!install_hook(xorstr_("present"), hooks::present::m_present, ttapp::client::renderer().m_present_address, hooks::present::hk_present))
		return fail_hook_initialize(xorstr_("present"));
	if (!install_hook(xorstr_("resize_buffers"), hooks::resize_buffers::m_resize_buffers, ttapp::client::renderer().m_resize_buffers_address, hooks::resize_buffers::hk_resize_buffers))
		return fail_hook_initialize(xorstr_("resize_buffers"));
	if (!install_hook(xorstr_("create_swap_chain"), hooks::create_swap_chain::m_create_swap_chain, ttapp::client::renderer().m_create_swap_chain_address, hooks::create_swap_chain::hk_create_swap_chain))
		return fail_hook_initialize(xorstr_("create_swap_chain"));

	install_optional_panorama_hooks();
	install_optional_match_found_handler();
	install_optional_panorama_event_hook();
	install_optional_material_color_hook(client_dll);

	return true;
}

void HookManager::destroy() {
	if (ttapp::client::menu().m_opened) {
		if (ImGui::GetCurrentContext())
			ImGui::GetIO().MouseDrawCursor = false;
		ShowCursor(TRUE);

		auto original = hooks::enable_cursor::m_enable_cursor.get_original<decltype(&hooks::enable_cursor::hk_enable_cursor)>();
		if (original) {
			__try {
				original(ttapp::client::interfaces().m_input_system, hooks::enable_cursor::m_enable_cursor_input);
			}
			__except (EXCEPTION_EXECUTE_HANDLER) {}
		}
	}

	ttapp::client::menu().m_opened = true;

	hooks::present::m_present.unhook();
	hooks::resize_buffers::m_resize_buffers.unhook();
	hooks::create_swap_chain::m_create_swap_chain.unhook();

	Sleep(200);

	hooks::create_move::m_create_move.unhook();
	hooks::mouse_input_enabled::m_mouse_input_enabled.unhook();
	hooks::enable_cursor::m_enable_cursor.unhook();
	hooks::frame_stage_notify::m_frame_stage_notify.unhook();
	hooks::fire_event_client_side::m_fire_event_client_side.unhook();
	hooks::level_init::m_level_init.unhook();
	hooks::match_found_handler::m_match_found_handler.unhook();
	hooks::panorama_event::m_panorama_event.unhook();
	hooks::panorama_construct_string::m_construct_string.unhook();
	hooks::panorama_get_string::m_get_string.unhook();
	g_material_color_mid_hook.uninstall();
	ttapp::client::auto_accept().reset();

	Sleep(100);

	ttapp::client::renderer().uninitialize();

	if (g_minhook_initialized) {
		MH_Uninitialize();
		g_minhook_initialized = false;
	}
}

bool __fastcall hooks::mouse_input_enabled::hk_mouse_input_enabled(void* ptr) {
	auto original = m_mouse_input_enabled.get_original<decltype(&hk_mouse_input_enabled)>();
	return ttapp::client::menu().m_opened ? false : original(ptr);
}

void* __fastcall hooks::enable_cursor::hk_enable_cursor(void* rcx, bool active) {
	auto original = m_enable_cursor.get_original<decltype(&hk_enable_cursor)>();

	m_enable_cursor_input = active;
	if (ttapp::client::menu().m_opened)
		active = false;

	return original(rcx, active);
}

void __fastcall hooks::create_move::hk_create_move(i_csgo_input* rcx, int slot, bool active) {
	auto original = m_create_move.get_original<decltype(&hk_create_move)>();

	original(rcx, slot, active);

	ttapp::client::frame().m_local_pawn = ttapp::client::interfaces().m_entity_system->get_local_pawn();
	ttapp::client::frame().m_local_controller = ttapp::client::interfaces().m_entity_system->get_local_controller();

	if (!ttapp::client::frame().m_local_controller)
		return;

	ttapp::client::frame().m_user_cmd = rcx->get_user_cmd(ttapp::client::frame().m_local_controller);

	ttapp::client::menu().on_create_move();
	ttapp::client::quick_stop().run(rcx, slot, active);
}

void hooks::frame_stage_notify::hk_frame_stage_notify(void* source_to_client, int stage) {
	auto original = m_frame_stage_notify.get_original<decltype(&hk_frame_stage_notify)>();

	if (stage == 7) {

		ttapp::client::frame().m_local_controller = ttapp::client::interfaces().m_entity_system->get_local_controller();
		ttapp::client::frame().m_local_pawn = ttapp::client::interfaces().m_entity_system->get_local_pawn();

		if (ttapp::client::frame().m_local_pawn != nullptr && ttapp::client::frame().m_local_controller != nullptr) {
			ttapp::client::skin_changer().run(stage);
			ttapp::client::glove_changer().run(stage);
			ttapp::client::agent_changer().run(stage);
		}
	}

	original(source_to_client, stage);
}
__int64 __fastcall hooks::level_init::hk_level_init(void* rcx, void* rdx) {
	auto original = m_level_init.get_original<decltype(&hk_level_init)>();
	ttapp::client::spectator_list().pause(5.0f);
	ttapp::client::frame().m_user_cmd = nullptr;
	ttapp::client::frame().m_local_pawn = nullptr;
	ttapp::client::frame().m_local_controller = nullptr;
	skin_events::on_level_init();

	return original(rcx, rdx);
}

bool __fastcall hooks::fire_event_client_side::hk_fire_event_client_side(void* p_game_event_manager, void* p_game_event) {
	auto original = m_fire_event_client_side.get_original<decltype(&hk_fire_event_client_side)>();

	if (!p_game_event || !i_game_event::get_name)
		return original(p_game_event_manager, p_game_event);

	const char* event_name = i_game_event::get_name(p_game_event);
	if (!event_name)
		return original(p_game_event_manager, p_game_event);

	skin_events::on_game_event(skin_events::hash_event_name(event_name), p_game_event);

	return original(p_game_event_manager, p_game_event);
}
HRESULT hooks::present::hk_present(IDXGISwapChain* swap_chain, unsigned int sync_interval, unsigned int flags) {
	auto original = m_present.get_original<decltype(&hk_present)>();

	ttapp::client::auto_accept().process_queue();

	ttapp::client::renderer().start_frame(swap_chain);

	auto* device_context = ttapp::client::renderer().get_device_context();
	auto* render_target = ttapp::client::renderer().get_render_target();
	if (device_context && render_target) {
		device_context->OMSetRenderTargets(1, &render_target, nullptr);

		ttapp::client::renderer().new_frame();

		ttapp::client::menu().draw();
		ttapp::client::spectator_list().draw();
		ttapp::client::renderer().end_frame();
	}

	return original(swap_chain, sync_interval, flags);
}

HRESULT hooks::resize_buffers::hk_resize_buffers(IDXGISwapChain* swap_chain, UINT buffer_count, UINT width, UINT height, DXGI_FORMAT new_format, UINT swap_chain_flags) {
	auto original = m_resize_buffers.get_original<decltype(&hk_resize_buffers)>();

	ttapp::client::renderer().destroy_render_target();
	ttapp::client::renderer().invalidate_device_objects();

	HRESULT result = original(swap_chain, buffer_count, width, height, new_format, swap_chain_flags);
	if (SUCCEEDED(result))
		ttapp::client::renderer().on_resize_buffers();

	return result;
}

HRESULT __stdcall hooks::create_swap_chain::hk_create_swap_chain(IDXGIFactory* factory, IUnknown* device, DXGI_SWAP_CHAIN_DESC* desc, IDXGISwapChain** swap_chain) {
	auto original = m_create_swap_chain.get_original<decltype(&hk_create_swap_chain)>();

	ttapp::client::renderer().destroy_render_target();
	return original(factory, device, desc, swap_chain);
}
void __fastcall hooks::match_found_handler::hk_match_found_handler() {
	auto original = m_match_found_handler.get_original<decltype(&hk_match_found_handler)>();

	ttapp::client::auto_accept().on_match_found();

	original();
}

__int64 __fastcall hooks::panorama_event::hk_panorama_event(void* rcx, const char* event_name, void* arg, float value) {
	auto original = m_panorama_event.get_original<decltype(&hk_panorama_event)>();

	ttapp::client::auto_accept().on_panorama_event(event_name);

	return original(rcx, event_name, arg, value);
}

void* __fastcall hooks::panorama_construct_string::hk_construct_string(void* loc, void* panel, const char* text, int64_t ctx, char escapes) {
	auto original = m_construct_string.get_original<decltype(&hk_construct_string)>();

	ttapp::client::auto_accept().queue_if_match_ready(text);
	return original(loc, panel, ttapp::client::auto_accept().resolve_panorama_text(text), ctx, escapes);
}

const char* __fastcall hooks::panorama_get_string::hk_get_string(void* self) {
	auto original = m_get_string.get_original<decltype(&hk_get_string)>();
	const char* text = original(self);

	return ttapp::client::auto_accept().resolve_panorama_text(text);
}
