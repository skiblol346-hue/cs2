#include "runtime/Runtime.hpp"
#include "game/items/ItemSchema.hpp"
#include "game/InterfaceRegistry.hpp"
#include "game/interfaces/i_localize.hpp"
#include "game/entities/c_cs_player_pawn.hpp"
#include <algorithm>
#include <cctype>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <unordered_map>
#include <windows.h>

namespace {

struct vpk_entry_t {
	std::uint16_t archive_index = 0;
	std::uint16_t preload_bytes = 0;
	std::uint32_t offset = 0;
	std::uint32_t length = 0;
	std::size_t preload_offset = 0;
};

bool read_file_bytes(const std::filesystem::path& path, std::vector<char>& out) {
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file)
		return false;

	const std::streamsize size = file.tellg();
	if (size <= 0)
		return false;

	out.resize(static_cast<std::size_t>(size));
	file.seekg(0, std::ios::beg);
	return file.read(out.data(), size).good();
}

bool read_range(const std::filesystem::path& path, std::uint32_t offset, std::uint32_t length, std::vector<char>& out) {
	std::ifstream file(path, std::ios::binary);
	if (!file)
		return false;

	std::vector<char> bytes(length);
	file.seekg(offset, std::ios::beg);
	if (!file.read(bytes.data(), length).good())
		return false;

	out.insert(out.end(), bytes.begin(), bytes.end());
	return true;
}

template <typename T>
bool read_le(const std::vector<char>& data, std::size_t& pos, T& out) {
	if (pos + sizeof(T) > data.size())
		return false;

	std::memcpy(&out, data.data() + pos, sizeof(T));
	pos += sizeof(T);
	return true;
}

bool read_c_string(const std::vector<char>& data, std::size_t& pos, std::string& out) {
	if (pos >= data.size())
		return false;

	const std::size_t start = pos;
	while (pos < data.size() && data[pos] != '\0')
		++pos;

	if (pos >= data.size())
		return false;

	out.assign(data.data() + start, pos - start);
	++pos;
	return true;
}

std::filesystem::path find_pak01_dir() {
	char exe_path[MAX_PATH]{};
	if (!GetModuleFileNameA(nullptr, exe_path, MAX_PATH))
		return {};

	std::filesystem::path dir = std::filesystem::path(exe_path).parent_path();
	for (int i = 0; i < 8 && !dir.empty(); ++i) {
		const std::filesystem::path direct = dir / "pak01_dir.vpk";
		if (std::filesystem::exists(direct))
			return direct;

		const std::filesystem::path csgo = dir / "csgo" / "pak01_dir.vpk";
		if (std::filesystem::exists(csgo))
			return csgo;

		const std::filesystem::path game_csgo = dir / "game" / "csgo" / "pak01_dir.vpk";
		if (std::filesystem::exists(game_csgo))
			return game_csgo;

		dir = dir.parent_path();
	}

	return {};
}

std::string make_vpk_path(const std::string& path, const std::string& name, const std::string& extension) {
	if (path.empty() || path == " ")
		return name + "." + extension;
	return path + "/" + name + "." + extension;
}

