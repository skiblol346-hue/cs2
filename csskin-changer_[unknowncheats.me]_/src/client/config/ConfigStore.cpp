#include "platform/ClientCore.hpp"
#include "runtime/Runtime.hpp"
#include "config/ConfigStore.hpp"
#include "features/skins/SkinChanger.hpp"
#include "features/skins/GloveChanger.hpp"
#include <iomanip>

void ConfigStore::push_item(bool* pointer, const std::string& category, const std::string& name, bool default_value) {
    m_booleans.push_back({ pointer, category, name, default_value });
}

void ConfigStore::push_item(int* pointer, const std::string& category, const std::string& name, int default_value) {
    m_ints.push_back({ pointer, category, name, default_value });
}

void ConfigStore::push_item(float* pointer, const std::string& category, const std::string& name, float default_value) {
    m_floats.push_back({ pointer, category, name, default_value });
}

void ConfigStore::push_item(std::string* pointer, const std::string& category, const std::string& name, const std::string& default_value) {
    m_strings.push_back({ pointer, category, name, default_value });
}

void ConfigStore::push_char_array(char* pointer, size_t size, const std::string& category, const std::string& name) {
    m_char_arrays.push_back({ pointer, size, category, name });
}

void ConfigStore::setup_values() {

    std::filesystem::create_directories(m_config_path);

    push_item(&ttapp::client::config().knife_changer.m_enabled, "knife_changer", "enabled", false);
    push_item(&ttapp::client::config().knife_changer.m_knife, "knife_changer", "knife", 0);
    push_item(&ttapp::client::config().knife_changer.m_paint_kit, "knife_changer", "paint_kit", 0);
    push_item(&ttapp::client::config().knife_changer.m_wear, "knife_changer", "wear", 0.0001f);
    push_item(&ttapp::client::config().knife_changer.m_seed, "knife_changer", "seed", 0);
    push_item(&ttapp::client::config().knife_changer.m_stattrak_enabled, "knife_changer", "stattrak_enabled", false);
    push_item(&ttapp::client::config().knife_changer.m_stattrak_count, "knife_changer", "stattrak_count", 0);
    push_char_array(ttapp::client::config().knife_changer.m_custom_name, sizeof(ttapp::client::config().knife_changer.m_custom_name), "knife_changer", "custom_name");
    push_item(&ttapp::client::config().knife_changer.m_custom_colors_enabled, "knife_changer", "custom_colors_enabled", false);
    for (int color = 0; color < 4; ++color) {
        const char* channels[] = { "r", "g", "b", "a" };
        for (int channel = 0; channel < 4; ++channel) {
            push_item(&ttapp::client::config().knife_changer.m_custom_colors[color][channel],
                "knife_changer",
                "custom_color_" + std::to_string(color) + "_" + channels[channel],
                1.0f);
        }
    }

    push_item(&ttapp::client::config().glove_changer.m_enabled, "glove_changer", "enabled", false);
    push_item(&ttapp::client::config().glove_changer.m_glove, "glove_changer", "glove", 0);
    push_item(&ttapp::client::config().glove_changer.m_paint_kit, "glove_changer", "paint_kit", 0);
    push_item(&ttapp::client::config().glove_changer.m_wear, "glove_changer", "wear", 0.0001f);
    push_item(&ttapp::client::config().glove_changer.m_seed, "glove_changer", "seed", 0);

    push_item(&ttapp::client::config().agent_changer.m_enabled, "agent_changer", "enabled", false);
    push_item(&ttapp::client::config().agent_changer.m_agent_ct, "agent_changer", "agent_ct", 0);
    push_item(&ttapp::client::config().agent_changer.m_agent_t, "agent_changer", "agent_t", 0);

    push_item(&ttapp::client::config().skin_changer.m_enabled, "skin_changer", "enabled", false);
    push_item(&ttapp::client::config().skin_changer.m_selected_weapon, "skin_changer", "selected_weapon", 0);
    for (int ci = 0; ci < 7; ++ci)
        push_item(&ttapp::client::config().skin_changer.weapon_cat_open[ci], "skin_changer",
                  "weapon_cat_open_" + std::to_string(ci), true);
    push_item(&ttapp::client::config().misc.m_quick_stop, "misc", "quick_stop", false);
    push_item(&ttapp::client::config().misc.m_auto_accept, "misc", "auto_accept", false);
    push_item(&ttapp::client::config().misc.m_auto_accept_status_text, "misc", "auto_accept_status_text", true);
    push_item(&ttapp::client::config().misc.m_spectator_list, "misc", "spectator_list", false);
    push_item(&ttapp::client::config().misc.m_spectator_list_x, "misc", "spectator_list_x", 20.0f);
    push_item(&ttapp::client::config().misc.m_spectator_list_y, "misc", "spectator_list_y", 220.0f);

    for (int i = 0; i < 100; i++) {
        std::string prefix = "weapon_" + std::to_string(i);
        push_item(&ttapp::client::config().skin_changer.weapon_skins[i].paint_kit, "skin_changer", prefix + "_paint_kit", 0);
        push_item(&ttapp::client::config().skin_changer.weapon_skins[i].wear, "skin_changer", prefix + "_wear", 0.0001f);
        push_item(&ttapp::client::config().skin_changer.weapon_skins[i].seed, "skin_changer", prefix + "_seed", 0);
        push_item(&ttapp::client::config().skin_changer.weapon_skins[i].stattrak_enabled, "skin_changer", prefix + "_stattrak_enabled", false);
        push_item(&ttapp::client::config().skin_changer.weapon_skins[i].stattrak_count, "skin_changer", prefix + "_stattrak_count", 0);
        push_char_array(ttapp::client::config().skin_changer.weapon_skins[i].custom_name, sizeof(ttapp::client::config().skin_changer.weapon_skins[i].custom_name), "skin_changer", prefix + "_custom_name");
        push_item(&ttapp::client::config().skin_changer.weapon_skins[i].custom_colors_enabled, "skin_changer", prefix + "_custom_colors_enabled", false);
        for (int color = 0; color < 4; ++color) {
            const char* channels[] = { "r", "g", "b", "a" };
            for (int channel = 0; channel < 4; ++channel) {
                push_item(&ttapp::client::config().skin_changer.weapon_skins[i].custom_colors[color][channel],
                    "skin_changer",
                    prefix + "_custom_color_" + std::to_string(color) + "_" + channels[channel],
                    1.0f);
            }
        }
        for (int slot = 0; slot < Config::skin_changer_t::sticker_slot_count; ++slot) {
            std::string sticker_prefix = prefix + "_sticker_" + std::to_string(slot);
            auto& sticker = ttapp::client::config().skin_changer.weapon_skins[i].stickers[slot];
            push_item(&sticker.id, "skin_changer", sticker_prefix + "_id", 0);
            push_item(&sticker.wear, "skin_changer", sticker_prefix + "_wear", 0.0f);
            push_item(&sticker.scale, "skin_changer", sticker_prefix + "_scale", 1.0f);
            push_item(&sticker.rotation, "skin_changer", sticker_prefix + "_rotation", 0.0f);
            push_item(&sticker.offset_x, "skin_changer", sticker_prefix + "_offset_x", 0.0f);
            push_item(&sticker.offset_y, "skin_changer", sticker_prefix + "_offset_y", 0.0f);
        }

        auto& keychain = ttapp::client::config().skin_changer.weapon_skins[i].keychain;
        push_item(&keychain.id, "skin_changer", prefix + "_keychain_id", 0);
        push_item(&keychain.offset_x, "skin_changer", prefix + "_keychain_offset_x", 0.0f);
        push_item(&keychain.offset_y, "skin_changer", prefix + "_keychain_offset_y", 0.0f);
        push_item(&keychain.offset_z, "skin_changer", prefix + "_keychain_offset_z", 0.0f);
        push_item(&keychain.seed, "skin_changer", prefix + "_keychain_seed", 0);
    }

    refresh();
}

