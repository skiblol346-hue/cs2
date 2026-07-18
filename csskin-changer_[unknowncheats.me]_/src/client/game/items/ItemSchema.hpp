#pragma once

#include "platform/ClientCore.hpp"
#include "game/entities/game_enums.hpp"
#include "game/entities/c_cs_player_pawn.hpp"
#include "game/interfaces/i_econ_item_system.hpp"
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

class CImageProxySource;

struct item_info_t {
	uint16_t definition_index;
	std::string name;
	const char* model_path;
	std::string simple_name;
	std::string category; // e.g. "Pistol", "Rifle", "SMG", "Sniper", "Shotgun", "Machine Gun"
	uint32_t subclass_id;
	int rarity;
	mutable CImageProxySource* m_image = nullptr;

	item_info_t(uint16_t def, std::string nm, const char* path, std::string simple_nm = "", std::string cat = "", uint32_t sub = 0, int rare = 1)
		: definition_index(def), name(std::move(nm)), model_path(path), simple_name(std::move(simple_nm)), category(std::move(cat)), subclass_id(sub), rarity(rare), m_image(nullptr) {}
};

struct paint_kit_info_t {
	int id;
	std::string name;
	std::string skinToken;
	int rarity;
	mutable CImageProxySource* m_image = nullptr;
};

struct cosmetic_item_info_t {
	int id = 0;
	std::string name;
	std::string internal_name;
	std::string image_inventory;
	int rarity = 1;
	mutable CImageProxySource* m_image = nullptr;
};

class ItemSchema {
public:
	std::vector<item_info_t> knives;
	std::vector<item_info_t> gloves;
	std::vector<item_info_t> weapons;
	std::vector<item_info_t> agents_ct;
	std::vector<item_info_t> agents_t;

	std::unordered_map<uint16_t, std::vector<paint_kit_info_t>> item_paint_kits;
	std::unordered_map<uint16_t, std::vector<const char*>> item_paint_kit_names;

	std::vector<const char*> knife_names_cstr;
	std::vector<const char*> glove_names_cstr;
	std::vector<const char*> weapon_names_cstr;
	std::vector<const char*> agent_ct_names_cstr;
	std::vector<const char*> agent_t_names_cstr;

	void initialize();
	bool is_initialized() const { return m_initialized; }
	bool ensure_paint_kits_for_item(uint16_t def_index);

	const std::vector<const char*>& get_paint_kit_names_for_item(uint16_t def_index);
	int get_paint_kit_id_for_item(uint16_t def_index, int index);
	const std::vector<cosmetic_item_info_t>& get_sticker_kits() const;
	const std::vector<cosmetic_item_info_t>& get_keychain_definitions() const;
	const cosmetic_item_info_t* find_sticker_kit(int id) const;
	const cosmetic_item_info_t* find_keychain_definition(int id) const;

private:
	bool m_initialized = false;
	std::vector<cosmetic_item_info_t> m_sticker_kits;
	std::vector<cosmetic_item_info_t> m_keychain_definitions;
	std::unordered_set<uint16_t> m_loaded_paint_kits;
	bool is_paint_kit_for_item(const char* simple_weapon_name, c_paint_kit* paint_kit);
	void build_paint_kits_for_item(uint16_t def_index, c_utl_map<int, c_econ_item_definition*>& items, c_utl_map<int, c_paint_kit*>& paint_kit_map);
	void load_cosmetic_catalogs();
};
class c_hud {
public:
	static std::uintptr_t find_hud_element(const char* name);
	static void clear_hud_weapon_icon(std::uintptr_t hud_weapons, std::int32_t index, std::int64_t unk = 0);

	static void clear_hud_weapon_icons();

	static void clear_hud_weapon_icon_for(class c_base_entity* weapon);
	static void regenerate_skins();
};