bool read_items_game(std::string& out) {
	const std::filesystem::path dir_path = find_pak01_dir();
	if (dir_path.empty())
		return false;

	std::vector<char> dir_data;
	if (!read_file_bytes(dir_path, dir_data))
		return false;

	std::size_t pos = 0;
	std::uint32_t signature = 0, version = 0, tree_size = 0;
	if (!read_le(dir_data, pos, signature) || !read_le(dir_data, pos, version) || !read_le(dir_data, pos, tree_size))
		return false;
	(void)signature;

	if (version == 2)
		pos += 16;

	if (pos >= dir_data.size())
		return false;

	const std::size_t tree_start = pos;
	const std::size_t file_data_start = tree_start + tree_size;
	vpk_entry_t found{};
	bool has_found = false;

	while (pos < dir_data.size() && !has_found) {
		std::string extension;
		if (!read_c_string(dir_data, pos, extension))
			return false;
		if (extension.empty())
			break;

		while (pos < dir_data.size() && !has_found) {
			std::string path;
			if (!read_c_string(dir_data, pos, path))
				return false;
			if (path.empty())
				break;

			while (pos < dir_data.size()) {
				std::string name;
				if (!read_c_string(dir_data, pos, name))
					return false;
				if (name.empty())
					break;

				std::uint32_t crc = 0;
				std::uint16_t terminator = 0;
				vpk_entry_t entry{};
				if (!read_le(dir_data, pos, crc) ||
					!read_le(dir_data, pos, entry.preload_bytes) ||
					!read_le(dir_data, pos, entry.archive_index) ||
					!read_le(dir_data, pos, entry.offset) ||
					!read_le(dir_data, pos, entry.length) ||
					!read_le(dir_data, pos, terminator))
					return false;
				(void)crc;
				(void)terminator;

				entry.preload_offset = pos;
				pos += entry.preload_bytes;

				if (make_vpk_path(path, name, extension) == "scripts/items/items_game.txt") {
					found = entry;
					has_found = true;
					break;
				}
			}
		}
	}

	if (!has_found)
		return false;

	std::vector<char> file_data;
	if (found.preload_bytes > 0) {
		if (found.preload_offset + found.preload_bytes > dir_data.size())
			return false;
		file_data.insert(file_data.end(), dir_data.begin() + found.preload_offset, dir_data.begin() + found.preload_offset + found.preload_bytes);
	}

	if (found.length > 0) {
		if (found.archive_index == 0x7fff) {
			if (file_data_start + found.offset + found.length > dir_data.size())
				return false;
			file_data.insert(file_data.end(), dir_data.begin() + file_data_start + found.offset, dir_data.begin() + file_data_start + found.offset + found.length);
		}
		else {
			char archive_name[32]{};
			std::snprintf(archive_name, sizeof(archive_name), "pak01_%03u.vpk", found.archive_index);
			if (!read_range(dir_path.parent_path() / archive_name, found.offset, found.length, file_data))
				return false;
		}
	}

	out.assign(file_data.begin(), file_data.end());
	return !out.empty();
}

void skip_ws(const std::string& text, std::size_t& pos, std::size_t end) {
	while (pos < end) {
		if (std::isspace(static_cast<unsigned char>(text[pos]))) {
			++pos;
			continue;
		}
		if (pos + 1 < end && text[pos] == '/' && text[pos + 1] == '/') {
			pos += 2;
			while (pos < end && text[pos] != '\n')
				++pos;
			continue;
		}
		break;
	}
}

bool read_quoted(const std::string& text, std::size_t& pos, std::size_t end, std::string& out) {
	skip_ws(text, pos, end);
	if (pos >= end || text[pos] != '"')
		return false;

	out.clear();
	++pos;
	while (pos < end) {
		const char ch = text[pos++];
		if (ch == '"')
			return true;
		if (ch == '\\' && pos < end)
			out.push_back(text[pos++]);
		else
			out.push_back(ch);
	}

	return false;
}

bool matching_brace(const std::string& text, std::size_t open_pos, std::size_t& close_pos) {
	int depth = 0;
	bool in_quote = false;

	for (std::size_t i = open_pos; i < text.size(); ++i) {
		const char ch = text[i];
		if (in_quote) {
			if (ch == '\\')
				++i;
			else if (ch == '"')
				in_quote = false;
			continue;
		}

		if (ch == '"') {
			in_quote = true;
			continue;
		}

		if (i + 1 < text.size() && ch == '/' && text[i + 1] == '/') {
			while (i < text.size() && text[i] != '\n')
				++i;
			continue;
		}

		if (ch == '{')
			++depth;
		else if (ch == '}') {
			--depth;
			if (depth == 0) {
				close_pos = i;
				return true;
			}
		}
	}

	return false;
}

bool find_section(const std::string& text, const char* name, std::size_t search_pos, std::size_t& begin, std::size_t& end, std::size_t& next_pos) {
	const std::string needle = std::string("\"") + name + "\"";
	std::size_t pos = text.find(needle, search_pos);
	while (pos != std::string::npos) {
		pos += needle.size();
		skip_ws(text, pos, text.size());
		if (pos < text.size() && text[pos] == '{') {
			std::size_t close = 0;
			if (matching_brace(text, pos, close)) {
				begin = pos + 1;
				end = close;
				next_pos = close + 1;
				return true;
			}
		}

		pos = text.find(needle, pos);
	}

	return false;
}

void skip_block(const std::string& text, std::size_t& pos, std::size_t end) {
	if (pos >= end || text[pos] != '{')
		return;

	std::size_t close = 0;
	if (matching_brace(text, pos, close))
		pos = close + 1;
}

