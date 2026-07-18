#include "Configs.hpp"
#include "../Menu/Menu.hpp"
#include "../Menu/Icons.h"
#include "../../Dependencies/ImGui/imgui.h"
#include "../../Dependencies/ImGui/imgui_internal.h"
#include <windows.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;

CConfigSystem g_Configs;

static std::string FileModTime(const std::string& path)
{
    WIN32_FILE_ATTRIBUTE_DATA fa;
    if (!GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &fa)) return "Unknown";
    FILETIME ft = fa.ftLastWriteTime;
    FileTimeToLocalFileTime(&ft, &ft);
    SYSTEMTIME st;
    FileTimeToSystemTime(&ft, &st);
    static const char* months[] = {
        "January","February","March","April","May","June",
        "July","August","September","October","November","December"
    };
    char buf[64];
    snprintf(buf, sizeof(buf), "%s %02d, %d %02d:%02d:%02d",
        months[st.wMonth - 1], st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond);
    return buf;
}

std::string CConfigSystem::GetDir() { return "C:\\North\\Configs\\"; }

std::string CConfigSystem::GetTimestamp() const
{
    SYSTEMTIME st;
    GetLocalTime(&st);
    static const char* months[] = {
        "January","February","March","April","May","June",
        "July","August","September","October","November","December"
    };
    char buf[64];
    snprintf(buf, sizeof(buf), "%s %02d, %d %02d:%02d:%02d",
        months[st.wMonth - 1], st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond);
    return buf;
}

void CConfigSystem::EnsureDir() const { fs::create_directories(GetDir()); }

void CConfigSystem::Refresh()
{
    std::string prevName = (m_loadedIdx >= 0 && m_loadedIdx < (int)m_list.size())
        ? m_list[m_loadedIdx].name : "";
    m_list.clear();
    m_loadedIdx = -1;

    ConfigEntry def;
    def.name = "default";
    def.filename = "default.json";
    def.isDefault = true;
    std::string defPath = GetDir() + "default.json";
    def.modified = fs::exists(defPath) ? FileModTime(defPath) : GetTimestamp();
    m_list.push_back(def);

    std::error_code ec;
    for (auto& entry : fs::directory_iterator(GetDir(), ec))
    {
        if (entry.path().extension() != ".json") continue;
        if (entry.path().stem().string() == "default") continue;
        ConfigEntry ce;
        ce.name = entry.path().stem().string();
        ce.filename = entry.path().filename().string();
        ce.isDefault = false;
        ce.modified = FileModTime(entry.path().string());
        m_list.push_back(ce);
    }

    for (int i = 0; i < (int)m_list.size(); ++i)
        if (m_list[i].name == prevName) { m_loadedIdx = i; break; }
}

std::string CConfigSystem::Serialize() const
{
    std::ostringstream o;
    o << std::fixed << std::setprecision(6) << std::boolalpha;
    o << "{\n";
    auto wb = [&](const char* k, bool  v) { o << "  \"" << k << "\": " << v << ",\n"; };
    auto wi = [&](const char* k, int   v) { o << "  \"" << k << "\": " << v << ",\n"; };
    auto wf = [&](const char* k, float v) { o << "  \"" << k << "\": " << v << ",\n"; };
    auto wc = [&](const char* k, const float* c) {
        o << "  \"" << k << "\": [" << c[0] << ", " << c[1] << ", " << c[2] << ", " << c[3] << "],\n";
        };
    wb("lb_enable", lb_enable);         wb("lb_draw_fov", lb_draw_fov);
    wb("lb_dynamic_fov", lb_dynamic_fov);    wf("lb_amount", lb_amount);
    wf("lb_multiplier", lb_multiplier);     wf("lb_smooth_pitch", lb_smooth_pitch);
    wf("lb_smooth_yaw", lb_smooth_yaw);     wf("lb_target_switch", lb_target_switch);
    wf("lb_min_hit_chance", lb_min_hit_chance); wf("lb_min_damage", lb_min_damage);
    wb("lb_only_body", lb_only_body);      wb("lb_prefer_head", lb_prefer_head);
    wb("lb_prefer_body", lb_prefer_body);    wb("lb_body_lethal", lb_body_lethal);
    o << "  \"hg_selections\": [";
    for (int i = 0; i < 6; ++i) o << (hg_selections[i] ? "true" : "false") << (i < 5 ? ", " : "");
    o << "],\n";
    wb("lb_af_enable", lb_af_enable);     wb("lb_af_auto_stop", lb_af_auto_stop);
    wb("lb_pro_spread", lb_pro_spread);    wf("lb_delay_shots", lb_delay_shots);
    wf("lb_af_min_hit", lb_af_min_hit);   wf("lb_ghost_time", lb_ghost_time);
    wb("p_enable", p_enable);   wb("p_wall", p_wall);     wb("p_name", p_name);
    wb("p_box", p_box);      wb("p_health", p_health);   wb("p_armor", p_armor);
    wb("p_ammo", p_ammo);     wb("p_weapon", p_weapon);   wb("p_skeleton", p_skeleton);
    wi("box_pos", box_pos);    wi("ammo_pos", ammo_pos);
    wc("box_col", box_col);     wc("ammo_col", ammo_col);
    wc("name_col", name_col);    wc("skel_col", skel_col);
    wb("f_bomb", f_bomb);   wb("f_hp", f_hp);    wb("f_ammo", f_ammo);
    wb("f_rel", f_rel);    wb("f_scope", f_scope); wb("f_flash", f_flash);
    wb("f_dist", f_dist);
    o << "  \"f_kit\": " << f_kit << "\n}\n";
    return o.str();
}

