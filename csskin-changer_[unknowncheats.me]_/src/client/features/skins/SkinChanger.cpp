#include "runtime/Runtime.hpp"
#include "features/skins/SkinChanger.hpp"
#include "game/items/EconItemAttributeManager.hpp"
#include "game/items/ItemSchema.hpp"
#include "game/InterfaceRegistry.hpp"
#include "game/SchemaRegistry.hpp"
#include "game/interfaces/i_econ_item_system.hpp"
#include "ui/Menu.hpp"
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
#include <utility>

namespace {

using anim_graph_rebuild_fn = void(__fastcall*)(void*, std::uint8_t);
using hud_model_arms_refresh_fn = void(__fastcall*)(void*, bool);
using remove_keychain_entity_fn = void(__fastcall*)(c_base_entity*);

void stattrak_file_log(const char* format, ...) {
	char log_path[MAX_PATH] = {};
	logger::build_path(log_path, sizeof(log_path), "stattrak_log.txt");
	FILE* file = nullptr;
	if (fopen_s(&file, log_path, "a") != 0 || !file)
		return;

	fprintf(file, "[%llu] ", static_cast<unsigned long long>(GetTickCount64()));

	va_list args;
	va_start(args, format);
	vfprintf(file, format, args);
	va_end(args);

	fprintf(file, "\n");
	fclose(file);
}

std::uintptr_t read_ptr(std::uintptr_t address) {
	__try {
		return *reinterpret_cast<std::uintptr_t*>(address);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		return 0;
	}
}

void safe_rebuild_anim_graph(anim_graph_rebuild_fn fn, std::uintptr_t controller) {
	__try {
		fn(reinterpret_cast<void*>(controller), 2u);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {}
}

void safe_refresh_hud_arms(hud_model_arms_refresh_fn fn, void* hud_arms) {
	__try {
		fn(hud_arms, true);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {}
}

void safe_update_composite(c_econ_entity* weapon) {
	if (!valid_ptr(weapon))
		return;

	__try {
		weapon->update_composite(true);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {}
}

void safe_update_skin(c_econ_entity* weapon) {
	if (!valid_ptr(weapon))
		return;

	__try {
		weapon->update_skin(true);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {}
}

void safe_update_weapon_data(c_econ_entity* weapon) {
	if (!valid_ptr(weapon))
		return;

	__try {
		weapon->update_weapon_data();
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {}
}

void safe_update_subclass(c_econ_entity* weapon, uint16_t def_index) {
	if (!valid_ptr(weapon))
		return;

	__try {
		weapon->update_subclass(def_index);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {}
}

void safe_set_mesh_group_mask(c_base_entity* entity, uint64_t mesh_mask) {
	if (!valid_ptr(entity))
		return;

	__try {
		if (auto* scene_node = entity->m_scene_node())
			scene_node->set_mesh_group_mask(mesh_mask);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {}
}

bool safe_remove_keychain_entity(remove_keychain_entity_fn fn, c_base_entity* entity) {
	if (!fn || !valid_ptr(entity))
		return false;

	bool succeeded = true;
	__try {
		fn(entity);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		succeeded = false;
	}
	return succeeded;
}

remove_keychain_entity_fn keychain_entity_remove_fn() {
	static auto fn = reinterpret_cast<remove_keychain_entity_fn>(
		ttapp::client::patterns().scan(ttapp::client::modules().m_modules.client_dll.get_name(),
			"48 83 EC ? 48 85 C9 74 ? 48 8B 51 ? 8B 42 ? 0F BA E0 ?")
	);
	return fn;
}

float int_as_attribute_float(int value) {
	static_assert(sizeof(float) == sizeof(int), "attribute integer packing expects 32-bit float and int");
	float result = 0.0f;
	memcpy(&result, &value, sizeof(result));
	return result;
}

uint32_t keychain_cache_item_id_low(uint16_t def_index, int keychain_id, int keychain_seed) {
	uint32_t value = 0x80000000u;
	value ^= static_cast<uint32_t>(def_index) * 0x45D9F3Bu;
	value ^= static_cast<uint32_t>(keychain_id) * 0x119DE1F3u;
	value ^= static_cast<uint32_t>(keychain_seed) * 0x3449A9A9u;
	return value ? value : 0x80000000u;
}

uint32_t fresh_weapon_cache_item_id_low(uint16_t def_index, int cosmetic_id, int seed) {
	static uint32_t serial = 0;
	uint32_t value = keychain_cache_item_id_low(def_index, cosmetic_id, seed);
	value ^= ++serial * 0x9E3779B9u;
	return value | 0x80000000u;
}

c_paint_kit* find_paint_kit(int paint_kit_id) {
	if (paint_kit_id <= 0)
		return nullptr;

	auto* source2_client = ttapp::client::interfaces().m_source2_client;
	auto* item_system = source2_client ? source2_client->get_econ_item_system() : nullptr;
	auto* item_schema = item_system ? item_system->get_econ_item_schema() : nullptr;
	return item_schema ? item_schema->get_paint_kits().find_by_key(paint_kit_id) : nullptr;
}

void sync_constructed_paint_kit(c_econ_item_view* item, int paint_kit_id) {
	if (!valid_ptr(item))
		return;

	static const char default_paint_kit_name[] = "default";

	__try {
		auto* paint_kit = item->construct_paint_kit();
		if (!valid_ptr(paint_kit))
			return;

		if (paint_kit_id <= 0) {
			paint_kit->m_id = 0;
			paint_kit->m_name = default_paint_kit_name;
			return;
		}

		if (auto* desired = find_paint_kit(paint_kit_id)) {
			paint_kit->m_id = desired->m_id;
			paint_kit->m_name = desired->m_name;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {}
}

using find_keychain_entity_fn = bool(__fastcall*)(c_cs_weapon_base*, std::uint32_t*);

char ascii_lower(char c) {
	return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + ('a' - 'A')) : c;
}

bool contains_ascii_icase(const char* text, const char* needle) {
	if (!text || !needle || !needle[0])
		return false;

	for (const char* cursor = text; *cursor; ++cursor) {
		int i = 0;
		while (needle[i] && cursor[i] && ascii_lower(cursor[i]) == ascii_lower(needle[i]))
			++i;
		if (!needle[i])
			return true;
	}
	return false;
}

bool safe_contains_ascii_icase(const char* text, const char* needle) {
	__try {
		return contains_ascii_icase(text, needle);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
}

void safe_copy_text(char* out, size_t out_size, const char* text) {
	if (!out || out_size == 0)
		return;
	out[0] = '\0';
	if (!text)
		return;

	__try {
		size_t i = 0;
		for (; i + 1 < out_size && text[i]; ++i)
			out[i] = text[i];
		out[i] = '\0';
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		strcpy_s(out, out_size, "?");
	}
}

int safe_entity_handle(c_base_entity* entity) {
	if (!valid_ptr(entity))
		return static_cast<int>(INVALID_EHANDLE_INDEX);

	__try {
		return entity->get_handle().to_int();
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		return static_cast<int>(INVALID_EHANDLE_INDEX);
	}
}

int safe_owner_entity_handle(c_base_entity* entity) {
	if (!valid_ptr(entity))
		return static_cast<int>(INVALID_EHANDLE_INDEX);

	__try {
		return entity->m_owner_entity().to_int();
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		return static_cast<int>(INVALID_EHANDLE_INDEX);
	}
}

find_keychain_entity_fn keychain_entity_find_fn() {
	static auto find_fn = []() -> find_keychain_entity_fn {
		auto* sequence = ttapp::client::patterns().scan(
			ttapp::client::modules().m_modules.client_dll.get_name(),
			"48 8D 54 24 40 C7 44 24 40 FF FF FF FF 49 8B CE E8 ? ? ? ? 8B 4C 24 40 41 BC FF 7F 00 00");
		if (!sequence)
			return nullptr;
		auto* call = sequence + 16;
		return reinterpret_cast<find_keychain_entity_fn>(
			call + 5 + *reinterpret_cast<std::int32_t*>(call + 1));
	}();
	return find_fn;
}

bool get_live_keychain(c_cs_weapon_base* weapon, c_base_entity** live_entity = nullptr) {
	auto find_fn = keychain_entity_find_fn();
	auto* entity_system = ttapp::client::interfaces().m_entity_system;
	std::uint32_t handle = INVALID_EHANDLE_INDEX;
	c_base_entity* entity = nullptr;
	__try {
		if (!find_fn || !entity_system || !find_fn(weapon, &handle) || handle == INVALID_EHANDLE_INDEX)
			return false;

		entity = reinterpret_cast<c_base_entity*>(
			entity_system->get_base_entity(handle & ENT_ENTRY_MASK));
		if (!valid_ptr(entity))
			return false;
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}

	if (live_entity)
		*live_entity = entity;
	return true;
}

bool has_weapon_extra_cosmetics(const Config::skin_changer_t::weapon_skin_t& skin) {
	for (int slot = 0; slot < Config::skin_changer_t::sticker_slot_count; ++slot)
		if (skin.stickers[slot].id > 0)
			return true;

	return skin.keychain.id > 0 || skin.stattrak_enabled;
}

bool has_live_weapon_cosmetics(c_econ_entity* weapon, c_econ_item_view* item) {
	if (!valid_ptr(weapon) || !valid_ptr(item))
		return false;

	if (weapon->m_paint_kit() != 0 || weapon->m_StatTrak() >= 0 || item->m_entity_quality() == QUALITY_STRANGE)
		return true;

	__try {
		if (item->m_custom_name()[0] != '\0')
			return true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}

	float value = 0.0f;
	if (econ_item_attribute_manager::get_value(item, 80, value) && value != 0.0f)
		return true;
	if (econ_item_attribute_manager::get_value(item, 299, value) && value != 0.0f)
		return true;
	for (int slot = 0; slot < Config::skin_changer_t::sticker_slot_count; ++slot) {
		if (econ_item_attribute_manager::get_value(item, static_cast<uint16_t>(113 + slot * 4), value) && value != 0.0f)
			return true;
	}

	return get_live_keychain(reinterpret_cast<c_cs_weapon_base*>(weapon));
}

bool sticker_slots_match(
	const Config::skin_changer_t::sticker_slot_t (&lhs)[Config::skin_changer_t::sticker_slot_count],
	const Config::skin_changer_t::sticker_slot_t (&rhs)[Config::skin_changer_t::sticker_slot_count]) {
	for (int slot = 0; slot < Config::skin_changer_t::sticker_slot_count; ++slot) {
		const auto& a = lhs[slot];
		const auto& b = rhs[slot];
		if (a.id != b.id
			|| a.wear != b.wear
			|| a.scale != b.scale
			|| a.rotation != b.rotation
			|| a.offset_x != b.offset_x
			|| a.offset_y != b.offset_y)
			return false;
	}
	return true;
}

bool keychains_match(const Config::skin_changer_t::keychain_t& lhs, const Config::skin_changer_t::keychain_t& rhs) {
	return lhs.id == rhs.id
		&& lhs.offset_x == rhs.offset_x
		&& lhs.offset_y == rhs.offset_y
		&& lhs.offset_z == rhs.offset_z
		&& lhs.seed == rhs.seed;
}

void append_weapon_extra_attributes(
	std::vector<econ_item_attribute_manager::attribute_value_t>& attributes,
	const Config::skin_changer_t::weapon_skin_t& skin) {
	for (int slot = 0; slot < Config::skin_changer_t::sticker_slot_count; ++slot) {
		const auto& sticker = skin.stickers[slot];
		const auto sticker_base = static_cast<uint16_t>(113 + slot * 4);
		if (sticker.id <= 0) {
			attributes.push_back({ sticker_base, int_as_attribute_float(0) });
			continue;
		}

		const auto offset_base = static_cast<uint16_t>(278 + slot * 2);
		attributes.push_back({ sticker_base, int_as_attribute_float(sticker.id) });
		attributes.push_back({ static_cast<uint16_t>(sticker_base + 1), sticker.wear });
		attributes.push_back({ static_cast<uint16_t>(sticker_base + 2), sticker.scale });
		attributes.push_back({ static_cast<uint16_t>(sticker_base + 3), sticker.rotation });
		attributes.push_back({ offset_base, sticker.offset_x });
		attributes.push_back({ static_cast<uint16_t>(offset_base + 1), sticker.offset_y });

		const bool uses_custom_placement = slot == 4
			|| sticker.rotation != 0.0f
			|| sticker.offset_x != 0.0f
			|| sticker.offset_y != 0.0f;
		if (uses_custom_placement)
			attributes.push_back({ static_cast<uint16_t>(290 + slot), int_as_attribute_float(1) });
	}

	if (skin.stattrak_enabled) {
		const int stattrak_count = skin.stattrak_count < 0 ? 0 : skin.stattrak_count;
		attributes.push_back({ 80, int_as_attribute_float(stattrak_count) });
		attributes.push_back({ 81, int_as_attribute_float(0) });
	}

	if (skin.keychain.id <= 0)
		return;

	attributes.push_back({ 299, int_as_attribute_float(skin.keychain.id) });
	attributes.push_back({ 300, skin.keychain.offset_x });
	attributes.push_back({ 301, skin.keychain.offset_y });
	attributes.push_back({ 302, skin.keychain.offset_z });
	attributes.push_back({ 306, int_as_attribute_float(skin.keychain.seed) });
}

c_base_entity* get_hud_arms(c_cs_player_pawn* local_pawn) {
	if (!valid_ptr(local_pawn))
		return nullptr;
	auto* entity_system = ttapp::client::interfaces().m_entity_system;
	if (!entity_system)
		return nullptr;

	auto arms_handle = local_pawn->m_hud_model_arms();
	if (!arms_handle.is_valid())
		return nullptr;

	return reinterpret_cast<c_base_entity*>(
		entity_system->get_base_entity(arms_handle.get_entry_index())
		);
}

struct scene_node_scan_info_t {
	c_game_scene_node* child = nullptr;
	c_game_scene_node* next_sibling = nullptr;
	c_entity_instance* owner = nullptr;
	char owner_class_name[64] = {};
	char owner_designer_name[64] = {};
};

struct item_static_log_info_t {
	char model_name[160] = {};
	char view_model[160] = {};
	char weapon_name[96] = {};
	char item_name[96] = {};
};

void safe_read_item_static_log_info(c_econ_item_view* item, item_static_log_info_t& info) {
	if (!valid_ptr(item))
		return;

	__try {
		auto* static_data = item->get_static_data();
		if (!valid_ptr(static_data))
			return;

		safe_copy_text(info.model_name, sizeof(info.model_name), static_data->get_model_name());
		safe_copy_text(info.view_model, sizeof(info.view_model), static_data->get_view_model());
		safe_copy_text(info.weapon_name, sizeof(info.weapon_name), static_data->get_weapon_name());
		safe_copy_text(info.item_name, sizeof(info.item_name), static_data->get_item_name());
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		strcpy_s(info.model_name, sizeof(info.model_name), "?");
		strcpy_s(info.view_model, sizeof(info.view_model), "?");
		strcpy_s(info.weapon_name, sizeof(info.weapon_name), "?");
		strcpy_s(info.item_name, sizeof(info.item_name), "?");
	}
}

bool safe_read_scene_node_scan_info(c_game_scene_node* node, scene_node_scan_info_t& info) {
	if (!valid_ptr(node))
		return false;

	const char* class_name = nullptr;
	const char* designer_name = nullptr;
	__try {
		info.child = node->m_child();
		info.next_sibling = node->m_next_sibling();
		info.owner = node->m_owner();
		if (valid_ptr(info.owner)) {
			auto* binding = info.owner->get_schema_class_binding();
			class_name = binding ? binding->m_binding_name : nullptr;
			designer_name = info.owner->get_designer_name();
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}

	safe_copy_text(info.owner_class_name, sizeof(info.owner_class_name), class_name);
	safe_copy_text(info.owner_designer_name, sizeof(info.owner_designer_name), designer_name);
	return true;
}

void log_stattrak_scene_root(const char* phase, const char* root_name, c_game_scene_node* root, uint16_t def_index, int weapon_handle, int hud_weapon_handle) {
	if (!valid_ptr(root)) {
		stattrak_file_log(
			"scene root missing phase=%s root=%s def=%u weapon_handle=0x%08X hud_handle=0x%08X",
			phase,
			root_name,
			static_cast<unsigned int>(def_index),
			weapon_handle,
			hud_weapon_handle);
		return;
	}

	c_game_scene_node* stack[128] = {};
	c_game_scene_node* visited_nodes[128] = {};
	int stack_size = 0;
	int visited = 0;
	int owners = 0;
	int addons = 0;
	int stattrak_named = 0;
	int owner_weapon_matches = 0;
	int owner_hud_matches = 0;
	int interesting = 0;
	int logged = 0;
	stack[stack_size++] = root;

	while (stack_size > 0 && visited < 128) {
		auto* node = stack[--stack_size];
		if (!valid_ptr(node))
			continue;

		bool already_visited = false;
		for (int i = 0; i < visited; ++i) {
			if (visited_nodes[i] == node) {
				already_visited = true;
				break;
			}
		}
		if (already_visited)
			continue;

		scene_node_scan_info_t info{};
		if (!safe_read_scene_node_scan_info(node, info))
			continue;

		visited_nodes[visited++] = node;

		if (valid_ptr(info.next_sibling) && stack_size < 128)
			stack[stack_size++] = info.next_sibling;
		if (valid_ptr(info.child) && stack_size < 128)
			stack[stack_size++] = info.child;

		if (!valid_ptr(info.owner))
			continue;

		++owners;
		auto* owner_entity = reinterpret_cast<c_base_entity*>(info.owner);
		const int owner_handle = safe_entity_handle(owner_entity);
		const int owner_entity_handle = safe_owner_entity_handle(owner_entity);
		const bool is_addon = safe_contains_ascii_icase(info.owner_designer_name, "addon")
			|| safe_contains_ascii_icase(info.owner_class_name, "addon");
		const bool is_stattrak_named = safe_contains_ascii_icase(info.owner_designer_name, "stattrak")
			|| safe_contains_ascii_icase(info.owner_class_name, "stattrak")
			|| safe_contains_ascii_icase(info.owner_designer_name, "counter")
			|| safe_contains_ascii_icase(info.owner_class_name, "counter");
		const bool owner_weapon_match = owner_handle == weapon_handle || owner_entity_handle == weapon_handle;
		const bool owner_hud_match = hud_weapon_handle != static_cast<int>(INVALID_EHANDLE_INDEX)
			&& (owner_handle == hud_weapon_handle || owner_entity_handle == hud_weapon_handle);
		const bool should_log = is_addon || is_stattrak_named || owner_weapon_match || owner_hud_match;

		if (is_addon)
			++addons;
		if (is_stattrak_named)
			++stattrak_named;
		if (owner_weapon_match)
			++owner_weapon_matches;
		if (owner_hud_match)
			++owner_hud_matches;
		if (should_log)
			++interesting;

		if (should_log && logged < 16) {
			stattrak_file_log(
				"scene node phase=%s root=%s def=%u node=%p owner=%p handle=0x%08X owner_entity=0x%08X class=%s designer=%s addon=%d stattrak_name=%d owner_weapon=%d owner_hud=%d",
				phase,
				root_name,
				static_cast<unsigned int>(def_index),
				node,
				info.owner,
				owner_handle,
				owner_entity_handle,
				info.owner_class_name,
				info.owner_designer_name,
				is_addon ? 1 : 0,
				is_stattrak_named ? 1 : 0,
				owner_weapon_match ? 1 : 0,
				owner_hud_match ? 1 : 0);
			++logged;
		}
	}

	stattrak_file_log(
		"scene summary phase=%s root=%s def=%u nodes=%d owners=%d interesting=%d addons=%d stattrak_named=%d owner_weapon=%d owner_hud=%d logged=%d weapon_handle=0x%08X hud_handle=0x%08X",
		phase,
		root_name,
		static_cast<unsigned int>(def_index),
		visited,
		owners,
		interesting,
		addons,
		stattrak_named,
		owner_weapon_matches,
		owner_hud_matches,
		logged,
		weapon_handle,
		hud_weapon_handle);
}

void log_stattrak_scene_scan(const char* phase, c_cs_player_pawn* local_pawn, c_econ_entity* weapon, c_base_entity* hud_weapon, uint16_t def_index) {
	const int weapon_handle = safe_entity_handle(weapon);
	const int hud_weapon_handle = safe_entity_handle(hud_weapon);
	stattrak_file_log(
		"scene scan phase=%s def=%u weapon=%p hud_weapon=%p weapon_handle=0x%08X hud_handle=0x%08X",
		phase,
		static_cast<unsigned int>(def_index),
		weapon,
		hud_weapon,
		weapon_handle,
		hud_weapon_handle);

	if (valid_ptr(weapon))
		log_stattrak_scene_root(phase, "world_weapon", weapon->m_scene_node(), def_index, weapon_handle, hud_weapon_handle);
	else
		log_stattrak_scene_root(phase, "world_weapon", nullptr, def_index, weapon_handle, hud_weapon_handle);

	if (valid_ptr(hud_weapon))
		log_stattrak_scene_root(phase, "hud_weapon", hud_weapon->m_scene_node(), def_index, weapon_handle, hud_weapon_handle);
	else
		log_stattrak_scene_root(phase, "hud_weapon", nullptr, def_index, weapon_handle, hud_weapon_handle);

	auto* hud_arms = get_hud_arms(local_pawn);
	if (valid_ptr(hud_arms))
		log_stattrak_scene_root(phase, "hud_arms", hud_arms->m_scene_node(), def_index, weapon_handle, hud_weapon_handle);
	else
		log_stattrak_scene_root(phase, "hud_arms", nullptr, def_index, weapon_handle, hud_weapon_handle);
}

int remove_hud_keychain_addons_from(c_game_scene_node* root, int hud_weapon_handle, int* removed_handles, int& removed_handle_count) {
	if (!valid_ptr(root) || hud_weapon_handle == static_cast<int>(INVALID_EHANDLE_INDEX))
		return 0;

	auto remove_fn = keychain_entity_remove_fn();
	if (!remove_fn)
		return 0;

	c_game_scene_node* stack[64] = {};
	int stack_size = 0;
	int visited = 0;
	int removed = 0;
	stack[stack_size++] = root;

	while (stack_size > 0 && visited < 64) {
		auto* node = stack[--stack_size];
		scene_node_scan_info_t info{};
		if (!safe_read_scene_node_scan_info(node, info))
			continue;
		++visited;

		if (valid_ptr(info.next_sibling) && stack_size < 64)
			stack[stack_size++] = info.next_sibling;
		if (valid_ptr(info.child) && stack_size < 64)
			stack[stack_size++] = info.child;

		if (!safe_contains_ascii_icase(info.owner_designer_name, "cs2_hudmodel_addon"))
			continue;

		auto* owner_entity = reinterpret_cast<c_base_entity*>(info.owner);
		if (!valid_ptr(owner_entity))
			continue;

		int addon_owner_handle = static_cast<int>(INVALID_EHANDLE_INDEX);
		__try {
			addon_owner_handle = owner_entity->m_owner_entity().to_int();
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			continue;
		}
		if (addon_owner_handle != hud_weapon_handle)
			continue;

		const int addon_handle = safe_entity_handle(owner_entity);
		bool already_removed = false;
		for (int i = 0; i < removed_handle_count; ++i) {
			if (removed_handles[i] == addon_handle) {
				already_removed = true;
				break;
			}
		}
		if (already_removed)
			continue;

		if (safe_remove_keychain_entity(remove_fn, owner_entity)) {
			if (removed_handle_count < 16)
				removed_handles[removed_handle_count++] = addon_handle;
			++removed;
		}
	}

	return removed;
}

int remove_stattrak_scene_entities_from(c_game_scene_node* root, int* removed_handles, int& removed_handle_count) {
	if (!valid_ptr(root))
		return 0;

	auto remove_fn = keychain_entity_remove_fn();
	if (!remove_fn)
		return 0;

	c_game_scene_node* stack[128] = {};
	c_game_scene_node* visited_nodes[128] = {};
	int stack_size = 0;
	int visited = 0;
	int removed = 0;
	stack[stack_size++] = root;

	while (stack_size > 0 && visited < 128) {
		auto* node = stack[--stack_size];
		if (!valid_ptr(node))
			continue;

		bool already_visited = false;
		for (int i = 0; i < visited; ++i) {
			if (visited_nodes[i] == node) {
				already_visited = true;
				break;
			}
		}
		if (already_visited)
			continue;

		scene_node_scan_info_t info{};
		if (!safe_read_scene_node_scan_info(node, info))
			continue;
		visited_nodes[visited++] = node;

		if (valid_ptr(info.next_sibling) && stack_size < 128)
			stack[stack_size++] = info.next_sibling;
		if (valid_ptr(info.child) && stack_size < 128)
			stack[stack_size++] = info.child;

		if (!valid_ptr(info.owner))
			continue;

		auto* owner_entity = reinterpret_cast<c_base_entity*>(info.owner);
		const int owner_handle = safe_entity_handle(owner_entity);
		const int owner_entity_handle = safe_owner_entity_handle(owner_entity);
		const bool stattrak_named = safe_contains_ascii_icase(info.owner_designer_name, "stattrak")
			|| safe_contains_ascii_icase(info.owner_class_name, "stattrak")
			|| safe_contains_ascii_icase(info.owner_designer_name, "counter")
			|| safe_contains_ascii_icase(info.owner_class_name, "counter");
		const bool orphan_hud_addon = safe_contains_ascii_icase(info.owner_designer_name, "cs2_hudmodel_addon")
			&& owner_entity_handle == static_cast<int>(INVALID_EHANDLE_INDEX);
		if (!stattrak_named && !orphan_hud_addon)
			continue;

		bool already_removed = false;
		for (int i = 0; i < removed_handle_count; ++i) {
			if (removed_handles[i] == owner_handle) {
				already_removed = true;
				break;
			}
		}
		if (already_removed)
			continue;

		if (safe_remove_keychain_entity(remove_fn, owner_entity)) {
			if (removed_handle_count < 32)
				removed_handles[removed_handle_count++] = owner_handle;
			++removed;
		}
	}

	return removed;
}

int remove_stattrak_scene_entities(c_cs_player_pawn* local_pawn, c_econ_entity* weapon, c_base_entity* hud_weapon) {
	int removed_handles[32] = {};
	int removed_handle_count = 0;
	int removed = 0;

	if (valid_ptr(weapon))
		if (auto* scene_node = weapon->m_scene_node())
			removed += remove_stattrak_scene_entities_from(scene_node, removed_handles, removed_handle_count);

	if (valid_ptr(hud_weapon))
		if (auto* hud_node = hud_weapon->m_scene_node())
			removed += remove_stattrak_scene_entities_from(hud_node, removed_handles, removed_handle_count);

	if (auto* hud_arms = get_hud_arms(local_pawn))
		if (auto* arms_node = hud_arms->m_scene_node())
			removed += remove_stattrak_scene_entities_from(arms_node, removed_handles, removed_handle_count);

	return removed;
}

int remove_stale_keychain_hud_addons(c_cs_player_pawn* local_pawn, c_base_entity* hud_weapon) {
	const int hud_weapon_handle = safe_entity_handle(hud_weapon);
	if (hud_weapon_handle == static_cast<int>(INVALID_EHANDLE_INDEX))
		return 0;

	int removed_handles[16] = {};
	int removed_handle_count = 0;
	int removed = 0;
	if (auto* hud_arms = get_hud_arms(local_pawn))
		if (auto* arms_node = hud_arms->m_scene_node())
			removed += remove_hud_keychain_addons_from(arms_node, hud_weapon_handle, removed_handles, removed_handle_count);

	if (valid_ptr(hud_weapon))
		if (auto* hud_node = hud_weapon->m_scene_node())
			removed += remove_hud_keychain_addons_from(hud_node, hud_weapon_handle, removed_handles, removed_handle_count);

	return removed;
}

int remove_stale_keychain_entities(c_base_entity* keep_entity) {
	auto* entity_system = ttapp::client::interfaces().m_entity_system;
	if (!entity_system)
		return 0;

	auto remove_fn = keychain_entity_remove_fn();
	if (!remove_fn)
		return 0;

	int removed = 0;
	for (int i = 0; i <= ENT_ENTRY_MASK; ++i) {
		auto* entity = reinterpret_cast<c_base_entity*>(entity_system->get_base_entity(i));
		if (!valid_ptr(entity) || entity == keep_entity)
			continue;

		const char* class_name = nullptr;
		const char* designer_name = nullptr;
		__try {
			auto* binding = entity->get_schema_class_binding();
			class_name = binding ? binding->m_binding_name : nullptr;
			designer_name = entity->get_designer_name();
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			continue;
		}

		if (!safe_contains_ascii_icase(class_name, "keychain") && !safe_contains_ascii_icase(designer_name, "keychain"))
			continue;

		if (safe_remove_keychain_entity(remove_fn, entity))
			++removed;
	}

	return removed;
}

void rebuild_hud_weapon_graph(c_base_entity* hud_weapon) {
	if (!valid_ptr(hud_weapon))
		return;

	static auto fn = reinterpret_cast<anim_graph_rebuild_fn>(
		ttapp::client::patterns().scan(ttapp::client::modules().m_modules.client_dll.get_name(),
			"40 55 56 48 83 EC 28 4C 89 74 24 58 48 8B F1 80 FA FF 75 04 0F B6 51 18")
		);
	if (!fn)
		return;

	const auto base = reinterpret_cast<std::uintptr_t>(hud_weapon);
	const auto body = read_ptr(base + 0x30);
	if (!valid_ptr(body))
		return;

	const auto controller = body + 0x4E0;
	const auto graph = read_ptr(controller + 0x448);
	if (valid_ptr(graph))
		safe_rebuild_anim_graph(fn, controller);
}

void refresh_hud_model_arms(c_cs_player_pawn* local_pawn) {
	auto* hud_arms = get_hud_arms(local_pawn);
	if (!valid_ptr(hud_arms))
		return;

	static auto fn = reinterpret_cast<hud_model_arms_refresh_fn>(
		ttapp::client::patterns().scan(ttapp::client::modules().m_modules.client_dll.get_name(),
			"48 89 74 24 ? 41 56 48 83 EC 50 48 89 5C 24")
		);
	if (!fn)
		return;

	safe_refresh_hud_arms(fn, hud_arms);
}

} // namespace

c_base_entity* SkinChanger::get_hud_weapon(c_base_entity* weapon, c_cs_player_pawn* local_pawn) {
	auto* entity_system = ttapp::client::interfaces().m_entity_system;
	if (!entity_system)
		return nullptr;

	auto* hud_arms = get_hud_arms(local_pawn);
	if (!valid_ptr(hud_arms))
		return nullptr;

	auto* arms_node = hud_arms->m_scene_node();
	if (!valid_ptr(arms_node))
		return nullptr;

	for (auto* vm = arms_node->m_child(); valid_ptr(vm); vm = vm->m_next_sibling()) {
		auto* vm_owner = vm->m_owner();
		if (!valid_ptr(vm_owner))
			continue;

		auto* vm_entity = reinterpret_cast<c_base_entity*>(vm_owner);
		const char* designer_name = vm_entity->get_designer_name();
		if (!designer_name || strcmp(designer_name, "cs2_hudmodel_weapon") != 0)
			continue;

		auto owner_handle = vm_entity->m_owner_entity();
		if (!owner_handle.is_valid())
			continue;

		if (entity_system->get_base_entity(owner_handle.get_entry_index()) == weapon)
			return vm_entity;
	}
	return nullptr;
}


void SkinChanger::apply_skin(c_econ_entity* weapon, c_econ_item_view* item, int paint_kit_id, const Config::skin_changer_t::weapon_skin_t& skin, c_cs_player_pawn* local_pawn, uint16_t def_index, bool add_stattrak_entity, bool refresh_visuals)
{
	auto* weapon_base = reinterpret_cast<c_cs_weapon_base*>(weapon);
	const bool ui_weapon = valid_ptr(weapon_base) && weapon_base->m_b_ui_weapon();
	const bool trace_stattrak = skin.stattrak_enabled || weapon->m_StatTrak() >= 0 || item->m_entity_quality() == QUALITY_STRANGE || add_stattrak_entity;
	auto log_apply_end = [&]() {
		if (trace_stattrak) {
			stattrak_file_log(
				"apply end weapon=%p item=%p def=%u fallback=%d quality=%d item_id_high=0x%08X item_id_low=0x%08X item_id=%llu",
				weapon,
				item,
				static_cast<unsigned int>(def_index),
				weapon->m_StatTrak(),
				item->m_entity_quality(),
				item->m_item_id_high(),
				item->m_item_id_low(),
				static_cast<unsigned long long>(item->m_item_id()));
		}
	};
	if (trace_stattrak) {
		item_static_log_info_t static_info{};
		safe_read_item_static_log_info(item, static_info);
		stattrak_file_log(
			"apply begin weapon=%p item=%p def=%u paint=%d enabled=%d count=%d request_add=%d refresh=%d ui=%d current_fallback=%d quality=%d item_id_high=0x%08X item_id_low=0x%08X item_id=%llu",
			weapon,
			item,
			static_cast<unsigned int>(def_index),
			paint_kit_id,
			skin.stattrak_enabled ? 1 : 0,
			skin.stattrak_count,
			add_stattrak_entity ? 1 : 0,
			refresh_visuals ? 1 : 0,
			ui_weapon ? 1 : 0,
			weapon->m_StatTrak(),
			item->m_entity_quality(),
			item->m_item_id_high(),
			item->m_item_id_low(),
			static_cast<unsigned long long>(item->m_item_id()));
		stattrak_file_log(
			"item static def=%u weapon_name=%s item_name=%s model=%s view_model=%s",
			static_cast<unsigned int>(def_index),
			static_info.weapon_name,
			static_info.item_name,
			static_info.model_name,
			static_info.view_model);
	}

	const bool plain_default = paint_kit_id == 0
		&& skin.paint_kit == 0
		&& skin.custom_name[0] == '\0'
		&& !has_weapon_extra_cosmetics(skin);
	const bool had_live_keychain = skin.keychain.id > 0 && get_live_keychain(weapon_base);
	if (had_live_keychain) {
		auto* hud_weapon = get_hud_weapon(weapon, local_pawn);
		remove_stale_keychain_hud_addons(local_pawn, hud_weapon);
		remove_stale_keychain_entities(nullptr);
	}

	auto* controller = local_pawn->get_controller();
	uint32_t local_account_id = controller ? (uint32_t)controller->m_steam_id() : 0;

	const uint32_t item_id_low = (skin.keychain.id > 0 || had_live_keychain)
		? fresh_weapon_cache_item_id_low(def_index, skin.keychain.id, skin.keychain.seed)
		: fresh_weapon_cache_item_id_low(def_index, paint_kit_id, skin.seed);
	uint64_t item_id_full = (0xFFFFFFFFull << 32) | item_id_low;

	item->m_item_id_high() = 0xFFFFFFFF;
	item->m_item_id_low() = item_id_low;
	item->m_item_id() = item_id_full;           //m_iItemID
	item->m_account_id() = local_account_id;    //m_iAccountID

	item->m_initialized() = true;            //m_bInitialized
	item->m_bDisallowSOCm() = false;
	item->m_bRestoreCustomMaterialAfterPrecache() = true;

	weapon->m_paint_kit() = paint_kit_id;
	weapon->m_wear() = skin.wear;
	weapon->m_seed() = skin.seed;
	const int stattrak_count = skin.stattrak_count < 0 ? 0 : skin.stattrak_count;
	bool clearing_stattrak = false;
	if (skin.stattrak_enabled) {
		item->m_entity_quality() = QUALITY_STRANGE;
		weapon->m_StatTrak() = stattrak_count;
		item->m_account_id() = local_account_id;
	}
	else {
		clearing_stattrak = weapon->m_StatTrak() >= 0 || item->m_entity_quality() == QUALITY_STRANGE;
		weapon->m_StatTrak() = -1;
		if (clearing_stattrak)
			item->m_entity_quality() = QUALITY_DEFAULT;
	}

	if (plain_default) {
		const uint32_t default_item_id_low = fresh_weapon_cache_item_id_low(def_index, 0, 0);
		item->m_item_id_high() = 0xFFFFFFFF;
		item->m_item_id_low() = default_item_id_low;
		item->m_item_id() = (0xFFFFFFFFull << 32) | default_item_id_low;
		weapon->m_wear() = 0.0f;
		weapon->m_seed() = 0;
		item->m_entity_quality() = QUALITY_DEFAULT;
		item->m_bRestoreCustomMaterialAfterPrecache() = true;
		item->m_custom_name()[0] = '\0';
		item->m_name_description_ptr() = 0;
		econ_item_attribute_manager::remove(item);
		item->invalidate_attribute_cache();
		sync_constructed_paint_kit(item, 0);

		if (!refresh_visuals) {
			log_apply_end();
			return;
		}

		safe_set_mesh_group_mask(weapon, 1);
		if (auto* hud_weapon = get_hud_weapon(weapon, local_pawn))
			safe_set_mesh_group_mask(hud_weapon, 1);
		safe_update_composite(weapon);
		safe_update_skin(weapon);
		safe_update_weapon_data(weapon);
		log_apply_end();
		return;
	}

	std::vector<econ_item_attribute_manager::attribute_value_t> extra_attributes;
	extra_attributes.reserve(Config::skin_changer_t::sticker_slot_count * 7 + 5);
	append_weapon_extra_attributes(extra_attributes, skin);
	econ_item_attribute_manager::remove(item);
	econ_item_attribute_manager::create(
		item,
		paint_kit_id,
		skin.wear,
		skin.seed,
		extra_attributes.data(),
		extra_attributes.size());

	item->invalidate_attribute_cache();
	sync_constructed_paint_kit(item, paint_kit_id);

	const bool can_add_stattrak_entity = skin.stattrak_enabled && add_stattrak_entity && !ui_weapon;
	if (!can_add_stattrak_entity && trace_stattrak) {
		stattrak_file_log(
			"add skipped weapon=%p item=%p def=%u enabled=%d request_add=%d ui=%d fallback=%d quality=%d",
			weapon,
			item,
			static_cast<unsigned int>(def_index),
			skin.stattrak_enabled ? 1 : 0,
			add_stattrak_entity ? 1 : 0,
			ui_weapon ? 1 : 0,
			weapon->m_StatTrak(),
			item->m_entity_quality());
	}

	if (!refresh_visuals) {
		if (trace_stattrak)
			stattrak_file_log("refresh deferred weapon=%p item=%p def=%u", weapon, item, static_cast<unsigned int>(def_index));
		log_apply_end();
		return;
	}

	if (trace_stattrak)
		stattrak_file_log("refresh update_composite begin weapon=%p item=%p def=%u", weapon, item, static_cast<unsigned int>(def_index));
	safe_update_composite(weapon);
	if (trace_stattrak)
		stattrak_file_log("refresh update_composite end weapon=%p item=%p def=%u", weapon, item, static_cast<unsigned int>(def_index));

	if (skin.custom_name[0] != '\0')
	{
		strcpy_s(item->m_custom_name(), 161, skin.custom_name);
		item->m_name_description_ptr() = 0;
		weapon->add_nametag_entity();
	}

	else
	{
		item->m_custom_name()[0] = '\0';
	}

	bool uses_old_model = false;
	c_paint_kit* pk = nullptr;
	if ((pk = find_paint_kit(paint_kit_id)))
		uses_old_model = pk->uses_old_model();

	uint64_t mesh_mask = uses_old_model ? 2 : 1;

	safe_set_mesh_group_mask(weapon, mesh_mask);

	if (auto* hud_weapon = get_hud_weapon(weapon, local_pawn))
		safe_set_mesh_group_mask(hud_weapon, mesh_mask);

	if (trace_stattrak)
		stattrak_file_log("refresh update_skin begin weapon=%p item=%p def=%u", weapon, item, static_cast<unsigned int>(def_index));
	safe_update_skin(weapon);
	if (trace_stattrak)
		stattrak_file_log("refresh update_skin end weapon=%p item=%p def=%u", weapon, item, static_cast<unsigned int>(def_index));

	if (trace_stattrak)
		stattrak_file_log("refresh update_weapon_data begin weapon=%p item=%p def=%u", weapon, item, static_cast<unsigned int>(def_index));
	safe_update_weapon_data(weapon);
	if (trace_stattrak)
		stattrak_file_log("refresh update_weapon_data end weapon=%p item=%p def=%u", weapon, item, static_cast<unsigned int>(def_index));

	if (clearing_stattrak && !ui_weapon) {
		auto* stattrak_hud_weapon = get_hud_weapon(weapon, local_pawn);
		log_stattrak_scene_scan("before_clear", local_pawn, weapon, stattrak_hud_weapon, def_index);
		const int removed = remove_stattrak_scene_entities(local_pawn, weapon, stattrak_hud_weapon);
		stattrak_file_log("clear stattrak entities weapon=%p item=%p def=%u removed=%d", weapon, item, static_cast<unsigned int>(def_index), removed);
		const bool sticker_entities_added = weapon_base->add_sticker_entities();
		stattrak_file_log("clear restore stickers weapon=%p item=%p def=%u result=%d", weapon, item, static_cast<unsigned int>(def_index), sticker_entities_added ? 1 : 0);
		if (skin.custom_name[0] != '\0')
			weapon_base->add_name_tag_entity();
		if (valid_ptr(stattrak_hud_weapon)) {
			rebuild_hud_weapon_graph(stattrak_hud_weapon);
			if (auto* hud_node = stattrak_hud_weapon->m_scene_node())
				hud_node->set_mesh_group_mask(mesh_mask);
		}
		refresh_hud_model_arms(local_pawn);
		log_stattrak_scene_scan("after_clear", local_pawn, weapon, get_hud_weapon(weapon, local_pawn), def_index);
	}

	if (can_add_stattrak_entity) {
		auto* stattrak_hud_weapon = get_hud_weapon(weapon, local_pawn);
		log_stattrak_scene_scan("before_add", local_pawn, weapon, stattrak_hud_weapon, def_index);
		const bool sticker_entities_added = weapon_base->add_sticker_entities();
		stattrak_file_log("add prerequisite stickers weapon=%p item=%p def=%u result=%d", weapon, item, static_cast<unsigned int>(def_index), sticker_entities_added ? 1 : 0);
		stattrak_file_log("add begin phase=post_refresh weapon=%p item=%p def=%u count=%d ui=%d", weapon, item, static_cast<unsigned int>(def_index), stattrak_count, ui_weapon ? 1 : 0);
		const bool added = weapon->add_stattrak_entity();
		stattrak_file_log("add end phase=post_refresh weapon=%p item=%p def=%u result=%d fallback=%d quality=%d", weapon, item, static_cast<unsigned int>(def_index), added ? 1 : 0, weapon->m_StatTrak(), item->m_entity_quality());
		log_stattrak_scene_scan("after_add", local_pawn, weapon, stattrak_hud_weapon, def_index);
		if (added) {
			if (valid_ptr(stattrak_hud_weapon)) {
				rebuild_hud_weapon_graph(stattrak_hud_weapon);
				if (auto* hud_node = stattrak_hud_weapon->m_scene_node())
					hud_node->set_mesh_group_mask(mesh_mask);
			}
			refresh_hud_model_arms(local_pawn);
			log_stattrak_scene_scan("after_hud_refresh", local_pawn, weapon, get_hud_weapon(weapon, local_pawn), def_index);
		}
	}

	if (skin.keychain.id > 0 || had_live_keychain) {
		auto* static_data = item->get_static_data();
		const char* view_model = static_data ? static_data->get_view_model() : nullptr;
		if (view_model && view_model[0] != '\0') {
			if (auto* hud_weapon = get_hud_weapon(weapon, local_pawn)) {
				hud_weapon->set_model(view_model);
				rebuild_hud_weapon_graph(hud_weapon);
				if (auto* hud_node = hud_weapon->m_scene_node())
					hud_node->set_mesh_group_mask(mesh_mask);
			}
			refresh_hud_model_arms(local_pawn);
		}
		if (skin.keychain.id > 0) {
			weapon_base->add_sticker_entities();
			weapon_base->add_name_tag_entity();
			weapon_base->add_keychain_entity(true);

			c_base_entity* current_keychain = nullptr;
			if (get_live_keychain(weapon_base, &current_keychain))
				remove_stale_keychain_entities(current_keychain);
		}
	}

	log_apply_end();
}


void SkinChanger::initialize() {
	if (m_initialized)
		return;

	if (!ttapp::client::item_schema().is_initialized())
		ttapp::client::item_schema().initialize();

	m_initialized = ttapp::client::item_schema().is_initialized();
}

static bool is_weapon_skin_current(c_econ_entity* weapon, c_econ_item_view* item, const Config::skin_changer_t::weapon_skin_t& skin, int paint_kit_id) {
	if (!valid_ptr(weapon) || !valid_ptr(item))
		return false;

	const bool name_matches = skin.custom_name[0] == '\0'
		? item->m_custom_name()[0] == '\0'
		: strcmp(item->m_custom_name(), skin.custom_name) == 0;
	const int stattrak_count = skin.stattrak_count < 0 ? 0 : skin.stattrak_count;
	const bool stattrak_matches = skin.stattrak_enabled
		? item->m_entity_quality() == QUALITY_STRANGE && weapon->m_StatTrak() == stattrak_count
		: weapon->m_StatTrak() == -1;

	return weapon->m_paint_kit() == paint_kit_id
		&& weapon->m_wear() == skin.wear
		&& weapon->m_seed() == skin.seed
		&& item->m_item_id_high() == 0xFFFFFFFF
		&& item->m_initialized()
		&& item->m_bRestoreCustomMaterialAfterPrecache()
		&& name_matches
		&& stattrak_matches;
}

void SkinChanger::refresh_weapon_skin_visuals(c_econ_entity* weapon, c_cs_player_pawn* local_pawn, int paint_kit_id) {
	bool uses_old_model = false;
	if (auto* pk = find_paint_kit(paint_kit_id))
		uses_old_model = pk->uses_old_model();

	const uint64_t mesh_mask = uses_old_model ? 2 : 1;
	safe_set_mesh_group_mask(weapon, mesh_mask);

	if (auto* hud_weapon = get_hud_weapon(weapon, local_pawn))
		safe_set_mesh_group_mask(hud_weapon, mesh_mask);

	safe_update_composite(weapon);
	safe_update_skin(weapon);
	safe_update_weapon_data(weapon);
}

void SkinChanger::process_weapon(c_econ_entity* weapon, c_econ_item_view* item, c_cs_player_pawn* local_pawn, uint32_t weapon_handle, bool force_update, bool& did_update, uint64_t local_steam_id, bool active_weapon_changed) {
	
	const uint64_t original_owner_xuid = weapon->get_original_owner_xuid();
	if (original_owner_xuid != 0 && original_owner_xuid != local_steam_id)
		return;

	uint16_t def_index = item->m_definition_index();
	int config_index = Config::skin_changer_t::get_config_index(def_index);
	if (config_index == 0) {
		m_last_weapon_cosmetic_states.erase(weapon_handle);
		return;
	}

	auto& skin = ttapp::client::config().skin_changer.weapon_skins[config_index];
	const bool has_extra_cosmetics = has_weapon_extra_cosmetics(skin);
	if (skin.paint_kit == 0 && !has_extra_cosmetics) {
		const bool had_cached_state = m_last_weapon_cosmetic_states.find(weapon_handle) != m_last_weapon_cosmetic_states.end();
		if (had_cached_state || has_live_weapon_cosmetics(weapon, item)) {
			apply_skin(weapon, item, 0, skin, local_pawn, def_index, false);
			m_visual_refresh_frames[weapon_handle] = 2;
			c_hud::clear_hud_weapon_icon_for(weapon);
			did_update = true;
		}
		else if (auto pending_refresh = m_visual_refresh_frames.find(weapon_handle); pending_refresh != m_visual_refresh_frames.end()) {
			refresh_weapon_skin_visuals(weapon, local_pawn, 0);
			if (--pending_refresh->second <= 0)
				m_visual_refresh_frames.erase(pending_refresh);
		}
		m_last_weapon_cosmetic_states.erase(weapon_handle);
		return;
	}

	int paint_kit_id = 0;
	if (skin.paint_kit > 0)
		paint_kit_id = ttapp::client::item_schema().get_paint_kit_id_for_item(def_index, skin.paint_kit);
	if (skin.paint_kit > 0 && paint_kit_id == 0 && !has_extra_cosmetics) {
		m_last_weapon_cosmetic_states.erase(weapon_handle);
		return;
	}

	weapon_cosmetic_state_t cosmetic_state{};
	cosmetic_state.custom_colors_enabled = skin.custom_colors_enabled;
	memcpy(cosmetic_state.custom_colors, skin.custom_colors, sizeof(cosmetic_state.custom_colors));
	for (int slot = 0; slot < Config::skin_changer_t::sticker_slot_count; ++slot)
		cosmetic_state.stickers[slot] = skin.stickers[slot];
	cosmetic_state.keychain = skin.keychain;
	cosmetic_state.stattrak_enabled = skin.stattrak_enabled;
	cosmetic_state.stattrak_count = skin.stattrak_count < 0 ? 0 : skin.stattrak_count;
	auto last_cosmetic_state = m_last_weapon_cosmetic_states.find(weapon_handle);
	const bool last_had_stattrak = last_cosmetic_state != m_last_weapon_cosmetic_states.end()
		&& last_cosmetic_state->second.stattrak_enabled;
	const bool active_weapon = weapon_handle == m_last_active_weapon_handle;
	auto* weapon_base = reinterpret_cast<c_cs_weapon_base*>(weapon);
	const bool ui_weapon = valid_ptr(weapon_base) && weapon_base->m_b_ui_weapon();
	bool stattrak_entity_added = false;
	if (last_cosmetic_state != m_last_weapon_cosmetic_states.end())
		stattrak_entity_added = last_cosmetic_state->second.stattrak_entity_added;
	const bool stattrak_count_changed = last_cosmetic_state != m_last_weapon_cosmetic_states.end()
		&& last_cosmetic_state->second.stattrak_count != cosmetic_state.stattrak_count;
	const bool defer_active_switch_stattrak_add = skin.stattrak_enabled && active_weapon && active_weapon_changed && !stattrak_entity_added && !ui_weapon;
	const bool should_add_stattrak_entity = skin.stattrak_enabled && active_weapon && (!stattrak_entity_added || stattrak_count_changed) && !ui_weapon && !defer_active_switch_stattrak_add;
	cosmetic_state.stattrak_entity_added = skin.stattrak_enabled && (stattrak_entity_added || (active_weapon && !ui_weapon));

	bool cosmetic_state_matches = false;
	if (last_cosmetic_state != m_last_weapon_cosmetic_states.end()) {
		const auto& last = last_cosmetic_state->second;
		const bool colors_match = last.custom_colors_enabled == cosmetic_state.custom_colors_enabled
			&& (!cosmetic_state.custom_colors_enabled
				|| memcmp(last.custom_colors, cosmetic_state.custom_colors, sizeof(cosmetic_state.custom_colors)) == 0);
		cosmetic_state_matches = colors_match
			&& sticker_slots_match(last.stickers, cosmetic_state.stickers)
			&& keychains_match(last.keychain, cosmetic_state.keychain)
			&& last.stattrak_enabled == cosmetic_state.stattrak_enabled
			&& last.stattrak_count == cosmetic_state.stattrak_count
			&& last.stattrak_entity_added == cosmetic_state.stattrak_entity_added;
	}

	const bool skin_current = is_weapon_skin_current(weapon, item, skin, paint_kit_id);
	const bool trace_stattrak = skin.stattrak_enabled || last_had_stattrak || weapon->m_StatTrak() >= 0 || item->m_entity_quality() == QUALITY_STRANGE;
	if (trace_stattrak) {
		stattrak_file_log(
			"decision handle=0x%08X active_handle=0x%08X weapon=%p item=%p def=%u config=%d paint=%d active=%d active_changed=%d ui=%d enabled=%d count=%d count_changed=%d fallback=%d quality=%d last_added=%d desired_added=%d should_add=%d defer_add=%d skin_current=%d cosmetic_match=%d item_id_high=0x%08X item_id_low=0x%08X",
			weapon_handle,
			m_last_active_weapon_handle,
			weapon,
			item,
			static_cast<unsigned int>(def_index),
			config_index,
			paint_kit_id,
			active_weapon ? 1 : 0,
			active_weapon_changed ? 1 : 0,
			ui_weapon ? 1 : 0,
			skin.stattrak_enabled ? 1 : 0,
			cosmetic_state.stattrak_count,
			stattrak_count_changed ? 1 : 0,
			weapon->m_StatTrak(),
			item->m_entity_quality(),
			stattrak_entity_added ? 1 : 0,
			cosmetic_state.stattrak_entity_added ? 1 : 0,
			should_add_stattrak_entity ? 1 : 0,
			defer_active_switch_stattrak_add ? 1 : 0,
			skin_current ? 1 : 0,
			cosmetic_state_matches ? 1 : 0,
			item->m_item_id_high(),
			item->m_item_id_low());
	}

	if (skin_current && cosmetic_state_matches) {
		auto pending_refresh = m_visual_refresh_frames.find(weapon_handle);
		if (pending_refresh != m_visual_refresh_frames.end()) {
			refresh_weapon_skin_visuals(weapon, local_pawn, paint_kit_id);
			if (--pending_refresh->second <= 0)
				m_visual_refresh_frames.erase(pending_refresh);
		}
		return;
	}

	if (defer_active_switch_stattrak_add) {
		if (trace_stattrak)
			stattrak_file_log("process deferred_active_switch_stattrak handle=0x%08X weapon=%p item=%p def=%u", weapon_handle, weapon, item, static_cast<unsigned int>(def_index));
		return;
	}

	const bool defer_inactive_stattrak_refresh = skin.stattrak_enabled && !active_weapon && !stattrak_entity_added;
	apply_skin(weapon, item, paint_kit_id, skin, local_pawn, def_index, should_add_stattrak_entity, !defer_inactive_stattrak_refresh);
	if (trace_stattrak)
		stattrak_file_log("process post_apply handle=0x%08X weapon=%p item=%p def=%u", weapon_handle, weapon, item, static_cast<unsigned int>(def_index));
	m_last_weapon_cosmetic_states[weapon_handle] = cosmetic_state;
	if (defer_inactive_stattrak_refresh) {
		if (trace_stattrak)
			stattrak_file_log("process deferred_inactive_stattrak handle=0x%08X weapon=%p item=%p def=%u", weapon_handle, weapon, item, static_cast<unsigned int>(def_index));
		return;
	}
	m_visual_refresh_frames[weapon_handle] = 2;
	c_hud::clear_hud_weapon_icon_for(weapon);
	if (trace_stattrak)
		stattrak_file_log("process post_hud_clear handle=0x%08X weapon=%p item=%p def=%u", weapon_handle, weapon, item, static_cast<unsigned int>(def_index));
	did_update = true;
}

static bool has_skin_engine_reset(c_econ_entity* weapon, c_econ_item_view* item, uint64_t local_steam_id) {
	if (!valid_ptr(weapon) || !valid_ptr(item))
		return false;

	const uint64_t original_owner_xuid = weapon->get_original_owner_xuid();
	if (original_owner_xuid != 0 && original_owner_xuid != local_steam_id)
		return false;

	const uint16_t def_index = item->m_definition_index();
	const bool is_knife = (def_index == WEAPON_KNIFE || def_index == WEAPON_KNIFE_T || (def_index >= 500 && def_index <= 526));
	if (is_knife)
		return false;

	const int config_index = Config::skin_changer_t::get_config_index(def_index);
	if (config_index == 0)
		return false;

	const auto& skin = ttapp::client::config().skin_changer.weapon_skins[config_index];
	const bool has_extra_cosmetics = has_weapon_extra_cosmetics(skin);
	if (skin.paint_kit == 0 && !has_extra_cosmetics)
		return false;

	int paint_kit_id = 0;
	if (skin.paint_kit > 0)
		paint_kit_id = ttapp::client::item_schema().get_paint_kit_id_for_item(def_index, skin.paint_kit);
	if (skin.paint_kit > 0 && paint_kit_id == 0 && !has_extra_cosmetics)
		return false;

	return !is_weapon_skin_current(weapon, item, skin, paint_kit_id);
}

void SkinChanger::process_knife(c_econ_entity* weapon, c_econ_item_view* item, c_cs_player_pawn* local_pawn, uint32_t weapon_handle, bool force_update, bool& did_update, uint64_t local_steam_id) {
	const uint64_t original_owner_xuid = weapon->get_original_owner_xuid();
	if (original_owner_xuid != 0 && original_owner_xuid != local_steam_id)
		return;

	auto& knife_cfg = ttapp::client::config().knife_changer;
	auto& schema = ttapp::client::item_schema();
	if (!schema.is_initialized()
		|| knife_cfg.m_knife < 0
		|| knife_cfg.m_knife >= (int)schema.knives.size())
		return;

	const uint16_t def_index = item->m_definition_index();
	const auto& selected = schema.knives[knife_cfg.m_knife];
	const uint16_t selected_knife = selected.definition_index;
	if (selected_knife == 0)
		return;

	int paint_kit_id = schema.get_paint_kit_id_for_item(selected_knife, knife_cfg.m_paint_kit);
	const bool knife_stattrak_enabled = knife_cfg.m_stattrak_enabled;
	const int knife_stattrak_count = knife_cfg.m_stattrak_count < 0 ? 0 : knife_cfg.m_stattrak_count;
	const bool plain_default_knife = paint_kit_id == 0
		&& !knife_stattrak_enabled
		&& knife_cfg.m_custom_name[0] == '\0'
		&& !knife_cfg.m_custom_colors_enabled;
	const float desired_knife_wear = plain_default_knife ? 0.0f : knife_cfg.m_wear;
	const int desired_knife_seed = plain_default_knife ? 0 : knife_cfg.m_seed;
	const int knife_quality = knife_stattrak_enabled ? QUALITY_STRANGE : QUALITY_UNUSUAL;
	auto* controller = local_pawn->get_controller();
	uint32_t local_account_id = controller ? static_cast<uint32_t>(controller->m_steam_id()) : 0;
	const uint32_t item_id_high = 0xFFFFFFFF;
	const uint32_t item_id_low = fresh_weapon_cache_item_id_low(selected_knife, paint_kit_id, desired_knife_seed);
	const uint64_t item_id_full = (0xFFFFFFFFull << 32) | item_id_low;
	const bool same_knife_weapon = m_last_knife_weapon_handle == weapon_handle && m_last_knife_weapon == weapon;
	const bool knife_stattrak_entity_added = same_knife_weapon && m_last_knife_stattrak_entity_added;
	const bool knife_stattrak_count_changed = same_knife_weapon && m_last_knife_stattrak_count != knife_stattrak_count;
	const bool active_knife = weapon_handle == m_last_active_weapon_handle;
	auto* weapon_base = reinterpret_cast<c_cs_weapon_base*>(weapon);
	const bool ui_weapon = valid_ptr(weapon_base) && weapon_base->m_b_ui_weapon();
	const bool should_add_knife_stattrak_entity = knife_stattrak_enabled && active_knife && (!knife_stattrak_entity_added || knife_stattrak_count_changed) && !ui_weapon;
	const bool expected_knife_stattrak_entity_added = knife_stattrak_enabled && (knife_stattrak_entity_added || (active_knife && !ui_weapon));
	const bool clearing_knife_stattrak = !knife_stattrak_enabled && (weapon->m_StatTrak() >= 0 || item->m_entity_quality() == QUALITY_STRANGE || knife_stattrak_entity_added);
	const bool trace_knife_stattrak = knife_stattrak_enabled || clearing_knife_stattrak || knife_stattrak_entity_added;
	if (trace_knife_stattrak) {
		stattrak_file_log(
			"knife decision handle=0x%08X active_handle=0x%08X weapon=%p item=%p live_def=%u selected_def=%u paint=%d active=%d ui=%d enabled=%d count=%d count_changed=%d fallback=%d quality=%d last_added=%d desired_added=%d should_add=%d clearing=%d item_id_high=0x%08X item_id_low=0x%08X",
			weapon_handle,
			m_last_active_weapon_handle,
			weapon,
			item,
			static_cast<unsigned int>(def_index),
			static_cast<unsigned int>(selected_knife),
			paint_kit_id,
			active_knife ? 1 : 0,
			ui_weapon ? 1 : 0,
			knife_stattrak_enabled ? 1 : 0,
			knife_stattrak_count,
			knife_stattrak_count_changed ? 1 : 0,
			weapon->m_StatTrak(),
			item->m_entity_quality(),
			knife_stattrak_entity_added ? 1 : 0,
			expected_knife_stattrak_entity_added ? 1 : 0,
			should_add_knife_stattrak_entity ? 1 : 0,
			clearing_knife_stattrak ? 1 : 0,
			item->m_item_id_high(),
			item->m_item_id_low());
	}
	color4_state_t knife_color_state{};
	knife_color_state.enabled = knife_cfg.m_custom_colors_enabled;
	memcpy(knife_color_state.colors, knife_cfg.m_custom_colors, sizeof(knife_color_state.colors));

	bool config_changed = (m_last_knife != selected_knife) ||
		(m_last_knife_paint_kit_id != paint_kit_id) ||
		(m_last_knife_wear != knife_cfg.m_wear) ||
		(m_last_knife_seed != knife_cfg.m_seed) ||
		(m_last_knife_stattrak_enabled != knife_stattrak_enabled) ||
		(m_last_knife_stattrak_count != knife_stattrak_count) ||
		(m_last_knife_stattrak_entity_added != expected_knife_stattrak_entity_added) ||
		(m_last_knife_color_state.enabled != knife_color_state.enabled) ||
		(knife_color_state.enabled && memcmp(m_last_knife_color_state.colors, knife_color_state.colors, sizeof(knife_color_state.colors)) != 0);
	const char* model_path = selected.model_path;
	auto* hud_weapon = get_hud_weapon(weapon, local_pawn);
	bool uses_old_model = false;
	if (paint_kit_id > 0)
		if (auto* pk = find_paint_kit(paint_kit_id))
			uses_old_model = pk->uses_old_model();
	const uint64_t mesh_mask = uses_old_model ? 1 : 2;
	auto refresh_knife_visuals = [&]() {
		safe_update_subclass(weapon, selected_knife);
		safe_set_mesh_group_mask(weapon, mesh_mask);
		if (hud_weapon)
			safe_set_mesh_group_mask(hud_weapon, mesh_mask);
		safe_update_composite(weapon);
		safe_update_skin(weapon);
		safe_update_weapon_data(weapon);
	};

	char subclass_buf[16];
	sprintf_s(subclass_buf, "%u", selected_knife);
	const uint32_t selected_subclass = string_token_hash(subclass_buf);
	const bool name_matches = knife_cfg.m_custom_name[0] == '\0'
		? item->m_custom_name()[0] == '\0'
		: strcmp(item->m_custom_name(), knife_cfg.m_custom_name) == 0;
	const bool live_state_matches = def_index == selected_knife
		&& item->m_entity_quality() == knife_quality
		&& item->m_item_id_high() == item_id_high
		&& item->m_initialized()
		&& item->m_bRestoreCustomMaterialAfterPrecache()
		&& weapon->m_paint_kit() == paint_kit_id
		&& weapon->m_wear() == desired_knife_wear
		&& weapon->m_seed() == desired_knife_seed
		&& weapon->m_StatTrak() == (knife_stattrak_enabled ? knife_stattrak_count : -1)
		&& weapon->m_nSubclassID() == selected_subclass
		&& name_matches;
	const bool applied_state_matches = !config_changed
		&& m_last_knife_weapon == weapon
		&& m_last_knife_item == item
		&& m_last_knife_pawn == local_pawn
		&& m_last_knife_weapon_handle == weapon_handle
		&& m_last_knife_hud_weapon == hud_weapon
		&& m_last_knife_model_path == model_path;

	if (live_state_matches && (!force_update || applied_state_matches)) {
		if (auto pending_refresh = m_visual_refresh_frames.find(weapon_handle); pending_refresh != m_visual_refresh_frames.end()) {
			refresh_knife_visuals();
			if (--pending_refresh->second <= 0)
				m_visual_refresh_frames.erase(pending_refresh);
		}
		return;
	}

	item->m_definition_index() = selected_knife;
	item->m_entity_quality() = knife_quality;
	item->m_item_id_high() = item_id_high;
	item->m_item_id_low() = item_id_low;
	item->m_item_id() = item_id_full;
	item->m_account_id() = local_account_id;
	item->m_initialized() = true;            //m_bInitialized
	item->m_bDisallowSOCm() = false;//m_bRestoreCustomMaterialAfterPrecache
	item->m_bRestoreCustomMaterialAfterPrecache() = true;

	if (model_path)
		weapon->set_model(model_path);

	weapon->m_paint_kit() = paint_kit_id;
	weapon->m_wear() = desired_knife_wear;
	weapon->m_seed() = desired_knife_seed;
	weapon->m_StatTrak() = knife_stattrak_enabled ? knife_stattrak_count : -1;

	std::vector<econ_item_attribute_manager::attribute_value_t> knife_extra_attributes;
	if (knife_stattrak_enabled) {
		knife_extra_attributes.push_back({ 80, int_as_attribute_float(knife_stattrak_count) });
		knife_extra_attributes.push_back({ 81, int_as_attribute_float(0) });
	}

	econ_item_attribute_manager::remove(item);
	if (paint_kit_id > 0 || !knife_extra_attributes.empty()) {
		econ_item_attribute_manager::create(
			item,
			paint_kit_id,
			knife_cfg.m_wear,
			knife_cfg.m_seed,
			knife_extra_attributes.data(),
			knife_extra_attributes.size());
	}

	item->invalidate_attribute_cache();
	sync_constructed_paint_kit(item, paint_kit_id);

	safe_update_subclass(weapon, selected_knife);
	safe_set_mesh_group_mask(weapon, mesh_mask);
	if (hud_weapon)
		safe_set_mesh_group_mask(hud_weapon, mesh_mask);
	safe_update_composite(weapon);

	if (knife_cfg.m_custom_name[0] != '\0')
	{
		strcpy_s(item->m_custom_name(), 161, knife_cfg.m_custom_name);
		item->m_name_description_ptr() = 0;
		weapon->add_nametag_entity();
	}

	else
	{
		item->m_custom_name()[0] = '\0'; 
	}

	safe_update_skin(weapon);
	safe_update_weapon_data(weapon);
	if (hud_weapon) {
		if (model_path) {
			hud_weapon->set_model(model_path);
			rebuild_hud_weapon_graph(hud_weapon);
			refresh_hud_model_arms(local_pawn);
		}
		safe_set_mesh_group_mask(hud_weapon, mesh_mask);
	}

	bool knife_stattrak_entity_added_after_apply = knife_stattrak_entity_added;
	if (clearing_knife_stattrak && !ui_weapon) {
		auto* stattrak_hud_weapon = get_hud_weapon(weapon, local_pawn);
		log_stattrak_scene_scan("knife_before_clear", local_pawn, weapon, stattrak_hud_weapon, selected_knife);
		const int removed = remove_stattrak_scene_entities(local_pawn, weapon, stattrak_hud_weapon);
		stattrak_file_log("knife clear stattrak entities weapon=%p item=%p def=%u removed=%d", weapon, item, static_cast<unsigned int>(selected_knife), removed);
		if (valid_ptr(stattrak_hud_weapon)) {
			rebuild_hud_weapon_graph(stattrak_hud_weapon);
			if (auto* hud_node = stattrak_hud_weapon->m_scene_node())
				hud_node->set_mesh_group_mask(mesh_mask);
		}
		refresh_hud_model_arms(local_pawn);
		log_stattrak_scene_scan("knife_after_clear", local_pawn, weapon, get_hud_weapon(weapon, local_pawn), selected_knife);
		knife_stattrak_entity_added_after_apply = false;
	}

	if (should_add_knife_stattrak_entity) {
		auto* stattrak_hud_weapon = get_hud_weapon(weapon, local_pawn);
		log_stattrak_scene_scan("knife_before_add", local_pawn, weapon, stattrak_hud_weapon, selected_knife);
		const bool sticker_entities_added = weapon_base->add_sticker_entities();
		stattrak_file_log("knife add prerequisite stickers weapon=%p item=%p def=%u result=%d", weapon, item, static_cast<unsigned int>(selected_knife), sticker_entities_added ? 1 : 0);
		stattrak_file_log("knife add begin phase=post_refresh weapon=%p item=%p def=%u count=%d ui=%d", weapon, item, static_cast<unsigned int>(selected_knife), knife_stattrak_count, ui_weapon ? 1 : 0);
		const bool added = weapon->add_stattrak_entity();
		stattrak_file_log("knife add end phase=post_refresh weapon=%p item=%p def=%u result=%d fallback=%d quality=%d", weapon, item, static_cast<unsigned int>(selected_knife), added ? 1 : 0, weapon->m_StatTrak(), item->m_entity_quality());
		log_stattrak_scene_scan("knife_after_add", local_pawn, weapon, stattrak_hud_weapon, selected_knife);
		if (added) {
			knife_stattrak_entity_added_after_apply = true;
			if (valid_ptr(stattrak_hud_weapon)) {
				rebuild_hud_weapon_graph(stattrak_hud_weapon);
				if (auto* hud_node = stattrak_hud_weapon->m_scene_node())
					hud_node->set_mesh_group_mask(mesh_mask);
			}
			refresh_hud_model_arms(local_pawn);
			log_stattrak_scene_scan("knife_after_hud_refresh", local_pawn, weapon, get_hud_weapon(weapon, local_pawn), selected_knife);
		}
	}



	m_last_knife = selected_knife;
	m_last_knife_paint_kit_id = paint_kit_id;
	m_last_knife_wear = knife_cfg.m_wear;
	m_last_knife_seed = knife_cfg.m_seed;
	m_last_knife_stattrak_enabled = knife_stattrak_enabled;
	m_last_knife_stattrak_count = knife_stattrak_count;
	m_last_knife_stattrak_entity_added = knife_stattrak_enabled && knife_stattrak_entity_added_after_apply;
	m_last_knife_color_state = knife_color_state;
	m_last_knife_weapon_handle = weapon_handle;
	m_last_knife_weapon = weapon;
	m_last_knife_item = item;
	m_last_knife_pawn = local_pawn;
	m_last_knife_hud_weapon = hud_weapon;
	m_last_knife_model_path = model_path;
	if (plain_default_knife)
		m_visual_refresh_frames[weapon_handle] = 2;
	else
		m_visual_refresh_frames.erase(weapon_handle);
	c_hud::clear_hud_weapon_icon_for(weapon);
	did_update = true;
}

void SkinChanger::run(int stage) {
	
	const bool skin_enabled = ttapp::client::config().skin_changer.m_enabled;
	const bool knife_enabled = ttapp::client::config().knife_changer.m_enabled;

	if ((!skin_enabled && !knife_enabled) || stage != 7 || !ttapp::client::frame().m_local_pawn)
		return;

	
	auto* local_pawn = reinterpret_cast<c_cs_player_pawn*>(ttapp::client::frame().m_local_pawn);
	if (!valid_ptr(local_pawn) || local_pawn->m_health() <= 0)
		return;


	auto* controller = local_pawn->get_controller();
	if (!controller) return;

	const float current_spawn_time = local_pawn->m_last_spawn_time_index();
	const int   current_team = local_pawn->m_team_num();
	uint64_t    local_steam_id = controller->m_steam_id();

	
	const bool team_changed = (current_team != m_last_team) && m_last_team != 0;
	const bool spawn_changed = (current_spawn_time != m_last_spawn_time) && m_last_spawn_time != 0.0f;

	
	static auto last_cfg_skin = ttapp::client::config().skin_changer;
	static auto last_cfg_knife = ttapp::client::config().knife_changer;

	
	bool config_changed = memcmp(&last_cfg_skin, &ttapp::client::config().skin_changer, sizeof(last_cfg_skin)) != 0 ||
		memcmp(&last_cfg_knife, &ttapp::client::config().knife_changer, sizeof(last_cfg_knife)) != 0;

	std::vector<uint32_t> current_weapon_indices;
	auto* weapon_service = local_pawn->m_weapon_services();
	auto* entity_system = ttapp::client::interfaces().m_entity_system;
	if (!entity_system)
		return;
	uint32_t current_active_weapon_handle = 0;
	bool engine_reset = false;
	if (valid_ptr(weapon_service)) {
		if (weapon_service->m_active_weapon().is_valid())
			current_active_weapon_handle = weapon_service->m_active_weapon().to_int();

		auto& my_weapons = weapon_service->my_weapons();
		for (unsigned int i = 0; i < my_weapons.m_size; i++) {
			current_weapon_indices.push_back(my_weapons.m_elements[i].to_int());

			if (!skin_enabled || engine_reset)
				continue;

			auto* weapon = reinterpret_cast<c_econ_entity*>(
				entity_system->get_base_entity(my_weapons.m_elements[i].get_entry_index())
				);
			if (!valid_ptr(weapon))
				continue;

			auto* item = weapon->m_attribute_manager()->m_item();
			if (has_skin_engine_reset(weapon, item, local_steam_id))
				engine_reset = true;
		}
	}

	
	bool weapon_changed = (current_weapon_indices != m_last_weapon_indices);
	bool active_weapon_changed = current_active_weapon_handle != m_last_active_weapon_handle;

	
	m_last_weapon_indices = current_weapon_indices;
	m_last_active_weapon_handle = current_active_weapon_handle;

	if (team_changed || spawn_changed || should_update || weapon_changed || active_weapon_changed || config_changed || engine_reset) {
		m_update_frames = 6; 
		should_update = false;

		last_cfg_skin = ttapp::client::config().skin_changer;
		last_cfg_knife = ttapp::client::config().knife_changer;
	}
	auto sync_pawn_state = [&]() {
		m_last_spawn_time = current_spawn_time;
		m_last_team = current_team;
		};

	
	if (m_update_frames <= 0) {
		sync_pawn_state();
		return;
	}

	
	if (!valid_ptr(weapon_service)) {
		sync_pawn_state();
		return;
	}

	auto& my_weapons = weapon_service->my_weapons();
	std::vector<std::pair<c_econ_item_view*, uint32_t>> restore_list;
	bool did_update = false;

	for (unsigned int i = 0; i < my_weapons.m_size; i++) {
		auto* weapon = reinterpret_cast<c_econ_entity*>(
			entity_system->get_base_entity(my_weapons.m_elements[i].get_entry_index())
			);
		if (!weapon) continue;

		auto* item = weapon->m_attribute_manager()->m_item();
		if (!valid_ptr(item)) continue;

		uint32_t original_id = item->m_item_id_high();
	const uint16_t def_index = item->m_definition_index();
	const bool is_knife = (def_index == WEAPON_KNIFE || def_index == WEAPON_KNIFE_T || (def_index >= 500 && def_index <= 526));

	if (is_knife && knife_enabled)
		process_knife(weapon, item, local_pawn, my_weapons.m_elements[i].to_int(), true, did_update, local_steam_id);
		else if (!is_knife && skin_enabled)
			process_weapon(weapon, item, local_pawn, my_weapons.m_elements[i].to_int(), true, did_update, local_steam_id, active_weapon_changed);
		
	}

	if (did_update) {
		stattrak_file_log("run regenerate_skins begin");
		c_hud::regenerate_skins();
		stattrak_file_log("run regenerate_skins end");
	}
	
	
	sync_pawn_state();
	m_update_frames--;
}