std::unordered_map<std::string, std::string> parse_entry(const std::string& text, std::size_t begin, std::size_t end) {
	std::unordered_map<std::string, std::string> values;
	std::size_t pos = begin;

	while (pos < end) {
		std::string key;
		if (!read_quoted(text, pos, end, key)) {
			++pos;
			continue;
		}

		skip_ws(text, pos, end);
		if (pos < end && text[pos] == '{') {
			skip_block(text, pos, end);
			continue;
		}

		std::string value;
		if (read_quoted(text, pos, end, value))
			values[key] = value;
	}

	return values;
}

int rarity_from_string(const std::string& rarity) {
	if (rarity == "uncommon")
		return 2;
	if (rarity == "rare")
		return 3;
	if (rarity == "mythical")
		return 4;
	if (rarity == "legendary")
		return 5;
	if (rarity == "ancient")
		return 6;
	if (rarity == "contraband")
		return 7;
	return 1;
}

std::string value_or_empty(const std::unordered_map<std::string, std::string>& values, const char* key) {
	auto it = values.find(key);
	return it != values.end() ? it->second : std::string{};
}

std::string localize_or_raw(const std::string& token) {
	if (token.empty())
		return {};

	const char* localized = localize::find_safe(token.c_str());
	if (localized && *localized)
		return localized;
	return token;
}

bool parse_id(const std::string& text, int& id) {
	char* end = nullptr;
	const long value = std::strtol(text.c_str(), &end, 10);
	if (!end || *end != '\0' || value <= 0 || value > INT_MAX)
		return false;
	id = static_cast<int>(value);
	return true;
}

void upsert_cosmetic(std::vector<cosmetic_item_info_t>& out, cosmetic_item_info_t item) {
	auto it = std::find_if(out.begin(), out.end(), [&](const cosmetic_item_info_t& existing) {
		return existing.id == item.id;
	});

	if (it == out.end())
		out.push_back(std::move(item));
	else {
		item.m_image = it->m_image;
		if (item.image_inventory.empty())
			item.image_inventory = it->image_inventory;
		*it = std::move(item);
	}
}

void sort_cosmetics(std::vector<cosmetic_item_info_t>& items) {
	std::sort(items.begin(), items.end(), [](const cosmetic_item_info_t& a, const cosmetic_item_info_t& b) {
		if (a.rarity != b.rarity)
			return a.rarity > b.rarity;
		return a.name < b.name;
		});
}

void parse_cosmetic_section(const std::string& text, const char* section_name, bool stickers, std::vector<cosmetic_item_info_t>& out) {
	std::size_t search_pos = 0;
	std::size_t begin = 0, end = 0, next_pos = 0;
	while (find_section(text, section_name, search_pos, begin, end, next_pos)) {
		search_pos = next_pos;
		std::size_t pos = begin;
		while (pos < end) {
			std::string id_text;
			if (!read_quoted(text, pos, end, id_text)) {
				++pos;
				continue;
			}

			int id = 0;
			if (!parse_id(id_text, id)) {
				skip_ws(text, pos, end);
				if (pos < end && text[pos] == '{')
					skip_block(text, pos, end);
				continue;
			}

			skip_ws(text, pos, end);
			if (pos >= end || text[pos] != '{')
				continue;

			std::size_t close = 0;
			if (!matching_brace(text, pos, close) || close > end)
				break;

			const auto values = parse_entry(text, pos + 1, close);
			pos = close + 1;

			cosmetic_item_info_t item{};
			item.id = id;
			item.internal_name = value_or_empty(values, "name");
			item.rarity = rarity_from_string(value_or_empty(values, "item_rarity"));

			if (stickers) {
				item.name = localize_or_raw(value_or_empty(values, "item_name"));
				item.image_inventory = value_or_empty(values, "image_inventory");
				if (item.image_inventory.empty()) {
					const std::string material = value_or_empty(values, "sticker_material");
					if (!material.empty())
						item.image_inventory = "econ/stickers/" + material;
				}
				if (item.image_inventory.empty()) {
					const std::string material = value_or_empty(values, "patch_material");
					if (!material.empty())
						item.image_inventory = "econ/patches/" + material;
				}
			}
			else {
				item.name = localize_or_raw(value_or_empty(values, "loc_name"));
				item.image_inventory = value_or_empty(values, "image_inventory");
				if (item.image_inventory.empty()) {
					const std::string highlight = value_or_empty(values, "highlight_reel");
					const std::size_t event_end = highlight.find('_');
					if (event_end != std::string::npos) {
						const std::string event = highlight.substr(0, event_end);
						item.image_inventory = "econ/keychains/" + event + "/kc_" + event;
					}
				}
			}

			if (item.name.empty())
				item.name = !item.internal_name.empty() ? item.internal_name : ("ID " + std::to_string(id));

			upsert_cosmetic(out, std::move(item));
		}
	}

	sort_cosmetics(out);
}

