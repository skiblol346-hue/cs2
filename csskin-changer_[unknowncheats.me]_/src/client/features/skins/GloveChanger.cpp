#include "runtime/Runtime.hpp"
#include "features/skins/GloveChanger.hpp"
#include "game/items/EconItemAttributeManager.hpp"
#include "game/InterfaceRegistry.hpp"

#include "features/skins/GloveChanger.hpp"
#include "game/items/EconItemAttributeManager.hpp"
#include "game/InterfaceRegistry.hpp"

void GloveChanger::run(int stage) {
	if (!ttapp::client::config().glove_changer.m_enabled || stage != 7 || !ttapp::client::frame().m_local_pawn)
		return;

	auto* local_pawn = reinterpret_cast<c_cs_player_pawn*>(ttapp::client::frame().m_local_pawn);
	if (!valid_ptr(local_pawn) || local_pawn->m_health() <= 0)
		return;

	

	auto* identity = local_pawn->m_entity();
	if (!identity || !identity->is_valid())
		return;

	if (!identity->is_safe_to_modify())
		return;

	if (ttapp::client::config().glove_changer.m_glove == 0)
		return;

	auto* glove_item = local_pawn->m_econ_gloves();
	if (!glove_item)
		return;

	if (!ttapp::client::item_schema().is_initialized()
		|| ttapp::client::config().glove_changer.m_glove < 0
		|| ttapp::client::config().glove_changer.m_glove >= (int)ttapp::client::item_schema().gloves.size())
		return;

	const uint16_t selected_glove =
		ttapp::client::item_schema().gloves[ttapp::client::config().glove_changer.m_glove].definition_index;
	if (selected_glove == 0)
		return;

	int paint_kit_id = ttapp::client::item_schema().get_paint_kit_id_for_item(selected_glove, ttapp::client::config().glove_changer.m_paint_kit);

	auto* paint_kit = glove_item->construct_paint_kit();

	const float current_spawn_time = local_pawn->m_last_spawn_time_index();
	const int   current_team = local_pawn->m_team_num();

	const bool config_changed = (m_last_glove != selected_glove)
		|| (m_last_paint_kit_id != paint_kit_id)
		|| (m_last_wear != ttapp::client::config().glove_changer.m_wear)
		|| (m_last_seed != ttapp::client::config().glove_changer.m_seed);
	const bool team_changed = (current_team != m_last_team) && m_last_team != 0;
	const bool spawn_changed = (current_spawn_time != m_last_spawn_time);
	const bool pawn_state_changed = team_changed || spawn_changed;

	std::vector<uint32_t> current_weapon_indices;
	auto* weapon_service = local_pawn->m_weapon_services();
	if (valid_ptr(weapon_service)) {
		auto& my_weapons = weapon_service->my_weapons();
		for (unsigned int i = 0; i < my_weapons.m_size; i++) {
			current_weapon_indices.push_back(my_weapons.m_elements[i].to_int());
		}
	}
	const bool weapon_changed = (current_weapon_indices != m_last_weapon_indices);
	m_last_weapon_indices = current_weapon_indices;

	const bool engine_reset = (glove_item->m_definition_index() != selected_glove)
		|| !glove_item->m_initialized()
		|| local_pawn->m_need_to_reapply_gloves();

	if (team_changed)
		m_clear_frames = 2;
	if (config_changed || pawn_state_changed || engine_reset || should_update || weapon_changed)
		m_update_frames = 4;

	if (m_clear_frames > 0) {
		econ_item_attribute_manager::remove(glove_item);

		glove_item->m_definition_index() = 0;
		glove_item->m_initialized() = false;
		local_pawn->m_need_to_reapply_gloves() = true;
		m_clear_frames--;

		m_last_glove = 0;
		m_last_paint_kit_id = 0;
		m_last_team = current_team;
		should_update = false;
		return;
	}

	if (m_update_frames <= 0) {
		should_update = false;
		return;
	}

	glove_item->m_definition_index() = selected_glove;
	glove_item->m_entity_quality() = QUALITY_UNUSUAL;

	if (paint_kit && paint_kit_id > 0) {
		auto* source2_client = ttapp::client::interfaces().m_source2_client;
		auto* item_system = source2_client ? source2_client->get_econ_item_system() : nullptr;
		auto* item_schema = item_system ? item_system->get_econ_item_schema() : nullptr;
		if (auto* desired_pk = item_schema ? item_schema->get_paint_kits().find_by_key(paint_kit_id) : nullptr)
			paint_kit->m_name = desired_pk->m_name;
	}

	econ_item_attribute_manager::remove(glove_item);
	if (paint_kit_id > 0)
		econ_item_attribute_manager::create(glove_item, paint_kit_id,
			ttapp::client::config().glove_changer.m_wear, ttapp::client::config().glove_changer.m_seed);
	auto* controller = local_pawn->get_controller();
	uint32_t local_account_id = controller ? (uint32_t)controller->m_steam_id() : 0;
	uint32_t origIDHigh = glove_item->m_item_id_high();
	uint32_t real_item_id_low = glove_item->m_item_id_low();
	uint64_t real_id_full = glove_item->m_item_id();

	glove_item->m_item_id_high() = 0xFFFFFFFF;
	glove_item->m_item_id_low() = real_item_id_low;
	glove_item->m_item_id() = real_id_full;           //m_iItemID
	glove_item->m_account_id() = local_account_id; //m_iAccountID

	glove_item->m_initialized() = true;            //m_bInitialized
	glove_item->m_bDisallowSOCm() = false;

	glove_item->m_bRestoreCustomMaterialAfterPrecache() = true;
	glove_item->m_initialized() = true;
	local_pawn->set_body_group();
	local_pawn->m_need_to_reapply_gloves() = true;

	m_last_glove = selected_glove;
	m_last_paint_kit_id = paint_kit_id;
	m_last_wear = ttapp::client::config().glove_changer.m_wear;
	m_last_seed = ttapp::client::config().glove_changer.m_seed;
	m_last_spawn_time = current_spawn_time;
	m_last_team = current_team;
	m_update_frames--;
	should_update = false;
}