void save_last_state(const std::string& config_name)
{
    std::filesystem::create_directories("C:\\ttapp\\");
    const char* state_path = "C:\\ttapp\\state.json";

    nlohmann::json state;
    state["last_config"] = config_name;
    state["menu_key"] = ttapp::client::config().misc.m_menu_key;

    std::ofstream file(state_path);
    if (file.is_open()) {
        file << state.dump(4);
        file.close();
    }
}

void ConfigStore::save(const std::string& name)
{
    if (name.empty())
        return;


    std::filesystem::create_directories(m_config_path);

    std::string path = m_config_path + name + ".cfg";

    json data = json::object();

    auto ensure_category = [&](const std::string& category)
        {
            if (data.find(category) == data.end())
                data[category] = json::object();
        };

    for (auto& item : m_booleans)
    {
        ensure_category(item.category);
        data[item.category][item.name] = *item.pointer;
    }

    for (auto& item : m_ints)
    {
        ensure_category(item.category);
        data[item.category][item.name] = *item.pointer;
    }

    for (auto& item : m_floats)
    {
        ensure_category(item.category);
        data[item.category][item.name] = *item.pointer;
    }

    for (auto& item : m_strings)
    {
        ensure_category(item.category);
        data[item.category][item.name] = *item.pointer;
    }

    for (auto& item : m_char_arrays)
    {
        ensure_category(item.category);
        data[item.category][item.name] = std::string(item.pointer);
    }

    std::ofstream file(path);
    if (!file.is_open())
        return;

    file << std::setw(4) << data << std::endl;
    const bool wrote_file = file.good();
    file.close();

    if (!wrote_file)
        return;

    m_selected_config = name;
    save_last_state(name);
    printf("[Config] Save called on instance: %p\n", this);
    refresh();
}