const cosmetic_item_info_t* find_cosmetic(const std::vector<cosmetic_item_info_t>& items, int id) {
	auto it = std::find_if(items.begin(), items.end(), [id](const cosmetic_item_info_t& item) {
		return item.id == id;
		});
	return it != items.end() ? &*it : nullptr;
}

} // namespace

bool ItemSchema::is_paint_kit_for_item(const char* simple_weapon_name, c_paint_kit* paint_kit) {
	if (!simple_weapon_name || !paint_kit || !paint_kit->m_name)
		return false;

	std::string path = "panorama/images/econ/default_generated/" +
		std::string(simple_weapon_name) + "_" +
		paint_kit->m_name + "_light_png.vtex_c";

	return ttapp::client::interfaces().m_file_system->exists(path.c_str(), "GAME");
}

void ItemSchema::build_paint_kits_for_item(uint16_t def_index, c_utl_map<int, c_econ_item_definition*>& items, c_utl_map<int, c_paint_kit*>& paint_kit_map) {
	c_econ_item_definition* item_def = nullptr;
	const int items_n = items.count();
	for (int i = 0; i < items_n; i++) {
		auto& node = items.element(i);
		if (node.m_value && node.m_value->m_definition_index == def_index) {
			item_def = node.m_value;
			break;
		}
	}

	if (!item_def)
		return;

	const char* simple_name = item_def->get_item_name();
	if (!simple_name || simple_name[0] == '\0')
		return;


	auto& item_kits = item_paint_kits[def_index];
	if (item_kits.empty())
		item_kits.push_back({ 0, "Default", "default", 1 });

	const int kits_n = paint_kit_map.count();
	for (int i = 0; i < kits_n; i++) {
		auto& node = paint_kit_map.element(i);
		if (!node.m_value)
			continue;

		c_paint_kit* kit = node.m_value;
		if (!kit->m_name || kit->m_id <= 0)
			continue;

		if (is_paint_kit_for_item(simple_name, kit)) {
			const char* display = localize::find_safe(kit->m_description_tag);
			if (!display || !*display)
				display = kit->m_name;

			uint32_t item_rarity = item_def->m_item_rarity;

			uint32_t paint_kit_rarity = kit->m_rarity;

			int final_rarity = (int)item_rarity + (int)paint_kit_rarity - 1;
			if (final_rarity < 0) final_rarity = 0;

			if (paint_kit_rarity == 7) {
				if (final_rarity > 7) final_rarity = 7;
			}
			else {
				if (final_rarity > 6) final_rarity = 6;
			}

			item_kits.push_back({
				kit->m_id,
				std::string(display),
				std::string(kit->m_name),
				final_rarity
			});
		}
	}

	if (item_kits.size() > 1) {
		std::sort(item_kits.begin() + 1, item_kits.end(), [](const paint_kit_info_t& a, const paint_kit_info_t& b) {
			if (a.rarity != b.rarity)
				return a.rarity > b.rarity;
			return a.name < b.name;
			});
	}
}

