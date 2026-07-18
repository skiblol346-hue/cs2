#pragma once

#include <cstdint>

namespace skin_events {

std::uint32_t hash_event_name(const char* str);
void on_level_init();
void on_game_event(std::uint32_t event_hash, void* game_event);

} // namespace skin_events
