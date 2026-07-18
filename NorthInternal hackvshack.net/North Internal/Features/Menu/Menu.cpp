#include <windows.h>
#include <cstring>
#include "Menu.hpp"
#include "UI.hpp"
#include "Icons.h"
#include "font_data.h"
#include "../SkinChanger/SkinChanger.hpp"
#include "../Configs/Configs.hpp"
#include "../../Dependencies/ImGui/imgui.h"
#include "../../Dependencies/ImGui/imgui_internal.h"

CMenu g_Menu;

bool p_enable = true, p_wall = false, p_name = false, p_box = false, p_health = false, p_armor = false, p_ammo = false, p_weapon = false, p_skeleton = false;
int box_pos = 0, ammo_pos = 0;
float box_col[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float ammo_col[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float name_col[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float skel_col[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
bool f_bomb = false, f_hp = false, f_ammo = false, f_rel = false, f_scope = false, f_flash = false, f_dist = false, f_kit = false;

bool lb_enable = false, lb_draw_fov = false, lb_dynamic_fov = false;
float lb_amount = 0.0f, lb_multiplier = 4.0f, lb_smooth_pitch = 0.0f, lb_smooth_yaw = 0.0f;
float lb_target_switch = 250.0f, lb_min_hit_chance = 0.0f, lb_min_damage = 0.0f;
bool hg_selections[6] = {};
const char* hg_items[] = { "Head", "Neck", "Upper chest", "Lower chest", "Stomach", "Legs" };
bool lb_only_body = false, lb_prefer_head = false, lb_prefer_body = false, lb_body_lethal = false;
bool lb_af_enable = false, lb_af_auto_stop = false, lb_pro_spread = false;
float lb_delay_shots = 0.0f, lb_af_min_hit = 0.0f, lb_ghost_time = 0.0f;

bool vm_hand_shakes = true, vm_instant_flip = false, vm_knife_other = false;
float vm_fov = 60.0f;
bool vm_custom_origin = false;
float vm_origin_x = 0.0f, vm_origin_y = 0.0f, vm_origin_z = 0.0f;
bool vm_custom_angles = false;
float vm_pitch = 0.0f, vm_yaw = 0.0f, vm_roll = 0.0f;

bool cam_free_look = false;
float cam_aspect_ratio = 0.0f, cam_fov = 0.0f, cam_zoom_fov1 = 0.0f, cam_zoom_fov2 = 0.0f;

bool tp_enable = false;
float tp_max_dist = 150.0f;

void CMenu::Toggle() { bOpened = !bOpened; }
bool CMenu::HandleInput() const { return bOpened; }

void CMenu::LoadFonts()
{
    ImGuiIO& io = ImGui::GetIO();

    char verdana_path[260], verdanab_path[260];
    GetEnvironmentVariableA("WINDIR", verdana_path, sizeof(verdana_path));
    GetEnvironmentVariableA("WINDIR", verdanab_path, sizeof(verdanab_path));

    strcat_s(verdana_path, sizeof(verdana_path), "\\Fonts\\verdana.ttf");
    strcat_s(verdanab_path, sizeof(verdanab_path), "\\Fonts\\verdanab.ttf");

    ImFontConfig config;
    config.OversampleH = 3;
    config.OversampleV = 1;

    LogoFont = io.Fonts->AddFontFromFileTTF(verdanab_path, 22.0f, &config);
    if (!LogoFont)
        LogoFont = io.Fonts->AddFontDefault();

    io.FontDefault = io.Fonts->AddFontFromFileTTF(verdana_path, 15.0f, &config);
    if (!io.FontDefault)
        io.FontDefault = io.Fonts->AddFontDefault();

    ImFontConfig espConfig;
    espConfig.OversampleH = 2;
    espConfig.OversampleV = 1;
    ESPFont = io.Fonts->AddFontFromFileTTF(verdana_path, 11.0f, &espConfig);
    if (!ESPFont)
        ESPFont = io.Fonts->AddFontDefault();

    static const ImWchar icon_ranges[] = { 0xf000, 0xf9ff, 0 };
    ImFontConfig icons_config;
    icons_config.PixelSnapH = true;
    icons_config.OversampleH = 3;
    icons_config.OversampleV = 1;
    IconFont = io.Fonts->AddFontFromMemoryTTF((void*)fa_solid_900_data, sizeof(fa_solid_900_data), 18.0f, &icons_config, icon_ranges);

    io.Fonts->Build();
}

void CMenu::Render()
{
    if (!bOpened) return;

    auto CenteredText = [](const char* text) {
        float avail = ImGui::GetContentRegionAvail().x;
        float textW = ImGui::CalcTextSize(text).x;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (avail - textW) * 0.5f);
        ImGui::TextDisabled("%s", text);
        };

    auto BindButton = [](const char* id) {
        ImGui::PushID(id);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.20f, 0.20f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);
        ImGui::Button("##bind", ImVec2(22, 16));
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);
        ImGui::PopID();
        };

    ImGui::SetNextWindowSize(ImVec2(780, 620));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.08f, 1.00f));

    if (ImGui::Begin("##MainFrame", &bOpened, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar))
    {
        ImVec2 p = ImGui::GetWindowPos();
        float width = ImGui::GetWindowWidth();
        ImDrawList* fg_draw_list = ImGui::GetForegroundDrawList();

        static float rainbow_hue = 0.0f;
        rainbow_hue += ImGui::GetIO().DeltaTime * 0.2f;
        if (rainbow_hue > 1.0f) rainbow_hue -= 1.0f;

        const int segments = 24;
        const float seg_w = width / segments;
        for (int i = 0; i < segments; i++)
        {
            float hue_l = rainbow_hue + (float)i / segments;
            float hue_r = rainbow_hue + (float)(i + 1) / segments;
            if (hue_l > 1.0f) hue_l -= 1.0f;
            if (hue_r > 1.0f) hue_r -= 1.0f;
            ImU32 col_l = ImColor::HSV(hue_l, 1.0f, 1.0f);
            ImU32 col_r = ImColor::HSV(hue_r, 1.0f, 1.0f);
            fg_draw_list->AddRectFilledMultiColor(
                ImVec2(p.x + i * seg_w, p.y),
                ImVec2(p.x + (i + 1) * seg_w, p.y + 3),
                col_l, col_r, col_r, col_l);
        }

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.10f, 0.10f, 0.10f, 1.00f));
        if (ImGui::BeginChild("##LeftSideBar", ImVec2(160, 620), false))
        {
            if (LogoFont) ImGui::PushFont(LogoFont);
            ImVec2 textSize = ImGui::CalcTextSize("MemeSense");
            ImGui::SetCursorPos(ImVec2((160.0f - textSize.x) * 0.5f, 22));
            ImGui::TextColored(ImVec4(1.0f, 0.20f, 0.20f, 1.00f), "Meme");
            ImGui::SameLine(0, 0);
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.00f), "Sense");
            if (LogoFont) ImGui::PopFont();

            ImGui::SetCursorPosY(60);

            auto AddTab = [this](int id, const char* icon, const char* label) {
                ImVec2 pos = ImGui::GetCursorScreenPos();
                ImVec2 size = ImVec2(160, 40);
                bool is_active = (iActiveTab == id);
                ImDrawList* draw = ImGui::GetWindowDrawList();

                if (is_active)
                {
                    draw->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), ImColor(28, 28, 28));
                    draw->AddRectFilled(pos, ImVec2(pos.x + 3, pos.y + size.y), ImColor(255, 50, 50));
                }

                if (IconFont) ImGui::PushFont(IconFont);
                draw->AddText(IconFont, 18.0f, ImVec2(pos.x + 12, pos.y + 10), is_active ? ImColor(255, 50, 50) : ImColor(180, 180, 180), icon);
                if (IconFont) ImGui::PopFont();

                draw->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(pos.x + 38, pos.y + 11), is_active ? ImColor(255, 255, 255) : ImColor(180, 180, 180), label);

                ImGui::SetCursorScreenPos(pos);
                if (ImGui::InvisibleButton(label, size)) iActiveTab = id;
                ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + size.y));
                };

            AddTab(0, ICON_FA_CROSSHAIRS, "LegitBot");
            AddTab(1, ICON_FA_MOUSE, "Aim Assist");
            AddTab(2, ICON_FA_USERS, "Players");
            AddTab(3, ICON_FA_SKULL, "Chams");
            AddTab(4, ICON_FA_TAG, "Items");
            AddTab(5, ICON_FA_FIRE, "Visuals");
            AddTab(6, ICON_FA_CLOUD, "World");
            AddTab(7, ICON_FA_EYE, "View");
            AddTab(8, ICON_FA_BARS, "Indicators");
            AddTab(9, ICON_FA_EDIT, "Miscellaneous");
            AddTab(10, ICON_FA_PAINT_BRUSH, "Inventory");
            AddTab(11, ICON_FA_FOLDER, "Configs");
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGui::SameLine(160);
        if (ImGui::BeginChild("##MainContent", ImVec2(620, 620), false))
        {
            if (iActiveTab == 0)
            {
                ImGui::SetCursorPos(ImVec2(20, 20));

                ImGui::BeginChild("##LBLeftCol", ImVec2(280, 580), false);
                {
                    CenteredText("General");
                    ImGui::Spacing();

                    CustomUI::Checkbox("Enable", &lb_enable);
                    CustomUI::Checkbox("Draw FOV", &lb_draw_fov);

                    ImGui::Spacing();
                    ImGui::Spacing();

                    CenteredText("Aim");
                    ImGui::Spacing();

                    CustomUI::Checkbox("Dynamic FOV", &lb_dynamic_fov);
                    ImGui::Spacing();

                    CustomUI::SliderFloat("Amount", &lb_amount, 0.0f, 180.0f, "%.3f" "\xc2\xb0");
                    ImGui::Spacing();
                    CustomUI::SliderFloat("Multiplier", &lb_multiplier, 0.0f, 10.0f, "%.2f");
                    ImGui::Spacing();
                    CustomUI::SliderFloat("Smooth pitch", &lb_smooth_pitch, 0.0f, 100.0f, "%.0f", "disabled");
                    ImGui::Spacing();
                    CustomUI::SliderFloat("Smooth yaw", &lb_smooth_yaw, 0.0f, 100.0f, "%.0f", "disabled");
                    ImGui::Spacing();
                    CustomUI::SliderFloat("Target switch delay", &lb_target_switch, 0.0f, 1000.0f, "%.0f ms");
                    ImGui::Spacing();
                    CustomUI::SliderFloat("Minimum hit chance", &lb_min_hit_chance, 0.0f, 100.0f, "%.0f%%");
                    ImGui::Spacing();
                    CustomUI::SliderFloat("Minimum damage", &lb_min_damage, 0.0f, 120.0f, "%.0f HP");
                }
                ImGui::EndChild();

                ImGui::SameLine(320);

                ImGui::BeginChild("##LBRightCol", ImVec2(280, 580), false);
                {
                    CenteredText("Hit groups");
                    ImGui::Spacing();

                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Selection");
                    ImGui::SameLine(80);
                    CustomUI::MultiCombo("##HitGroups", hg_selections, hg_items, 6);
                    ImGui::Spacing();

                    CustomUI::Checkbox("Only body aim", &lb_only_body);
                    CustomUI::Checkbox("Prefer head aim", &lb_prefer_head);
                    CustomUI::Checkbox("Prefer body aim", &lb_prefer_body);
                    CustomUI::Checkbox("Body aim if lethal", &lb_body_lethal);

                    ImGui::Spacing();
                    ImGui::Spacing();

                    CenteredText("Auto fire");
                    ImGui::Spacing();

                    CustomUI::Checkbox("Enable", &lb_af_enable);
                    CustomUI::Checkbox("Auto stop", &lb_af_auto_stop);
                    CustomUI::Checkbox("Pro spread", &lb_pro_spread);
                    ImGui::Spacing();

                    CustomUI::SliderFloat("Delay between shots", &lb_delay_shots, 0.0f, 1000.0f, "%.0f ms", "disabled");
                    ImGui::Spacing();
                    CustomUI::SliderFloat("Minimum hit chance", &lb_af_min_hit, 0.0f, 100.0f, "%.0f%%");
                    ImGui::Spacing();
                    CustomUI::SliderFloat("Ghost time", &lb_ghost_time, 0.0f, 1000.0f, "%.0f ms", "disabled");
                }
                ImGui::EndChild();
            }

            if (iActiveTab == 2)
            {
                ImGui::SetCursorPos(ImVec2(20, 20));

                ImGui::BeginChild("##GeneralSection", ImVec2(280, 550), false);
                {
                    CenteredText("General");
                    ImGui::Spacing();

                    const char* ammo_items[] = { "Bottom", "Left", "Right" };

                    CustomUI::Checkbox("Enable", &p_enable);
                    CustomUI::Checkbox("Through wall", &p_wall);

                    ImGui::BeginGroup();
                    CustomUI::Checkbox("Nickname", &p_name);
                    ImGui::SameLine(135);
                    CustomUI::ColorEdit("##NameCol", name_col);
                    ImGui::EndGroup();

                    ImGui::BeginGroup();
                    CustomUI::Checkbox("Box", &p_box);
                    ImGui::SameLine(135);
                    CustomUI::ColorEdit("##BoxCol", box_col);
                    ImGui::EndGroup();

                    CustomUI::Checkbox("Health bar", &p_health);
                    CustomUI::Checkbox("Armor bar", &p_armor);

                    ImGui::BeginGroup();
                    CustomUI::Checkbox("Ammo bar", &p_ammo);
                    ImGui::SameLine(135);
                    CustomUI::ColorEdit("##AmmoCol", ammo_col);
                    ImGui::SameLine(160);
                    CustomUI::Combo("##AmmoSide", &ammo_pos, ammo_items, 3);
                    ImGui::EndGroup();

                    CustomUI::Checkbox("Weapon", &p_weapon);

                    ImGui::BeginGroup();
                    CustomUI::Checkbox("Skeleton", &p_skeleton);
                    ImGui::SameLine(135);
                    CustomUI::ColorEdit("##SkelCol", skel_col);
                    ImGui::EndGroup();
                }
                ImGui::EndChild();

                ImGui::SameLine(320);

                ImGui::BeginChild("##FlagsSection", ImVec2(280, 550), false);
                {
                    CenteredText("Flags");
                    ImGui::Spacing();

                    CustomUI::Checkbox("Bomb", &f_bomb);
                    CustomUI::Checkbox("Health", &f_hp);
                    CustomUI::Checkbox("Ammo", &f_ammo);
                    CustomUI::Checkbox("Reload", &f_rel);
                    CustomUI::Checkbox("Scoped", &f_scope);
                    CustomUI::Checkbox("Flashed", &f_flash);
                    CustomUI::Checkbox("Distance", &f_dist);
                    CustomUI::Checkbox("Defuse kit", &f_kit);
                }
                ImGui::EndChild();
            }

            if (iActiveTab == 7)
            {
                ImGui::SetCursorPos(ImVec2(20, 20));

                ImGui::BeginChild("##ViewLeftCol", ImVec2(280, 580), false);
                {
                    CenteredText("View model");
                    ImGui::Spacing();

                    CustomUI::Checkbox("Hand shakes", &vm_hand_shakes);
                    CustomUI::Checkbox("Instant flip hands", &vm_instant_flip);
                    CustomUI::Checkbox("Knife in other hand", &vm_knife_other);

                    ImGui::Spacing();

                    CustomUI::SliderFloat("Field of view", &vm_fov, 0.0f, 180.0f, "%.0f\xc2\xb0");

                    ImGui::Spacing();

                    CustomUI::Checkbox("Custom origin", &vm_custom_origin);

                    ImGui::Spacing();

                    CustomUI::SliderFloat("X", &vm_origin_x, -10.0f, 10.0f, "%.3f");
                    ImGui::Spacing();
                    CustomUI::SliderFloat("Y", &vm_origin_y, -10.0f, 10.0f, "%.3f");
                    ImGui::Spacing();
                    CustomUI::SliderFloat("Z", &vm_origin_z, -10.0f, 10.0f, "%.3f");

                    ImGui::Spacing();

                    CustomUI::Checkbox("Custom angles", &vm_custom_angles);

                    ImGui::Spacing();

                    CustomUI::SliderFloat("Pitch", &vm_pitch, -180.0f, 180.0f, "%.3f\xc2\xb0");
                    ImGui::Spacing();
                    CustomUI::SliderFloat("Yaw", &vm_yaw, -180.0f, 180.0f, "%.3f\xc2\xb0");
                    ImGui::Spacing();
                    CustomUI::SliderFloat("Roll", &vm_roll, -180.0f, 180.0f, "%.3f\xc2\xb0");
                }
                ImGui::EndChild();

                ImGui::SameLine(320);

                ImGui::BeginChild("##ViewRightCol", ImVec2(280, 580), false);
                {
                    CenteredText("Camera");
                    ImGui::Spacing();

                    ImGui::BeginGroup();
                    CustomUI::Checkbox("Free look", &cam_free_look);
                    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
                    BindButton("##FreeLookBind");
                    ImGui::EndGroup();

                    ImGui::Spacing();

                    CustomUI::SliderFloat("Aspect ratio", &cam_aspect_ratio, 0.0f, 4.0f, "%.2f", "auto");
                    ImGui::Spacing();
                    CustomUI::SliderFloat("Field of view", &cam_fov, 0.0f, 180.0f, "%.0f\xc2\xb0", "auto");
                    ImGui::Spacing();
                    CustomUI::SliderFloat("Zoom field of view 1", &cam_zoom_fov1, 0.0f, 180.0f, "%.0f\xc2\xb0", "auto");
                    ImGui::Spacing();
                    CustomUI::SliderFloat("Zoom field of view 2", &cam_zoom_fov2, 0.0f, 180.0f, "%.0f\xc2\xb0", "auto");

                    ImGui::Spacing();
                    ImGui::Spacing();

                    CenteredText("Third-person");
                    ImGui::Spacing();

                    ImGui::BeginGroup();
                    CustomUI::Checkbox("Enable", &tp_enable);
                    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
                    BindButton("##TpEnableBind");
                    ImGui::EndGroup();

                    ImGui::Spacing();

                    CustomUI::SliderFloat("Max distance", &tp_max_dist, 0.0f, 500.0f, "%.0fu");
                }
                ImGui::EndChild();
            }

            if (iActiveTab == 10)
            {
                g_SkinChanger.Render();
            }

            if (iActiveTab == 11)
            {
                g_Configs.Render();
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
}