bool ItemSchema::ensure_paint_kits_for_item(uint16_t def_index) {
	if (!m_initialized || def_index == 0)
		return false;
	if (m_loaded_paint_kits.find(def_index) != m_loaded_paint_kits.end())
		return true;

	static std::mutex paint_kits_mutex;
	std::lock_guard<std::mutex> lock(paint_kits_mutex);

	if (m_loaded_paint_kits.find(def_index) != m_loaded_paint_kits.end())
		return true;

	auto* source2_client = ttapp::client::interfaces().m_source2_client;
	if (!source2_client)
		return false;

	auto* item_system = source2_client->get_econ_item_system();
	if (!item_system)
		return false;

	auto* item_schema = item_system->get_econ_item_schema();
	if (!item_schema)
		return false;

	auto& items = item_schema->get_sorted_item_definition_map();
	auto& paint_kit_map = item_schema->get_paint_kits();

	build_paint_kits_for_item(def_index, items, paint_kit_map);

	auto kits_it = item_paint_kits.find(def_index);
	if (kits_it == item_paint_kits.end() || kits_it->second.empty()) {
		auto& kits = item_paint_kits[def_index];
		kits.push_back({ 0, "Default", "default", 1 });
		kits_it = item_paint_kits.find(def_index);
	}

	auto& names = item_paint_kit_names[def_index];
	names.clear();
	for (auto& kit : kits_it->second)
		names.push_back(kit.name.c_str());

	m_loaded_paint_kits.insert(def_index);
	return true;
}

const std::vector<const char*>& ItemSchema::get_paint_kit_names_for_item(uint16_t def_index) {
	static std::vector<const char*> empty = { "Default" };
	ensure_paint_kits_for_item(def_index);

	auto it = item_paint_kit_names.find(def_index);
	return it != item_paint_kit_names.end() ? it->second : empty;
}

int ItemSchema::get_paint_kit_id_for_item(uint16_t def_index, int index) {
	ensure_paint_kits_for_item(def_index);

	auto it = item_paint_kits.find(def_index);
	if (it != item_paint_kits.end() && index >= 0 && index < (int)it->second.size())
		return it->second[index].id;
	return 0;
}

const std::vector<cosmetic_item_info_t>& ItemSchema::get_sticker_kits() const {
	return m_sticker_kits;
}

const std::vector<cosmetic_item_info_t>& ItemSchema::get_keychain_definitions() const {
	return m_keychain_definitions;
}

const cosmetic_item_info_t* ItemSchema::find_sticker_kit(int id) const {
	return find_cosmetic(m_sticker_kits, id);
}

const cosmetic_item_info_t* ItemSchema::find_keychain_definition(int id) const {
	return find_cosmetic(m_keychain_definitions, id);
}

void ItemSchema::load_cosmetic_catalogs() {
	std::string items_game;
	if (!read_items_game(items_game)) {
		LOG_ERROR(xorstr_("[item_schema] failed to load sticker/keychain catalogs"));
		return;
	}

	parse_cosmetic_section(items_game, "sticker_kits", true, m_sticker_kits);
	parse_cosmetic_section(items_game, "keychain_definitions", false, m_keychain_definitions);
	LOG_INFO(xorstr_("[item_schema] %d stickers, %d keychains"),
		(int)m_sticker_kits.size(), (int)m_keychain_definitions.size());
}