void CConfigSystem::Deserialize(const std::string& d)
{
    lb_enable = RBool(d, "lb_enable", lb_enable);
    lb_draw_fov = RBool(d, "lb_draw_fov", lb_draw_fov);
    lb_dynamic_fov = RBool(d, "lb_dynamic_fov", lb_dynamic_fov);
    lb_amount = RFloat(d, "lb_amount", lb_amount);
    lb_multiplier = RFloat(d, "lb_multiplier", lb_multiplier);
    lb_smooth_pitch = RFloat(d, "lb_smooth_pitch", lb_smooth_pitch);
    lb_smooth_yaw = RFloat(d, "lb_smooth_yaw", lb_smooth_yaw);
    lb_target_switch = RFloat(d, "lb_target_switch", lb_target_switch);
    lb_min_hit_chance = RFloat(d, "lb_min_hit_chance", lb_min_hit_chance);
    lb_min_damage = RFloat(d, "lb_min_damage", lb_min_damage);
    lb_only_body = RBool(d, "lb_only_body", lb_only_body);
    lb_prefer_head = RBool(d, "lb_prefer_head", lb_prefer_head);
    lb_prefer_body = RBool(d, "lb_prefer_body", lb_prefer_body);
    lb_body_lethal = RBool(d, "lb_body_lethal", lb_body_lethal);
    RBoolArray(d, "hg_selections", hg_selections, 6);
    lb_af_enable = RBool(d, "lb_af_enable", lb_af_enable);
    lb_af_auto_stop = RBool(d, "lb_af_auto_stop", lb_af_auto_stop);
    lb_pro_spread = RBool(d, "lb_pro_spread", lb_pro_spread);
    lb_delay_shots = RFloat(d, "lb_delay_shots", lb_delay_shots);
    lb_af_min_hit = RFloat(d, "lb_af_min_hit", lb_af_min_hit);
    lb_ghost_time = RFloat(d, "lb_ghost_time", lb_ghost_time);
    p_enable = RBool(d, "p_enable", p_enable);  p_wall = RBool(d, "p_wall", p_wall);
    p_name = RBool(d, "p_name", p_name);    p_box = RBool(d, "p_box", p_box);
    p_health = RBool(d, "p_health", p_health);  p_armor = RBool(d, "p_armor", p_armor);
    p_ammo = RBool(d, "p_ammo", p_ammo);    p_weapon = RBool(d, "p_weapon", p_weapon);
    p_skeleton = RBool(d, "p_skeleton", p_skeleton);
    box_pos = RInt(d, "box_pos", box_pos);
    ammo_pos = RInt(d, "ammo_pos", ammo_pos);
    RFloatArray(d, "box_col", box_col, 4); RFloatArray(d, "ammo_col", ammo_col, 4);
    RFloatArray(d, "name_col", name_col, 4); RFloatArray(d, "skel_col", skel_col, 4);
    f_bomb = RBool(d, "f_bomb", f_bomb);  f_hp = RBool(d, "f_hp", f_hp);
    f_ammo = RBool(d, "f_ammo", f_ammo);  f_rel = RBool(d, "f_rel", f_rel);
    f_scope = RBool(d, "f_scope", f_scope); f_flash = RBool(d, "f_flash", f_flash);
    f_dist = RBool(d, "f_dist", f_dist);  f_kit = RBool(d, "f_kit", f_kit);
}

