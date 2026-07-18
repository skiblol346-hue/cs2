#pragma once

#include "config/Config.hpp"
#include "config/ConfigStore.hpp"
#include "features/hud/SpectatorList.hpp"
#include "features/matchmaking/AutoAccept.hpp"
#include "features/movement/QuickStop.hpp"
#include "features/skins/GloveChanger.hpp"
#include "features/skins/SkinChanger.hpp"
#include "features/skins/AgentChanger.hpp"
#include "game/InterfaceRegistry.hpp"
#include "game/ModuleRegistry.hpp"
#include "game/SchemaRegistry.hpp"
#include "game/items/ItemSchema.hpp"
#include "hooks/HookManager.hpp"
#include "memory/PatternScanner.hpp"
#include "render/Dx11Renderer.hpp"
#include "runtime/GameFrameContext.hpp"
#include "ui/Menu.hpp"

struct Services {
    Config config;
    ConfigStore config_store;
    ModuleRegistry modules;
    InterfaceRegistry interfaces;
    PatternScanner patterns;
    SchemaRegistry schema;
    ItemSchema item_schema;
    QuickStop quick_stop;
    AutoAccept auto_accept;
    SpectatorList spectator_list;
    SkinChanger skin_changer;
    GloveChanger glove_changer;
    AgentChanger agent_changer;
    Dx11Renderer renderer;
    Menu menu;
    HookManager hooks;
    GameFrameContext frame;
};