void ItemSchema::initialize() {
	static std::mutex init_mutex;
	std::lock_guard<std::mutex> lock(init_mutex);

	if (m_initialized)
		return;

	LOG_INFO(xorstr_("[item_schema] Initializing..."));

	knives.clear();
	gloves.clear();
	weapons.clear();
	agents_ct.clear();
	agents_t.clear();
	m_sticker_kits.clear();
	m_keychain_definitions.clear();
	item_paint_kits.clear();
	item_paint_kit_names.clear();
	m_loaded_paint_kits.clear();
	knife_names_cstr.clear();
	glove_names_cstr.clear();
	weapon_names_cstr.clear();
	agent_ct_names_cstr.clear();
	agent_t_names_cstr.clear();

	agents_ct.push_back({ 0, "Default", "" });
	agents_t.push_back({ 0, "Default", "" });
	load_cosmetic_catalogs();

	auto* source2_client = ttapp::client::interfaces().m_source2_client;
	if (!source2_client) {
		LOG_ERROR(xorstr_("[item_schema] m_source2_client is null"));
		return;
	}

	auto* item_system = source2_client->get_econ_item_system();
	if (!item_system) {
		LOG_ERROR(xorstr_("[item_schema] m_source2_client->get_econ_item_system() is null"));
		return;
	}

	auto* item_schema = item_system->get_econ_item_schema();
	if (!item_schema) {
		LOG_ERROR(xorstr_("[item_schema] item_system->get_econ_item_schema() is null"));
		return;
	}

	auto& items = item_schema->get_sorted_item_definition_map();
	const int items_n = items.count();
	LOG_INFO(xorstr_("[item_schema] items_n count = %d"), items_n);

	for (int i = 0; i < items_n; i++) {
		auto& node = items.element(i);
		if (!node.m_value || !node.m_value->m_item_type_name)
			continue;

		c_econ_item_definition* item_def = node.m_value;

		std::string item_name;
		if (item_def->m_item_base_name && item_def->m_item_base_name[0] != '\0') {
			const char* localized = localize::find_safe(item_def->m_item_base_name);
			item_name = (localized && *localized) ? localized : item_def->m_item_base_name;
		} else {
			item_name = "Item " + std::to_string(item_def->m_definition_index);
		}

		const char* model_path = item_def->get_model_name();

		if (item_def->is_knife(false)) {
			if (std::find_if(knives.begin(), knives.end(), [&](const item_info_t& k) { return k.name == item_name; }) == knives.end()) {
				const char* simple_name_ptr = item_def->get_item_name();
				std::string simple_name = simple_name_ptr ? simple_name_ptr : "";
				knives.push_back({ item_def->m_definition_index, item_name, model_path, simple_name });
			}
		}
		else if (item_def->is_glove(false)) {
			if (std::find_if(gloves.begin(), gloves.end(), [&](const item_info_t& g) { return g.name == item_name; }) == gloves.end()) {
				const char* simple_name_ptr = item_def->get_item_name();
				std::string simple_name = simple_name_ptr ? simple_name_ptr : "";
				gloves.push_back({ item_def->m_definition_index, item_name, model_path, simple_name });
			}
		}
		else if (item_def->is_agent()) {
			if (model_path) {
				const char* simple_name_ptr = item_def->get_item_name();
				std::string simple_name = simple_name_ptr ? simple_name_ptr : "";
				if (strstr(model_path, "ctm_")) {
					if (std::find_if(agents_ct.begin(), agents_ct.end(), [&](const item_info_t& a) { return a.name == item_name; }) == agents_ct.end()) {
						agents_ct.push_back({ item_def->m_definition_index, item_name, model_path, simple_name, "", 0, (int)item_def->m_item_rarity });
					}
				} else if (strstr(model_path, "tm_")) {
					if (std::find_if(agents_t.begin(), agents_t.end(), [&](const item_info_t& a) { return a.name == item_name; }) == agents_t.end()) {
						agents_t.push_back({ item_def->m_definition_index, item_name, model_path, simple_name, "", 0, (int)item_def->m_item_rarity });
					}
				}
			}
		}
		else {
			const char* type_name = item_def->m_item_type_name;
			std::string category;
			bool is_weapon = false;
			if (strstr(type_name, "Pistol"))           { is_weapon = true; category = "Pistol"; }
			else if (strstr(type_name, "Rifle"))        { is_weapon = true; category = "Rifle"; }
			else if (strstr(type_name, "SubMachinegun")||strstr(type_name, "SMG")) { is_weapon = true; category = "SMG"; }
			else if (strstr(type_name, "Sniper"))       { is_weapon = true; category = "Sniper"; }
			else if (strstr(type_name, "Shotgun"))      { is_weapon = true; category = "Shotgun"; }
			else if (strstr(type_name, "Machine"))      { is_weapon = true; category = "Machine Gun"; }
			else if (item_def->m_definition_index == WEAPON_TASER) { is_weapon = true; category = "Other"; }

			if (is_weapon && item_def->m_definition_index > 0 && item_def->m_definition_index <= 70) {
				const char* simple_name_ptr = item_def->get_item_name();
				std::string simple_name = simple_name_ptr ? simple_name_ptr : "";
				weapons.push_back({ item_def->m_definition_index, item_name, model_path, simple_name, category });
			}
		}
	}

	std::sort(weapons.begin(), weapons.end(), [](const item_info_t& a, const item_info_t& b) {
		return a.definition_index < b.definition_index;
	});
	sort_cosmetics(m_sticker_kits);

	auto seed_default_paint_kit = [this](uint16_t def_index) {
		if (def_index == 0)
			return;

		auto& kits = item_paint_kits[def_index];
		kits.clear();
		kits.push_back({ 0, "Default", "default", 1 });

		auto& names = item_paint_kit_names[def_index];
		names.clear();
		names.push_back(kits.front().name.c_str());
	};

	for (auto& knife : knives)
		seed_default_paint_kit(knife.definition_index);
	for (auto& glove : gloves)
		seed_default_paint_kit(glove.definition_index);
	for (auto& weapon : weapons)
		seed_default_paint_kit(weapon.definition_index);

	for (auto& knife : knives)
		knife_names_cstr.push_back(knife.name.c_str());
	for (auto& glove : gloves)
		glove_names_cstr.push_back(glove.name.c_str());
	for (auto& weapon : weapons)
		weapon_names_cstr.push_back(weapon.name.c_str());
	for (auto& agent : agents_ct)
		agent_ct_names_cstr.push_back(agent.name.c_str());
	for (auto& agent : agents_t)
		agent_t_names_cstr.push_back(agent.name.c_str());

	m_initialized = true;
	LOG_INFO(xorstr_("[item_schema] %d knives, %d gloves, %d weapons, %d CT agents, %d T agents"),
		(int)knives.size() - 1, (int)gloves.size() - 1, (int)weapons.size(), (int)agents_ct.size() - 1, (int)agents_t.size() - 1);
}

