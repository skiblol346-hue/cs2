#include "platform/ClientCore.hpp"

#include "features/skins/SkinEventHandler.hpp"
#include "features/skins/AgentChanger.hpp"
#include "features/skins/GloveChanger.hpp"
#include "features/skins/SkinChanger.hpp"
#include "game/interfaces/i_game_event.hpp"
#include "platform/Hash.hpp"
#include "runtime/Runtime.hpp"

#include <cstdint>
#include <cstring>

namespace skin_events {
namespace event_hashes {
	constexpr std::uint32_t round_start = fnv1a::hash_32("round_start");
	constexpr std::uint32_t player_death = fnv1a::hash_32("player_death");
	constexpr std::uint32_t item_purchase = fnv1a::hash_32("item_purchase");
}
std::uint32_t hash_event_name(const char* str) {
	uint32_t hash = fnv1a::val_32_const;
	while (*str) {
		hash = (hash ^ static_cast<uint8_t>(*str)) * fnv1a::prime_32_const;
		str++;
	}
	return hash;
}

namespace knife_db {
	struct entry { std::uint16_t def; const char* full; };
	inline constexpr entry kSkinKnives[] = {
		{500, "weapon_bayonet"},                {503, "weapon_knife_css"},
		{505, "weapon_knife_flip"},             {506, "weapon_knife_gut"},
		{507, "weapon_knife_karambit"},         {508, "weapon_knife_m9_bayonet"},
		{509, "weapon_knife_tactical"},         {512, "weapon_knife_falchion"},
		{514, "weapon_knife_survival_bowie"},   {515, "weapon_knife_butterfly"},
		{516, "weapon_knife_push"},             {517, "weapon_knife_cord"},
		{518, "weapon_knife_canis"},            {519, "weapon_knife_ursus"},
		{520, "weapon_knife_gypsy_jackknife"},  {521, "weapon_knife_outdoor"},
		{522, "weapon_knife_stiletto"},         {523, "weapon_knife_widowmaker"},
		{525, "weapon_knife_skeleton"},         {526, "weapon_knife_kukri"},
	};

	inline const char* lookup(std::uint16_t def) {
		for (auto& e : kSkinKnives)
			if (e.def == def) return e.full;
		return nullptr;
	}

	inline bool matches(const char* name) {
		if (!name || !*name) return false;
		if (std::strncmp(name, "weapon_", 7) == 0) name += 7;
		if (std::strcmp(name, "knife") == 0 || std::strcmp(name, "knife_t") == 0)
			return true;
		for (auto& e : kSkinKnives)
			if (std::strcmp(name, e.full + 7) == 0) return true;
		return false;
	}
}

static const char* get_knife_weapon_name(int knife_index) {
	if (knife_index <= 0
		|| !ttapp::client::item_schema().is_initialized()
		|| knife_index >= (int)ttapp::client::item_schema().knives.size())
		return nullptr;
	return knife_db::lookup(ttapp::client::item_schema().knives[knife_index].definition_index);
}
void on_level_init()
{
	if (ttapp::client::config().knife_changer.m_enabled || ttapp::client::config().skin_changer.m_enabled) {
		ttapp::client::skin_changer().should_update = true;
	}

	if (ttapp::client::config().glove_changer.m_enabled) {
		ttapp::client::glove_changer().should_update = true;
	}

	if (ttapp::client::config().agent_changer.m_enabled) {
		ttapp::client::agent_changer().should_update = true;
	}
}

void on_game_event(std::uint32_t event_hash, void* game_event)
{
	if (event_hash == event_hashes::round_start || event_hash == event_hashes::item_purchase) {
		ttapp::client::skin_changer().should_update |= ttapp::client::config().knife_changer.m_enabled || ttapp::client::config().skin_changer.m_enabled;
		ttapp::client::glove_changer().should_update |= ttapp::client::config().glove_changer.m_enabled;
		ttapp::client::agent_changer().should_update |= ttapp::client::config().agent_changer.m_enabled;
		return;
	}

	if (event_hash != event_hashes::player_death || !ttapp::client::config().knife_changer.m_enabled)
		return;

	if (!ttapp::client::frame().m_local_controller || !i_game_event::get_player_controller || !i_game_event::get_string || !i_game_event::set_string)
		return;

	i_game_event::CUtlStringToken attacker_token("attacker");
	attacker_token.pad = 0xFFFFFFFF;

	void* attacker_controller = i_game_event::get_player_controller(game_event, &attacker_token);

	if (attacker_controller != ttapp::client::frame().m_local_controller)
		return;

	i_game_event::CUtlStringToken weapon_token("weapon");
	weapon_token.pad = 0xFFFFFFFF;

	const char* weapon_name = i_game_event::get_string(game_event, &weapon_token, nullptr);

	if (!valid_ptr(weapon_name))
		return;

	const bool is_knife = knife_db::matches(weapon_name);

	if (is_knife && ttapp::client::config().knife_changer.m_knife != 0) {
		const char* new_weapon_name = get_knife_weapon_name(ttapp::client::config().knife_changer.m_knife);
		if (new_weapon_name) {
			i_game_event::CUtlStringToken set_token("weapon");
			set_token.pad = 0xFFFFFFFF;
			i_game_event::set_string(game_event, &set_token, new_weapon_name, 0);
		}
	}
}

} // namespace skin_events