bool CConfigSystem::RBool(const std::string& j, const std::string& k, bool d)
{
    size_t p = j.find("\"" + k + "\":");
    if (p == std::string::npos) return d;
    p = j.find_first_not_of(" ", p + k.size() + 3);
    if (p == std::string::npos) return d;
    if (j.compare(p, 4, "true") == 0) return true;
    if (j.compare(p, 5, "false") == 0) return false;
    return d;
}

int CConfigSystem::RInt(const std::string& j, const std::string& k, int d)
{
    size_t p = j.find("\"" + k + "\":");
    if (p == std::string::npos) return d;
    p = j.find_first_not_of(" ", p + k.size() + 3);
    if (p == std::string::npos) return d;
    try { return std::stoi(j.substr(p)); }
    catch (...) { return d; }
}

float CConfigSystem::RFloat(const std::string& j, const std::string& k, float d)
{
    size_t p = j.find("\"" + k + "\":");
    if (p == std::string::npos) return d;
    p = j.find_first_not_of(" ", p + k.size() + 3);
    if (p == std::string::npos) return d;
    try { return std::stof(j.substr(p)); }
    catch (...) { return d; }
}

void CConfigSystem::RBoolArray(const std::string& j, const std::string& k, bool* out, int n)
{
    size_t p = j.find("\"" + k + "\":");
    if (p == std::string::npos) return;
    p = j.find('[', p);
    if (p == std::string::npos) return;
    ++p;
    for (int i = 0; i < n; ++i)
    {
        p = j.find_first_not_of(" ,", p);
        if (p == std::string::npos || j[p] == ']') break;
        if (j.compare(p, 4, "true") == 0) { out[i] = true;  p += 4; }
        else if (j.compare(p, 5, "false") == 0) { out[i] = false; p += 5; }
    }
}

void CConfigSystem::RFloatArray(const std::string& j, const std::string& k, float* out, int n)
{
    size_t p = j.find("\"" + k + "\":");
    if (p == std::string::npos) return;
    p = j.find('[', p);
    if (p == std::string::npos) return;
    ++p;
    for (int i = 0; i < n; ++i)
    {
        p = j.find_first_not_of(" ,", p);
        if (p == std::string::npos || j[p] == ']') break;
        try { size_t consumed; out[i] = std::stof(j.substr(p), &consumed); p += consumed; }
        catch (...) { break; }
    }
}

bool CConfigSystem::SaveToFile(const std::string& path)
{
    std::ofstream f(path, std::ios::trunc);
    if (!f.is_open()) return false;
    f << Serialize();
    return true;
}

bool CConfigSystem::LoadFromFile(const std::string& path)
{
    std::ifstream f(path);
    if (!f.is_open()) return false;
    std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    Deserialize(content);
    return true;
}

void CConfigSystem::DeleteEntry(int idx)
{
    if (idx <= 0 || idx >= (int)m_list.size() || m_list[idx].isDefault) return;
    DeleteFileA((GetDir() + m_list[idx].filename).c_str());
    if (m_loadedIdx == idx) m_loadedIdx = -1;
    else if (m_loadedIdx > idx) --m_loadedIdx;
    m_list.erase(m_list.begin() + idx);
}

void CConfigSystem::RenameEntry(int idx, const std::string& newName)
{
    if (idx <= 0 || idx >= (int)m_list.size() || m_list[idx].isDefault || newName.empty()) return;
    std::string oldPath = GetDir() + m_list[idx].filename;
    std::string newFile = newName + ".json";
    if (MoveFileA(oldPath.c_str(), (GetDir() + newFile).c_str()))
    {
        m_list[idx].name = newName;
        m_list[idx].filename = newFile;
    }
}

void CConfigSystem::Initialize()
{
    EnsureDir();
    std::string defPath = GetDir() + "default.json";
    if (!fs::exists(defPath)) SaveToFile(defPath);
    Refresh();
}

