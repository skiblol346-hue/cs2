#pragma once

#include "platform/ClientCore.hpp"
#include "config/Config.hpp"
#include "game/entities/c_cs_player_pawn.hpp"
#include "game/items/ItemSchema.hpp"
#include <chrono>
#include <unordered_map>

class SkinChanger {
public:
	void run(int stage);
	void initialize();

	bool should_update = false;
	bool is_initialized() const { return m_initialized; }

private:
	bool m_initialized = false;
	uint16_t m_last_knife = 0;
	int m_last_knife_paint_kit_id = 0;
	float m_last_knife_wear = 0.0001f;
	int m_last_knife_seed = 0;
	bool m_last_knife_stattrak_enabled = false;
	int m_last_knife_stattrak_count = 0;
	bool m_last_knife_stattrak_entity_added = false;

	int m_update_frames = 0;
	int m_clear_frames = 0;
	uint32_t m_last_active_weapon_handle = 0;


	bool m_last_skin_enabled = false;
	bool m_last_knife_enabled = false;
	float m_last_spawn_time = 0.0f;
	int m_last_team = 0;
	std::vector<std::pair<c_econ_item_view*, uint32_t>> m_restore_list;
	std::vector<uint32_t> m_last_weapon_indices;
	std::unordered_map<uint32_t, int> m_visual_refresh_frames;
	struct color4_state_t {
		bool enabled = false;
		float colors[4][4] = {};
	};
	struct weapon_cosmetic_state_t {
		bool custom_colors_enabled = false;
		float custom_colors[4][4] = {};
		Config::skin_changer_t::sticker_slot_t stickers[Config::skin_changer_t::sticker_slot_count];
		Config::skin_changer_t::keychain_t keychain;
		bool stattrak_enabled = false;
		int stattrak_count = 0;
		bool stattrak_entity_added = false;
	};
	std::unordered_map<uint32_t, weapon_cosmetic_state_t> m_last_weapon_cosmetic_states;
	color4_state_t m_last_knife_color_state;
	uint32_t m_last_knife_weapon_handle = 0;
	c_econ_entity* m_last_knife_weapon = nullptr;
	c_econ_item_view* m_last_knife_item = nullptr;
	c_cs_player_pawn* m_last_knife_pawn = nullptr;
	c_base_entity* m_last_knife_hud_weapon = nullptr;
	const char* m_last_knife_model_path = nullptr;
	std::vector<uint16_t> m_last_weapon_def_indices; 
	std::chrono::steady_clock::time_point m_change_detected_time; 
	bool m_change_pending_update = false; 


	void check_for_updates(c_cs_player_pawn* local_pawn);

	int m_last_health = 0;
	int m_restore_delay_frames = 0;
	c_base_entity* get_hud_weapon(c_base_entity* weapon, c_cs_player_pawn* local_pawn);
	void refresh_weapon_skin_visuals(c_econ_entity* weapon, c_cs_player_pawn* local_pawn, int paint_kit_id);
	void apply_skin(c_econ_entity* weapon, c_econ_item_view* item, int paint_kit_id, const Config::skin_changer_t::weapon_skin_t& skin, c_cs_player_pawn* local_pawn, uint16_t def_index = 0, bool add_stattrak_entity = true, bool refresh_visuals = true);
	void process_weapon(c_econ_entity* weapon, c_econ_item_view* item, c_cs_player_pawn* local_pawn, uint32_t weapon_handle, bool force_update, bool& did_update, uint64_t local_steam_id, bool active_weapon_changed);
	void process_knife(c_econ_entity* weapon, c_econ_item_view* item, c_cs_player_pawn* local_pawn, uint32_t weapon_handle, bool force_update, bool& did_update, uint64_t local_steam_id);
	void force_refresh_hud(c_econ_item_view* item);
};
