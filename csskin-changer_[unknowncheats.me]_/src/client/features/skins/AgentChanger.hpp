#pragma once

#include "platform/ClientCore.hpp"
#include "game/entities/c_cs_player_pawn.hpp"
#include "game/items/ItemSchema.hpp"

class AgentChanger {
public:
	void run(int stage);
	bool should_update = false;

private:
	uint16_t m_last_agent_ct = 0;
	uint16_t m_last_agent_t = 0;
	float m_last_spawn_time = -1.f;
	int m_last_team = 0;

	int m_update_frames = 0;
};