std::uintptr_t c_hud::find_hud_element(const char* name) {

	using fn_t = std::uintptr_t(__fastcall*)(const char*);
	static auto fn = reinterpret_cast<fn_t>(
		ttapp::client::patterns().scan(ttapp::client::modules().m_modules.client_dll.get_name(),
			"4C 8B DC 53 48 83 EC ? 48 8B 05")
	);
	return fn ? fn(name) : 0;
}

void c_hud::clear_hud_weapon_icon(std::uintptr_t hud_weapons, std::int32_t index, std::int64_t unk) {
	static auto call_address = ttapp::client::patterns().scan(ttapp::client::modules().m_modules.client_dll.get_name(), "E8 ? ? ? ? 8B F8 C6 84 24");
	if (!call_address)
		return;

	auto fn = reinterpret_cast<std::int64_t(__fastcall*)(std::uintptr_t, std::int32_t, std::int64_t)>(
		reinterpret_cast<std::uintptr_t>(call_address) + 5 + *reinterpret_cast<std::int32_t*>(call_address + 1)
	);
	fn(hud_weapons, index, unk);
}

struct hud_weapon_panel_t {
	std::uintptr_t base  = 0;
	std::uintptr_t data  = 0;
	std::int32_t   count = 0;
};

static bool resolve_weapon_panel(hud_weapon_panel_t& out) {
	const auto hud = c_hud::find_hud_element("HudWeaponSelection");
	if (!valid_ptr(hud))
		return false;

	out.base  = hud - 0x98;
	out.data  = *reinterpret_cast<std::uintptr_t*>(out.base + 0x58);
	out.count = *reinterpret_cast<std::int32_t*>  (out.base + 0x50);
	return valid_ptr(out.data) && out.count > 0 && out.count <= 64;
}

void c_hud::clear_hud_weapon_icons() {
	__try {
		hud_weapon_panel_t panel;
		if (!resolve_weapon_panel(panel))
			return;
		for (std::int32_t i = panel.count - 1; i >= 0; --i)
			clear_hud_weapon_icon(panel.base, i, 0);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {}
}

void c_hud::clear_hud_weapon_icon_for(c_base_entity* weapon) {
	if (!valid_ptr(weapon))
		return;
	__try {
		hud_weapon_panel_t panel;
		if (!resolve_weapon_panel(panel))
			return;

		auto* es = ttapp::client::interfaces().m_entity_system;
		for (std::int32_t i = panel.count - 1; i >= 0; --i) {
			const auto handle = *reinterpret_cast<std::int32_t*>(panel.data + 72 * i + 0x38);
			if (handle < 0)
				continue;
			if (es->get_base_entity(handle & 0x7FFF) == weapon) {
				clear_hud_weapon_icon(panel.base, i, 0);
				return;
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {}
}

static void invoke_regen(void(__fastcall* fn)()) {
	__try {
		fn();
	} __except (EXCEPTION_EXECUTE_HANDLER) {}
}

void c_hud::regenerate_skins() {

	static auto fn = reinterpret_cast<void(__fastcall*)()>(
		ttapp::client::patterns().scan(ttapp::client::modules().m_modules.client_dll.get_name(),
			"48 83 EC ? E8 ? ? ? ? 48 85 C0 0F 84 ? ? ? ? 48 8B 10"));
	if (fn)
		invoke_regen(fn);
}
