#pragma once

class Config;
class ConfigStore;
class ModuleRegistry;
class InterfaceRegistry;
class PatternScanner;
class SchemaRegistry;
class ItemSchema;
class QuickStop;
class AutoAccept;
class SpectatorList;
class SkinChanger;
class GloveChanger;
class AgentChanger;
class Dx11Renderer;
class Menu;
class HookManager;
struct GameFrameContext;

namespace ttapp::client {

Config& config();
ConfigStore& config_store();
ModuleRegistry& modules();
InterfaceRegistry& interfaces();
PatternScanner& patterns();
SchemaRegistry& schema();
ItemSchema& item_schema();
QuickStop& quick_stop();
AutoAccept& auto_accept();
SpectatorList& spectator_list();
SkinChanger& skin_changer();
GloveChanger& glove_changer();
AgentChanger& agent_changer();
Dx11Renderer& renderer();
Menu& menu();
HookManager& hooks();
GameFrameContext& frame();

}
