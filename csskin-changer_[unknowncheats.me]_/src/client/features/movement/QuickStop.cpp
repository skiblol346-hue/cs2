#include "platform/ClientCore.hpp"

#include "features/movement/QuickStop.hpp"
#include "runtime/Runtime.hpp"
#include "game/entities/c_cs_player_pawn.hpp"
#include "game/interfaces/i_csgo_input.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace {

constexpr float k_quick_stop_deadzone = 0.5f;
constexpr float k_quick_stop_move = 1.0f;
constexpr float k_quick_stop_button_speed = 20.0f;
constexpr float k_pi = 3.14159265358979323846f;
constexpr uint64_t k_movement_buttons = IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT;

bool has_movement_input(const c_user_cmd* cmd)
{
	if (!cmd)
		return true;

	return (cmd->m_button_state.m_button_state & k_movement_buttons) != 0;
}

void clear_movement_buttons(c_in_button_state& buttons)
{
	buttons.m_button_state &= ~k_movement_buttons;
	buttons.m_button_state2 &= ~k_movement_buttons;
	buttons.m_button_state3 &= ~k_movement_buttons;
}

void sync_buttons_pb(CBaseUserCmdPB* base_cmd, const c_in_button_state& buttons)
{
	if (!base_cmd)
		return;

	auto* buttons_pb = base_cmd->mutable_buttons_pb();
	if (!buttons_pb)
		return;

	buttons_pb->set_buttonstate1(buttons.m_button_state);
	buttons_pb->set_buttonstate2(buttons.m_button_state2);
	buttons_pb->set_buttonstate3(buttons.m_button_state3);
}

void set_quick_stop_buttons(c_user_cmd* cmd, float forward_move, float left_move)
{
	if (!cmd)
		return;

	clear_movement_buttons(cmd->m_button_state);

	constexpr float k_button_threshold = 0.05f;

	if (forward_move > k_button_threshold)
		cmd->m_button_state.set_button_state(IN_FORWARD, c_in_button_state::IN_BUTTON_DOWN);
	else if (forward_move < -k_button_threshold)
		cmd->m_button_state.set_button_state(IN_BACK, c_in_button_state::IN_BUTTON_DOWN);

	if (left_move > k_button_threshold)
		cmd->m_button_state.set_button_state(IN_MOVELEFT, c_in_button_state::IN_BUTTON_DOWN);
	else if (left_move < -k_button_threshold)
		cmd->m_button_state.set_button_state(IN_MOVERIGHT, c_in_button_state::IN_BUTTON_DOWN);
}

void clear_subtick_movement(CBaseUserCmdPB* base_cmd)
{
	if (!base_cmd)
		return;

	for (int i = 0; i < base_cmd->subtick_moves_size(); ++i) {
		auto* subtick = base_cmd->mutable_subtick_moves(i);
		if (!subtick)
			continue;

		subtick->set_analog_forward_delta(0.0f);
		subtick->set_analog_left_delta(0.0f);

		if (subtick->has_button() && (subtick->button() & k_movement_buttons) != 0) {
			subtick->clear_button();
			subtick->clear_pressed();
		}
	}
}

void log_quick_stop(const char* state, float speed, float forward_move, float left_move, const c_in_button_state& buttons)
{
	if (!logger::m_save_to_file)
		return;

	static DWORD64 last_log_ms = 0;
	static uint64_t last_buttons = 0;
	static const char* last_state = nullptr;
	static int last_direction = 0;

	const DWORD64 now = GetTickCount64();
	const int direction = left_move > 0.05f ? 1 : left_move < -0.05f ? -1 : forward_move > 0.05f ? 2 : forward_move < -0.05f ? -2 : 0;
	if (last_state == state && last_buttons == buttons.m_button_state && last_direction == direction && now - last_log_ms < 250)
		return;

	last_log_ms = now;
	last_buttons = buttons.m_button_state;
	last_state = state;
	last_direction = direction;

	char log_path[MAX_PATH] = {};
	logger::build_path(log_path, sizeof(log_path), "quick_stop_log.txt");

	FILE* file = nullptr;
	if (fopen_s(&file, log_path, "a") != 0 || !file)
		return;

	SYSTEMTIME time;
	GetLocalTime(&time);
	fprintf(file, "[%02d:%02d:%02d] %s speed=%.3f forward=%.3f left=%.3f buttons=0x%llX\n",
		time.wHour, time.wMinute, time.wSecond, state, speed, forward_move, left_move,
		static_cast<unsigned long long>(buttons.m_button_state));
	fclose(file);
}
} // namespace
void QuickStop::run(i_csgo_input* input, int, bool active)
{
	if (!ttapp::client::config().misc.m_quick_stop)
		return;

	if (!active) {
		return;
	}

	if (!input) {
		return;
	}

	auto* cmd = ttapp::client::frame().m_user_cmd;
	if (!cmd) {
		return;
	}

	if (!ttapp::client::frame().m_local_pawn) {
		return;
	}

	auto* base_cmd = cmd->get_base_cmd();
	if (!base_cmd) {
		return;
	}

	if (has_movement_input(cmd)) {
		return;
	}

	auto* local_pawn = reinterpret_cast<c_cs_player_pawn*>(ttapp::client::frame().m_local_pawn);
	if (!local_pawn->is_alive()) {
		return;
	}

	if (!(local_pawn->m_flags() & FL_ONGROUND)) {
		return;
	}

	const vec3_t velocity = local_pawn->m_vec_velocity();
	const float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
	if (speed < k_quick_stop_deadzone) {
		base_cmd->set_forwardmove(0.0f);
		base_cmd->set_leftmove(0.0f);
		input->forward_move = 0.0f;
		input->left_move = 0.0f;
		clear_movement_buttons(cmd->m_button_state);
		sync_buttons_pb(base_cmd, cmd->m_button_state);
		clear_subtick_movement(base_cmd);
		log_quick_stop("zero", speed, 0.0f, 0.0f, cmd->m_button_state);
		return;
	}

	const float yaw_rad = input->get_view_angles().y * (k_pi / 180.0f);
	const float sin_yaw = std::sin(yaw_rad);
	const float cos_yaw = std::cos(yaw_rad);

	const vec3_t opposite(-velocity.x, -velocity.y, 0.0f);
	const float forward = opposite.x * cos_yaw + opposite.y * sin_yaw;
	const float left = opposite.x * -sin_yaw + opposite.y * cos_yaw;

	const float max_component = std::max(std::abs(forward), std::abs(left));
	if (max_component > 0.0f) {
		const float forward_move = (forward / max_component) * k_quick_stop_move;
		const float left_move = (left / max_component) * k_quick_stop_move;

		base_cmd->set_forwardmove(forward_move);
		base_cmd->set_leftmove(left_move);
		input->forward_move = forward_move;
		input->left_move = left_move;
		if (speed >= k_quick_stop_button_speed)
			set_quick_stop_buttons(cmd, forward_move, left_move);
		else
			clear_movement_buttons(cmd->m_button_state);
		sync_buttons_pb(base_cmd, cmd->m_button_state);
		clear_subtick_movement(base_cmd);
		log_quick_stop("counter", speed, forward_move, left_move, cmd->m_button_state);
	}
}