void ConfigStore::load(const std::string& name)
{
    if (name.empty())
        return;

    std::string path = m_config_path + name + ".cfg";

    if (!std::filesystem::exists(path))
        return;

    std::ifstream file(path);
    if (!file.is_open())
        return;

    json data;
    try
    {
        file >> data;
    }
    catch (...)
    {
        LOG_ERROR(xorstr_("[config] failed to parse: %s"), name.c_str());
        file.close();
        return;
    }
    file.close();

    auto has_key = [&](const std::string& category, const std::string& key) -> bool
        {
            return data.find(category) != data.end() && data[category].find(key) != data[category].end();
        };

    for (auto& item : m_booleans)
    {
        if (has_key(item.category, item.name))
            *item.pointer = data[item.category][item.name].get<bool>();
    }

    for (auto& item : m_ints)
    {
        if (has_key(item.category, item.name))
            *item.pointer = data[item.category][item.name].get<int>();
    }

    for (auto& item : m_floats)
    {
        if (has_key(item.category, item.name))
            *item.pointer = data[item.category][item.name].get<float>();
    }

    for (auto& item : m_strings)
    {
        if (has_key(item.category, item.name))
            *item.pointer = data[item.category][item.name].get<std::string>();
    }

    for (auto& item : m_char_arrays)
    {
        if (has_key(item.category, item.name))
        {
            std::string value = data[item.category][item.name].get<std::string>();
            strncpy_s(item.pointer, item.size, value.c_str(), _TRUNCATE);
        }
    }

    ttapp::client::skin_changer().should_update = true;
    ttapp::client::glove_changer().should_update = true;
    ttapp::client::agent_changer().should_update = true;
    m_selected_config = name;
    save_last_state(name);
}

void ConfigStore::remove(const std::string& name)
{
    if (name.empty())
        return;

    std::string path = m_config_path + name + ".cfg";

    if (std::filesystem::exists(path))
        std::filesystem::remove(path);

    if (m_selected_config == name) {
        m_selected_config.clear();
        save_last_state("");
    }

    refresh();
}

void ConfigStore::reset()
{

    for (auto& item : m_booleans)
        *item.pointer = item.default_value;

    for (auto& item : m_ints)
        *item.pointer = item.default_value;

    for (auto& item : m_floats)
        *item.pointer = item.default_value;

    for (auto& item : m_strings)
        *item.pointer = item.default_value;

    for (auto& item : m_char_arrays)
        memset(item.pointer, 0, item.size);
}

void ConfigStore::refresh()
{
    m_config_files.clear();

    if (!std::filesystem::exists(m_config_path))
        return;

    for (const auto& entry : std::filesystem::directory_iterator(m_config_path))
    {
        if (!entry.is_regular_file())
            continue;

        if (entry.path().extension() != ".cfg")
            continue;

        std::string filename = entry.path().stem().string();
        m_config_files.push_back(filename);
    }
}

void ConfigStore::auto_load()
{
  

    const char* state_path = "C:\\ttapp\\state.json";
    if (!std::filesystem::exists(state_path)) return;

    try {
        std::ifstream file(state_path);
        nlohmann::json state;
        file >> state;

        /*if (state.contains("menu_key")) {
            ttapp::client::config().misc.m_menu_key = state["menu_key"].get<int>();
        }*/

        if (!state.contains("last_config") || !state["last_config"].is_string())
            return;

        std::string last_cfg = state["last_config"].get<std::string>();
        if (last_cfg.empty())
            return;

        load(last_cfg);
    }
    catch (...) {
       
    }
}

void ConfigStore::auto_load_key()
{


    const char* state_path = "C:\\ttapp\\state.json";
    if (!std::filesystem::exists(state_path)) return;

    try {
        std::ifstream file(state_path);
        nlohmann::json state;
        file >> state;

        if (state.contains("menu_key")) {
            ttapp::client::config().misc.m_menu_key = state["menu_key"].get<int>();
        }

        
    }
    catch (...) {

    }
}

void ConfigStore::auto_save()
{
    save(m_default_config);
}
