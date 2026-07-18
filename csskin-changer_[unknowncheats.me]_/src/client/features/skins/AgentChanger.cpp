#include "runtime/Runtime.hpp"
#include "features/skins/AgentChanger.hpp"
#include "game/InterfaceRegistry.hpp"

void AgentChanger::run(int stage) {
	if (!ttapp::client::config().agent_changer.m_enabled || stage != 7 || !ttapp::client::frame().m_local_pawn)
		return;

	auto* local_pawn = reinterpret_cast<c_cs_player_pawn*>(ttapp::client::frame().m_local_pawn);
	if (!valid_ptr(local_pawn) || local_pawn->m_health() <= 0)
		return;

	auto* identity = local_pawn->m_entity();
	if (!identity || !identity->is_valid())
		return;

	if (!identity->is_safe_to_modify())
		return;

	if (!ttapp::client::item_schema().is_initialized())
		return;

	const float current_spawn_time = local_pawn->m_last_spawn_time_index();
	const int current_team = local_pawn->m_team_num();

	// Team must be CT (3) or T (2)
	if (current_team != 2 && current_team != 3)
		return;

	int selected_idx = 0;
	uint16_t selected_agent_def = 0;
	const char* model_path = nullptr;

	if (current_team == 3) {
		selected_idx = ttapp::client::config().agent_changer.m_agent_ct;
		if (selected_idx > 0 && selected_idx < (int)ttapp::client::item_schema().agents_ct.size()) {
			selected_agent_def = ttapp::client::item_schema().agents_ct[selected_idx].definition_index;
			model_path = ttapp::client::item_schema().agents_ct[selected_idx].model_path;
		}
	} else {
		selected_idx = ttapp::client::config().agent_changer.m_agent_t;
		if (selected_idx > 0 && selected_idx < (int)ttapp::client::item_schema().agents_t.size()) {
			selected_agent_def = ttapp::client::item_schema().agents_t[selected_idx].definition_index;
			model_path = ttapp::client::item_schema().agents_t[selected_idx].model_path;
		}
	}

	const bool ct_agent_changed = (current_team == 3 && m_last_agent_ct != selected_agent_def);
	const bool t_agent_changed = (current_team == 2 && m_last_agent_t != selected_agent_def);
	const bool config_changed = ct_agent_changed || t_agent_changed;
	const bool team_changed = (current_team != m_last_team) && m_last_team != 0;
	const bool spawn_changed = (current_spawn_time != m_last_spawn_time);
	const bool pawn_state_changed = team_changed || spawn_changed;

	if (config_changed || pawn_state_changed || should_update) {
		m_update_frames = 4;
	}

	if (m_update_frames <= 0) {
		should_update = false;
		return;
	}

	if (model_path && model_path[0] != '\0') {
		local_pawn->set_model(model_path);
	}

	if (current_team == 3) {
		m_last_agent_ct = selected_agent_def;
	} else if (current_team == 2) {
		m_last_agent_t = selected_agent_def;
	}

	m_last_spawn_time = current_spawn_time;
	m_last_team = current_team;
	m_update_frames--;
	should_update = false;
}