void CConfigSystem::Render()
{
    if (!m_initialized) { Initialize(); m_initialized = true; }

    m_autoSaveAcc += ImGui::GetIO().DeltaTime;
    if (m_autoSaveAcc >= 10.f)
    {
        m_autoSaveAcc = 0.f;
        if (!m_list.empty() && m_list[0].isDefault && SaveToFile(GetDir() + "default.json"))
            m_list[0].modified = GetTimestamp();
    }

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImFont* iFont = g_Menu.IconFont;
    ImFont* dFont = ImGui::GetFont();
    const float fSz = ImGui::GetFontSize();
    const float iSz = 13.f;

    const float listW = 580.f;
    const float rowH = 58.f;
    const float visH = 54.f;
    const float btnH = 26.f;
    const float iconBW = 22.f;
    const float loadBW = 68.f;
    const float hdrH = 30.f;

    const ImU32 colModLabel = ImColor(115, 115, 115);
    const ImU32 colModDate = ImColor(210, 80, 105);
    const ImU32 colIconNorm = ImColor(130, 130, 130);
    const ImU32 colIconHovT = ImColor(210, 210, 210);
    const ImU32 colIconHovD = ImColor(255, 70, 70);
    const ImU32 colRowNorm = ImColor(22, 22, 22);
    const ImU32 colRowHov = ImColor(30, 30, 30);
    const ImU32 colRowBorder = ImColor(38, 38, 38);

    auto FlatIconBtn = [&](const char* id, const char* icon, float w, float h,
        ImU32 colHov) -> bool
        {
            ImVec2 sp = ImGui::GetCursorScreenPos();
            bool pressed = ImGui::InvisibleButton(id, ImVec2(w, h));
            bool hov = ImGui::IsItemHovered();
            bool held = ImGui::IsItemActive();
            if (icon && iFont)
            {
                float iw = iFont->CalcTextSizeA(iSz, FLT_MAX, 0.f, icon).x;
                dl->AddText(iFont, iSz,
                    ImVec2(sp.x + (w - iw) * 0.5f, sp.y + (h - iSz) * 0.5f + 1.f),
                    (hov || held) ? colHov : colIconNorm, icon);
            }
            return pressed;
        };

    auto BoxBtn = [&](const char* id, const char* icon, const char* label,
        float w, float h, bool accent) -> bool
        {
            ImVec2 sp = ImGui::GetCursorScreenPos();
            bool pressed = ImGui::InvisibleButton(id, ImVec2(w, h));
            bool hov = ImGui::IsItemHovered();
            bool held = ImGui::IsItemActive();
            ImU32 bg = accent
                ? (held ? ImColor(40, 6, 6) : hov ? ImColor(180, 32, 32) : ImColor(34, 34, 34))
                : (held ? ImColor(20, 20, 20) : hov ? ImColor(34, 34, 34) : ImColor(26, 26, 26));
            dl->AddRectFilled(sp, ImVec2(sp.x + w, sp.y + h), bg, 3.f);
            dl->AddRect(sp, ImVec2(sp.x + w, sp.y + h), accent ? ImColor(90, 20, 20) : ImColor(48, 48, 48), 3.f, 0, 1.f);

            float totalW = 0.f;
            if (icon && iFont) totalW += iFont->CalcTextSizeA(iSz, FLT_MAX, 0.f, icon).x + 5.f;
            if (label)         totalW += dFont->CalcTextSizeA(fSz, FLT_MAX, 0.f, label).x;
            float cx = sp.x + (w - totalW) * 0.5f;
            float cy_i = sp.y + (h - iSz) * 0.5f + 1.f;
            float cy_t = sp.y + (h - fSz) * 0.5f;
            if (icon && iFont)
            {
                float iw = iFont->CalcTextSizeA(iSz, FLT_MAX, 0.f, icon).x;
                dl->AddText(iFont, iSz, ImVec2(cx, cy_i), ImColor(215, 215, 215), icon);
                cx += iw + 5.f;
            }
            if (label)
                dl->AddText(dFont, fSz, ImVec2(cx, cy_t), ImColor(215, 215, 215), label);
            return pressed;
        };

    ImGui::SetCursorPos(ImVec2(20.f, 20.f));
    ImVec2 hdrStart = ImGui::GetCursorPos();
    ImGui::Dummy(ImVec2(listW, hdrH));
    ImGui::SetCursorPos(hdrStart);

    bool openCreate = false;
    bool openRename = false;

    if (BoxBtn("##BtnCreate", ICON_FA_PLUS, "Create", 90.f, hdrH, false))
    {
        memset(m_createBuf, 0, sizeof(m_createBuf));
        openCreate = true;
    }
    ImGui::SameLine(0, 6);
    if (BoxBtn("##BtnRefresh", ICON_FA_SYNC_ALT, "Refresh", 90.f, hdrH, false))
        Refresh();
    ImGui::SameLine(0, 6);
    BoxBtn("##BtnPersonal", ICON_FA_SLIDERS_H, "Personalization", 152.f, hdrH, false);

    ImGui::SetCursorPos(ImVec2(20.f, hdrStart.y + hdrH + 8.f));

    ImGui::TextColored(ImVec4(0.45f, 0.45f, 0.45f, 1.f), "You can restore your removed configs in ");
    ImGui::SameLine(0, 0);
    ImGui::TextColored(ImVec4(1.f, 0.25f, 0.25f, 1.f), "recent deleted");
    ImGui::SameLine(0, 0);
    ImGui::TextColored(ImVec4(0.45f, 0.45f, 0.45f, 1.f), " section.");
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6.f);
    ImGui::SetCursorPosX(20.f);

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.f, 0.f, 0.f, 0.f));
    if (ImGui::BeginChild("##CfgList", ImVec2(listW, 430.f), false,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
    {
        int deleteIdx = -1;

        for (int i = 0; i < (int)m_list.size(); ++i)
        {
            ConfigEntry& cfg = m_list[i];
            const bool   isDef = cfg.isDefault;

            ImGui::PushID(i);

            ImVec2 rowWinPos = ImGui::GetCursorPos();
            ImVec2 rowScrPos = ImGui::GetCursorScreenPos();

            ImGui::Dummy(ImVec2(listW, rowH));
            bool rowHov = ImGui::IsMouseHoveringRect(rowScrPos,
                ImVec2(rowScrPos.x + listW, rowScrPos.y + visH));

            dl->AddRectFilled(rowScrPos, ImVec2(rowScrPos.x + listW, rowScrPos.y + visH),
                rowHov ? colRowHov : colRowNorm, 5.f);
            dl->AddRect(rowScrPos, ImVec2(rowScrPos.x + listW, rowScrPos.y + visH),
                colRowBorder, 5.f, 0, 1.f);

            const char* rowIcon = isDef ? ICON_FA_PAINT_BRUSH : ICON_FA_COG;
            if (iFont)
                dl->AddText(iFont, 14.f, ImVec2(rowScrPos.x + 12.f, rowScrPos.y + 11.f),
                    ImColor(155, 155, 155), rowIcon);

            dl->AddText(dFont, fSz,
                ImVec2(rowScrPos.x + 32.f, rowScrPos.y + 8.f),
                ImColor(230, 230, 230), cfg.name.c_str());

            const char* modLabel = "Modified: ";
            float modLabelW = dFont->CalcTextSizeA(fSz - 1.f, FLT_MAX, 0.f, modLabel).x;
            float modX = rowScrPos.x + 32.f;
            float modY = rowScrPos.y + 30.f;
            dl->AddText(dFont, fSz - 1.f, ImVec2(modX, modY), colModLabel, modLabel);
            dl->AddText(dFont, fSz - 1.f, ImVec2(modX + modLabelW, modY), colModDate, cfg.modified.c_str());

            const float btnYOff = rowWinPos.y + (visH - btnH) * 0.5f;
            float       rightX = rowWinPos.x + listW - 10.f;

            const bool loaded = (m_loadedIdx == i);

            if (loaded)
            {
                ImGui::SetCursorPos(ImVec2(rightX - loadBW, btnYOff));
                if (BoxBtn("##Save", ICON_FA_SAVE, "Save", loadBW, btnH, true))
                {
                    if (SaveToFile(GetDir() + cfg.filename))
                        cfg.modified = GetTimestamp();
                }
                rightX -= loadBW + 8.f;

                ImGui::SetCursorPos(ImVec2(rightX - iconBW, btnYOff));
                FlatIconBtn("##Upload", ICON_FA_UPLOAD, iconBW, btnH, colIconHovT);
                rightX -= iconBW + 6.f;

                if (!isDef)
                {
                    ImGui::SetCursorPos(ImVec2(rightX - iconBW, btnYOff));
                    if (FlatIconBtn("##Pencil", ICON_FA_PENCIL_ALT, iconBW, btnH, colIconHovT))
                    {
                        m_editIdx = i;
                        strncpy_s(m_editBuf, sizeof(m_editBuf), cfg.name.c_str(), _TRUNCATE);
                        openRename = true;
                    }
                    rightX -= iconBW + 6.f;
                }

                ImGui::SetCursorPos(ImVec2(rightX - iconBW, btnYOff));
                if (FlatIconBtn("##Sync", ICON_FA_SYNC_ALT, iconBW, btnH, colIconHovT))
                {
                    if (LoadFromFile(GetDir() + cfg.filename))
                        cfg.modified = GetTimestamp();
                }
            }
            else
            {
                ImGui::SetCursorPos(ImVec2(rightX - loadBW, btnYOff));
                if (BoxBtn("##Load", ICON_FA_DOWNLOAD, "Load", loadBW, btnH, false))
                {
                    if (LoadFromFile(GetDir() + cfg.filename))
                        m_loadedIdx = i;
                }
                rightX -= loadBW + 8.f;

                if (!isDef)
                {
                    ImGui::SetCursorPos(ImVec2(rightX - iconBW, btnYOff));
                    if (FlatIconBtn("##Del", ICON_FA_TRASH, iconBW, btnH, colIconHovD))
                        deleteIdx = i;
                    rightX -= iconBW + 6.f;

                    ImGui::SetCursorPos(ImVec2(rightX - iconBW, btnYOff));
                    if (FlatIconBtn("##Edit", ICON_FA_PENCIL_ALT, iconBW, btnH, colIconHovT))
                    {
                        m_editIdx = i;
                        strncpy_s(m_editBuf, sizeof(m_editBuf), cfg.name.c_str(), _TRUNCATE);
                        openRename = true;
                    }
                }
            }

            ImGui::SetCursorPos(ImVec2(rowWinPos.x, rowWinPos.y + rowH));
            ImGui::Dummy(ImVec2(listW, 0.f));

            ImGui::PopID();
        }

        if (deleteIdx >= 0) DeleteEntry(deleteIdx);

        if (openCreate) ImGui::OpenPopup("##PopCreate");
        if (openRename) ImGui::OpenPopup("##PopRename");

        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.09f, 0.09f, 0.09f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.20f, 0.20f, 0.20f, 1.f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.f, 14.f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.14f, 0.14f, 0.14f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.18f, 0.18f, 0.18f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.72f, 0.16f, 0.16f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.88f, 0.24f, 0.24f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.50f, 0.08f, 0.08f, 1.f));

        auto PopupDialog = [&](const char* id, const char* title, char* buf,
            size_t bufSz, const char* btnTxt) -> bool
            {
                bool result = false;
                if (ImGui::BeginPopup(id))
                {
                    ImGui::TextColored(ImVec4(0.70f, 0.70f, 0.70f, 1.f), "%s", title);
                    ImGui::Spacing();
                    ImGui::SetNextItemWidth(220.f);
                    bool enter = ImGui::InputText("##inp", buf, bufSz,
                        ImGuiInputTextFlags_EnterReturnsTrue);
                    ImGui::SameLine(0, 8);
                    bool ok = ImGui::Button(btnTxt, ImVec2(70.f, 0));
                    if ((enter || ok) && buf[0]) { result = true; ImGui::CloseCurrentPopup(); }
                    ImGui::EndPopup();
                }
                return result;
            };

        if (PopupDialog("##PopCreate", "New config name", m_createBuf, sizeof(m_createBuf), "Create"))
        {
            std::string name(m_createBuf);
            std::string file = name + ".json";
            if (SaveToFile(GetDir() + file))
            {
                m_list.push_back(ConfigEntry{ name, file, GetTimestamp(), false });
                m_loadedIdx = (int)m_list.size() - 1;
            }
        }

        if (PopupDialog("##PopRename", "Rename config", m_editBuf, sizeof(m_editBuf), "Rename")
            && m_editIdx >= 0)
        {
            RenameEntry(m_editIdx, m_editBuf);
            m_editIdx = -1;
        }

        ImGui::PopStyleColor(7);
        ImGui::PopStyleVar(3);
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
}