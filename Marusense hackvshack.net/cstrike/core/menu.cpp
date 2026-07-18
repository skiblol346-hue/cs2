#include "menu.h"
#include "variables.h"



namespace NL
{
    static constexpr ImU32 BG_ITEM_HOV = IM_COL32(0x1a, 0x1a, 0x1a, 0xFF);
    static constexpr ImU32 BG_ITEM_SEL = IM_COL32(0x22, 0x22, 0x22, 0xFF);
    static constexpr ImU32 TRACK = IM_COL32(0x18, 0x18, 0x18, 0xFF);
    static constexpr ImU32 HEADER_LINE = IM_COL32(0x24, 0x24, 0x24, 0xFF);
    static constexpr ImU32 KNOB = IM_COL32(0xff, 0xff, 0xff, 0xFF);
    static constexpr ImU32 BTN_BG = IM_COL32(0x16, 0x16, 0x16, 0xFF);
    static constexpr ImU32 BTN_BDR = IM_COL32(0x2a, 0x2a, 0x2a, 0xFF);
    static constexpr ImU32 BTN_HOV = IM_COL32(0x20, 0x20, 0x20, 0xFF);
    static constexpr ImU32 BTN_PRI = IM_COL32(0x2e, 0x2e, 0x2e, 0xFF);
    static constexpr ImU32 BTN_PRI_HOV = IM_COL32(0x3a, 0x3a, 0x3a, 0xFF);
    static constexpr ImU32 AVATAR_BG = IM_COL32(0x20, 0x20, 0x20, 0x88);
    static constexpr ImU32 AVATAR_BDR = IM_COL32(0x50, 0x50, 0x50, 0xFF);

    static constexpr ImU32 GRAD_L = IM_COL32(0x03, 0x06, 0x18, 0xFF); 
    static constexpr ImU32 GRAD_R = IM_COL32(0x00, 0xb4, 0xd8, 0xFF); 
    static constexpr ImU32 GRAD_MID = IM_COL32(0x00, 0x8a, 0xb0, 0xFF);
}



namespace W
{


    bool Toggle(const char* label, bool* v)
    {
        ImGuiWindow* win = ImGui::GetCurrentWindow();
        if (win->SkipItems) return false;
        ImGuiID id = win->GetID(label);
        ImVec2  pos = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const float SZ = 13.f, R = 3.f;
        float fh = ImGui::GetFontSize();
        float avail = ImGui::GetContentRegionAvail().x;
        float ih = ImMax(SZ, fh);
        ImRect bb(pos, ImVec2(pos.x + avail, pos.y + ih));
        ImGui::ItemSize(bb);
        if (!ImGui::ItemAdd(bb, id)) return false;
        bool hov, held;
        bool clicked = ImGui::ButtonBehavior(bb, id, &hov, &held);
        if (clicked) *v = !*v;

        static std::unordered_map<ImGuiID, float> anim;
        float& t = anim[id];
        t += ((*v ? 1.f : 0.f) - t) * ImGui::GetIO().DeltaTime * 14.f;
        t = ImClamp(t, 0.f, 1.f);

        if (hov)
            dl->AddRectFilled(
                ImVec2(pos.x - 4.f, pos.y - 1.f),
                ImVec2(pos.x + avail + 4.f, pos.y + ih + 1.f),
                NL::BG_ITEM_HOV, 3.f);

        dl->AddText(ImVec2(pos.x, pos.y + (ih - fh) * 0.5f),
            hov ? MC::TEXT : MC::TEXT_DIM, label);

        ImVec2 bmin(pos.x + avail - SZ, pos.y + (ih - SZ) * 0.5f);
        ImVec2 bmax(bmin.x + SZ, bmin.y + SZ);


        if (t > 0.99f) {

            dl->AddRectFilledMultiColor(
                bmin, bmax,
                IM_COL32(0x03, 0x06, 0x18, 0xFF), IM_COL32(0x00, 0xb4, 0xd8, 0xFF),
                IM_COL32(0x00, 0xb4, 0xd8, 0xFF), IM_COL32(0x03, 0x06, 0x18, 0xFF));
            dl->AddRectFilled(bmin, bmax, IM_COL32(0, 0, 0, 0), R);
        }
        else if (t < 0.01f) {
            dl->AddRectFilled(bmin, bmax, MC::INPUT_BG, R);
            dl->AddRect(bmin, bmax, hov ? MC::ACCENT : MC::INPUT_BDR, R, 0, 1.f);
        }
        else {
            ImU32 bgCol = MC::LerpU32(MC::INPUT_BG, IM_COL32(0x00, 0x8a, 0xb0, 0xFF), t);
            dl->AddRectFilled(bmin, bmax, bgCol, R);
            ImU32 bdrCol = MC::LerpU32(MC::INPUT_BDR, MC::ACCENT, t);
            dl->AddRect(bmin, bmax, bdrCol, R, 0, 1.f);
        }


        if (t > 0.01f) {
            ImU32 checkCol = MC::LerpU32(IM_COL32(0, 0, 0, 0), NL::KNOB, t);

            ImVec2 p0(bmin.x + 2.f, bmin.y + 6.f);
            ImVec2 p1(bmin.x + 5.f, bmin.y + 9.5f);
            ImVec2 p2(bmin.x + 11.f, bmin.y + 3.5f);

            ImVec2 mid1 = ImLerp(p0, p1, ImMin(t * 2.f, 1.f));
            ImVec2 mid2 = ImLerp(p1, p2, ImMax(t * 2.f - 1.f, 0.f));
            dl->AddLine(p0, mid1, checkCol, 1.5f);
            if (t > 0.5f)
                dl->AddLine(p1, mid2, checkCol, 1.5f);
        }

        return clicked;
    }
    
    void Section(const char* label)
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2      pos = ImGui::GetCursorScreenPos();
        float       fh = ImGui::GetFontSize();
        float       avail = ImGui::GetContentRegionAvail().x;
        float       lineY = pos.y + fh * 0.5f;

        dl->AddRectFilledMultiColor(
            { pos.x,              lineY - 0.5f },
            { pos.x + avail * 0.55f, lineY + 0.5f },
            IM_COL32(0x55, 0x55, 0x55, 0xFF), IM_COL32(0x55, 0x55, 0x55, 0x00),
            IM_COL32(0x55, 0x55, 0x55, 0x00), IM_COL32(0x55, 0x55, 0x55, 0xFF));


        float lw = ImGui::CalcTextSize(label).x;
        float lpad = 6.f;
        dl->AddRectFilled(
            { pos.x - lpad, pos.y },
            { pos.x + lw + lpad, pos.y + fh },
            MC::BG_PANEL);

        dl->AddText(pos, MC::TEXT_DIM, label);
        ImGui::Dummy(ImVec2(0.f, fh + 6.f));
    }

    bool SliderRow(const char* label, float* value,
        float mn, float mx,
        const char* suffix = "", float step = 1.f)
    {
        ImGuiWindow* win = ImGui::GetCurrentWindow();
        if (win->SkipItems) return false;

        ImGuiID id = win->GetID(label);
        float   avail = ImGui::GetContentRegionAvail().x;
        float   fh = ImGui::GetFontSize();
        ImDrawList* dl = ImGui::GetWindowDrawList();

        ImVec2 pos = ImGui::GetCursorScreenPos();
        char buf[32];
        if (step >= 1.f && fabsf(step - 1.f) < 0.01f)
            ImFormatString(buf, sizeof(buf), "%d%s", (int)*value, suffix);
        else
            ImFormatString(buf, sizeof(buf), "%.2f%s", *value, suffix);

        ImVec2 vsz = ImGui::CalcTextSize(buf);
        dl->AddText(pos, MC::TEXT_DIM, label);
        dl->AddText(ImVec2(pos.x + avail - vsz.x, pos.y), IM_COL32(0x00, 0xb4, 0xd8, 0xFF), buf);
        ImGui::Dummy(ImVec2(avail, fh + 2.f));

        const float TRACK_H = 3.f, KR = 5.f;
        ImVec2 tp = ImGui::GetCursorScreenPos();
        ImVec2 tmin(tp.x, tp.y + KR - TRACK_H * 0.5f);
        ImVec2 tmax(tp.x + avail, tmin.y + TRACK_H);

        float norm = ImClamp((*value - mn) / (mx - mn), 0.f, 1.f);
        float fw = avail * norm;

        dl->AddRectFilled(tmin, tmax, NL::TRACK, 2.f);
        if (fw > 0.f)
            dl->AddRectFilledMultiColor(
                tmin, ImVec2(tmin.x + fw, tmax.y),
                IM_COL32(0x03, 0x06, 0x18, 0xFF), IM_COL32(0x00, 0xb4, 0xd8, 0xFF),
                IM_COL32(0x00, 0xb4, 0xd8, 0xFF), IM_COL32(0x03, 0x06, 0x18, 0xFF));

        float kx = tmin.x + fw, ky = tmin.y + TRACK_H * 0.5f;
        ImRect knobRc({ kx - KR, ky - KR }, { kx + KR, ky + KR });
        bool   kHov = knobRc.Contains(ImGui::GetMousePos());
        dl->AddCircleFilled({ kx, ky }, KR, kHov ? IM_COL32(0x48, 0xcf, 0xee, 0xFF) : NL::KNOB);
        dl->AddCircle({ kx, ky }, KR, IM_COL32(0x00, 0xb4, 0xd8, 0xFF), 12, 1.2f);

        ImGui::Dummy(ImVec2(avail, KR * 2.f + 4.f));

        ImRect sliderBb(tmin, ImVec2(tmax.x, tmax.y + KR * 2.f));
        ImGui::ItemSize(sliderBb);
        if (!ImGui::ItemAdd(sliderBb, id)) return false;

        bool hov, held;
        bool changed = ImGui::ButtonBehavior(sliderBb, id, &hov, &held);
        if (held && ImGui::IsMouseDragging(0))
        {
            float nn = ImClamp((ImGui::GetMousePos().x - tmin.x) / avail, 0.f, 1.f);
            float nv = mn + nn * (mx - mn);
            if (step > 0.f) nv = roundf(nv / step) * step;
            *value = ImClamp(nv, mn, mx);
            changed = true;
        }
        return changed;
    }



    bool SliderRowInt(const char* label, int* value, int mn, int mx,
        const char* suffix = "")
    {
        float f = (float)*value;
        bool  c = SliderRow(label, &f, (float)mn, (float)mx, suffix, 1.f);
        if (c) *value = (int)f;
        return c;
    }

   
    bool CheckboxRow(const char* label, bool* v)
    {
        ImGuiWindow* win = ImGui::GetCurrentWindow();
        if (win->SkipItems) return false;
        ImGuiID id = win->GetID(label);
        ImVec2  pos = ImGui::GetCursorScreenPos();
        float   avail = ImGui::GetContentRegionAvail().x;
        float   fh = ImGui::GetFontSize();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const float SZ = 13.f, R = 3.f;
        float ih = ImMax(SZ, fh);
        ImRect bb(pos, ImVec2(pos.x + avail, pos.y + ih));
        ImGui::ItemSize(bb);
        if (!ImGui::ItemAdd(bb, id)) return false;
        bool hov, held;
        bool clicked = ImGui::ButtonBehavior(bb, id, &hov, &held);
        if (clicked) *v = !*v;
        if (hov)
            dl->AddRectFilled(ImVec2(pos.x - 4.f, pos.y - 1.f),
                ImVec2(pos.x + avail + 4.f, pos.y + ih + 1.f),
                NL::BG_ITEM_HOV, 3.f);
        dl->AddText(ImVec2(pos.x, pos.y + (ih - fh) * 0.5f),
            hov ? MC::TEXT : MC::TEXT_DIM, label);
        ImVec2 bmin(pos.x + avail - SZ, pos.y + (ih - SZ) * 0.5f);
        ImVec2 bmax(bmin.x + SZ, bmin.y + SZ);
        if (*v) {
            dl->AddRectFilled(bmin, bmax, MC::ACCENT, R);
            dl->AddLine({ bmin.x + 2.f,  bmin.y + 6.f },
                { bmin.x + 5.f,  bmin.y + 9.5f }, NL::KNOB, 1.5f);
            dl->AddLine({ bmin.x + 5.f,  bmin.y + 9.5f },
                { bmin.x + 11.f, bmin.y + 3.5f }, NL::KNOB, 1.5f);
        }
        else {
            dl->AddRectFilled(bmin, bmax, MC::INPUT_BG, R);
            dl->AddRect(bmin, bmax, hov ? MC::ACCENT : MC::INPUT_BDR, R, 0, 1.f);
        }
        return clicked;
    }

 
    void PanelHeader(ImDrawList* dl, ImVec2 cp, ImVec2 cs, const char* title, bool)
    {
        const float HDR_H = 28.f;
        float fh = ImGui::GetFontSize();
        dl->AddLine({ cp.x, cp.y + HDR_H }, { cp.x + cs.x, cp.y + HDR_H },
            NL::HEADER_LINE, 1.f);
        dl->AddText({ cp.x + 10.f, cp.y + (HDR_H - fh) * 0.5f }, MC::TEXT, title);
    }


    bool SubTab(const char* label, bool active)
    {
        ImGuiWindow* win = ImGui::GetCurrentWindow();
        if (win->SkipItems) return false;

        ImGuiID     id = win->GetID(label);
        ImVec2      pos = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();

        ImVec2 sz = ImGui::CalcTextSize(label);
        float  px = 12.f, py = 6.f;
        float  w = sz.x + px * 2.f, h = sz.y + py * 2.f;

        ImRect bb(pos, { pos.x + w, pos.y + h });
        ImGui::ItemSize(bb);
        if (!ImGui::ItemAdd(bb, id)) return false;

        bool hov, held;
        bool clicked = ImGui::ButtonBehavior(bb, id, &hov, &held);

        ImU32 tc = active ? MC::TEXT : (hov ? MC::TEXT_DIM : MC::TEXT_MUTED);
        dl->AddText({ pos.x + px, pos.y + py }, tc, label);

        if (active)
            dl->AddLine({ pos.x, pos.y + h - 1.f },
                { pos.x + w, pos.y + h - 1.f }, NL::GRAD_R, 2.f);
        return clicked;
    }

    static bool    g_combo_open = false;
    static ImGuiID g_combo_id = 0;
    static float   g_combo_scroll = 0.f;

    bool Combo(const char* str_id, const char* label,
        const char* const* items, int count, int& sel,
        float width = 120.f)
    {
        bool        changed = false;
        ImGuiID     id = ImGui::GetID(str_id);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImGuiIO& io = ImGui::GetIO();
        ImVec2      p = ImGui::GetCursorScreenPos();

        float lw = 0.f;
        if (label[0] != '#')
        {
            dl->AddText({ p.x, p.y + (24.f - ImGui::GetTextLineHeight()) * 0.5f },
                MC::TEXT_DIM, label);
            lw = ImGui::CalcTextSize(label).x + 8.f;
        }

        ImRect bb({ p.x + lw, p.y }, { p.x + lw + width, p.y + 24.f });
        bool hov = bb.Contains(io.MousePos);
        bool open = g_combo_open && g_combo_id == id;

        if (hov && ImGui::IsMouseClicked(0))
        {
            if (open) g_combo_open = false;
            else { g_combo_open = true; g_combo_id = id; g_combo_scroll = 0.f; }
            open = g_combo_open && g_combo_id == id;
        }

     
        dl->AddRectFilled(bb.Min, bb.Max,
            open ? MC::INPUT_BG : (hov ? IM_COL32(0x1e, 0x1e, 0x1e, 0xFF) : MC::INPUT_BG));
        dl->AddRect(bb.Min, bb.Max,
            open ? NL::GRAD_R : (hov ? MC::BORDER : MC::INPUT_BDR), 0.f, 0, 1.f);

        if (open)
            dl->AddLine({ bb.Min.x + 1, bb.Min.y },
                { bb.Max.x - 1, bb.Min.y }, NL::GRAD_R, 1.f);


        dl->PushClipRect({ bb.Min.x + 6, bb.Min.y }, { bb.Max.x - 18, bb.Max.y }, true);
        dl->AddText({ bb.Min.x + 6, bb.Min.y + (24.f - ImGui::GetTextLineHeight()) * 0.5f },
            MC::TEXT, (sel >= 0 && sel < count) ? items[sel] : "---");
        dl->PopClipRect();

     
        float ax = bb.Max.x - 13.f, ay = bb.Min.y + 12.f;
        if (!open)
            dl->AddTriangleFilled({ ax, ay - 2 }, { ax + 7, ay - 2 }, { ax + 3.5f, ay + 3 },
                hov ? MC::TEXT : MC::TEXT_MUTED);
        else
            dl->AddTriangleFilled({ ax, ay + 2 }, { ax + 7, ay + 2 }, { ax + 3.5f, ay - 3 }, NL::GRAD_R);

        ImGui::Dummy(ImVec2(lw + width, 24.f));

        if (!open) return changed;

      
        float  ph = ImMin((float)count, 5.f) * 22.f;
        ImVec2 pp{ bb.Min.x, bb.Max.y + 1.f };

        if (ImRect(pp, { pp.x + width, pp.y + ph }).Contains(io.MousePos) && io.MouseWheel != 0.f)
        {
            float ms = 22.f * (count - 5);
            g_combo_scroll = ImClamp(g_combo_scroll - io.MouseWheel * 22.f, 0.f, ms > 0 ? ms : 0.f);
        }

        ImDrawList* fdl = ImGui::GetForegroundDrawList();

     
        fdl->AddRectFilled({ pp.x + 2, pp.y + 2 }, { pp.x + width + 2, pp.y + ph + 2 },
            IM_COL32(0, 0, 0, 60));
        fdl->AddRectFilled(pp, { pp.x + width, pp.y + ph }, MC::BG_PANEL);
        fdl->AddRect(pp, { pp.x + width, pp.y + ph }, MC::BORDER, 0.f, 0, 1.f);
        fdl->AddLine({ pp.x + 1, pp.y }, { pp.x + width - 1, pp.y }, NL::GRAD_R, 1.f);

        fdl->PushClipRect(pp, { pp.x + width, pp.y + ph }, true);

        int from = (int)(g_combo_scroll / 22.f);
        int to = ImMin(from + 7, count);

        for (int i = from; i < to; i++)
        {
            float  iy = pp.y + i * 22.f - g_combo_scroll;
            ImRect ib({ pp.x + 1, iy }, { pp.x + width - 1, iy + 22.f });
            bool   ih = ib.Contains(io.MousePos);
            bool   is = (i == sel);

            if (is)
            {
                fdl->AddRectFilledMultiColor(ib.Min, ib.Max,
                    IM_COL32(0x00, 0xb4, 0xd8, 0x55), IM_COL32(0x03, 0x06, 0x18, 0xFF),
                    IM_COL32(0x03, 0x06, 0x18, 0xFF), IM_COL32(0x00, 0xb4, 0xd8, 0x55));
            }
            else if (ih)
                fdl->AddRectFilled(ib.Min, ib.Max, IM_COL32(0x1c, 0x1c, 0x1c, 0xFF));

          
            if (is)
                fdl->AddRectFilled({ ib.Min.x, ib.Min.y + 3 },
                    { ib.Min.x + 2, ib.Max.y - 3 }, NL::GRAD_R);

       
            if (i > 0)
                fdl->AddLine({ ib.Min.x + 6, ib.Min.y },
                    { ib.Max.x - 6, ib.Min.y }, MC::BORDER_DIM, 1.f);

  
            fdl->AddText({ pp.x + 10, iy + (22.f - ImGui::GetTextLineHeight()) * 0.5f },
                is ? MC::TEXT : (ih ? IM_COL32(0xb0, 0xb0, 0xb8, 0xFF) : MC::TEXT_DIM),
                items[i]);

          
            if (is)
            {
                float cx = pp.x + width - 12, cy = iy + 11.f;
                fdl->AddLine({ cx - 3, cy }, { cx - 1, cy + 2.5f }, IM_COL32(0x48, 0xcf, 0xee, 0xFF), 1.5f);
                fdl->AddLine({ cx - 1, cy + 2.5f }, { cx + 3, cy - 3 }, IM_COL32(0x48, 0xcf, 0xee, 0xFF), 1.5f);
            }

            if (ih && ImGui::IsMouseClicked(0))
            {
                sel = i; changed = true; g_combo_open = false;
            }
        }

        fdl->PopClipRect();

        if (count > 5)
        {
            float ms = 22.f * (count - 5);
            float tr = ph / float(count * 22.f);
            float th = ph * tr;
            float ty = pp.y + (g_combo_scroll / ms) * (ph - th);
            fdl->AddRectFilled({ pp.x + width - 3, pp.y }, { pp.x + width, pp.y + ph }, MC::BORDER_DIM);
            fdl->AddRectFilled({ pp.x + width - 3, ty }, { pp.x + width, ty + th }, MC::TEXT_MUTED);
        }

        if (ImGui::IsMouseClicked(0)
            && !ImRect(pp, { pp.x + width, pp.y + ph }).Contains(io.MousePos)
            && !bb.Contains(io.MousePos))
            g_combo_open = false;

        return changed;
    }

    bool HotkeyRow(const char* label, unsigned int* key)
    {
        ImGuiWindow* win = ImGui::GetCurrentWindow();
        if (win->SkipItems) return false;

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2      pos = ImGui::GetCursorScreenPos();
        float       avail = ImGui::GetContentRegionAvail().x;
        float       fh = ImGui::GetFontSize();

        dl->AddText(pos, MC::TEXT_DIM, label);

        char ks[32] = "none";
        if (*key)
        {
            if (*key >= 'A' && *key <= 'Z') ImFormatString(ks, sizeof(ks), "%c", *key);
            else if (*key >= '0' && *key <= '9') ImFormatString(ks, sizeof(ks), "%c", *key);
            else switch (*key)
            {
            case VK_INSERT:  ImFormatString(ks, sizeof(ks), "INS");  break;
            case VK_DELETE:  ImFormatString(ks, sizeof(ks), "DEL");  break;
            case VK_HOME:    ImFormatString(ks, sizeof(ks), "HOME"); break;
            case VK_END:     ImFormatString(ks, sizeof(ks), "END");  break;
            case VK_SPACE:   ImFormatString(ks, sizeof(ks), "SPC");  break;
            case VK_RETURN:  ImFormatString(ks, sizeof(ks), "ENT");  break;
            case VK_ESCAPE:  ImFormatString(ks, sizeof(ks), "ESC");  break;
            case VK_LBUTTON: ImFormatString(ks, sizeof(ks), "M1");   break;
            case VK_RBUTTON: ImFormatString(ks, sizeof(ks), "M2");   break;
            case VK_MBUTTON: ImFormatString(ks, sizeof(ks), "M3");   break;
            default:         ImFormatString(ks, sizeof(ks), "0x%02X", *key); break;
            }
        }

        ImVec2 ksz = ImGui::CalcTextSize(ks);
        float  bpx = 6.f, bpy = 2.f;
        ImVec2 bmin(pos.x + avail - ksz.x - bpx * 2.f, pos.y - bpy);
        ImVec2 bmax(pos.x + avail, bmin.y + ksz.y + bpy * 2.f);
        dl->AddRectFilled(bmin, bmax, MC::INPUT_BG, 3.f);
        dl->AddRect(bmin, bmax, MC::INPUT_BDR, 3.f, 0, 1.f);
        dl->AddText({ bmin.x + bpx, bmin.y + bpy }, NL::GRAD_R, ks);

        ImGui::PushID(label);
        ImGui::SetCursorScreenPos(pos);
        ImGui::InvisibleButton("##hk_area", ImVec2(avail, fh + 4.f));
        bool changed = false;
        if (ImGui::HotKey("##hotkey", key)) changed = true;
        ImGui::PopID();
        ImGui::Dummy(ImVec2(avail, 2.f));
        return changed;
    }


    bool IconBtn(const char* label, ImVec2 sz,
        bool primary = false, bool danger = false)
    {
        ImGui::PushStyleColor(ImGuiCol_Button,
            primary ? MC::V4(NL::BTN_PRI) : MC::V4(NL::BTN_BG));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
            primary ? MC::V4(NL::BTN_PRI_HOV)
            : (danger ? ImVec4(0.20f, 0.06f, 0.06f, 1.f) : MC::V4(NL::BTN_HOV)));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, MC::V4(MC::ACCENT));
        ImGui::PushStyleColor(ImGuiCol_Border, MC::V4(NL::BTN_BDR));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.f);

        bool r = ImGui::Button(label, sz);

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(4);
        return r;
    }

} 

#define VARS_BOOL(x)  C_GET(bool,         Vars.x)
#define VARS_INT(x)   C_GET(int,          Vars.x)
#define VARS_FLOAT(x) C_GET(float,        Vars.x)
#define VARS_COL(x)   C_GET(Color_t,      Vars.x)
#define VARS_UINT(x)  C_GET(unsigned int, Vars.x)

#define BEGIN_PANEL(id, title, enabled_bool, cw, ch)                              \
    {                                                                              \
        ImGui::PushStyleVar  (ImGuiStyleVar_WindowPadding, ImVec2(10.f, 8.f));    \
        ImGui::PushStyleVar  (ImGuiStyleVar_ChildRounding, 4.f);                  \
        ImGui::PushStyleColor(ImGuiCol_ChildBg, MC::V4(MC::BG_PANEL));            \
        ImGui::PushStyleColor(ImGuiCol_Border,  MC::V4(MC::BORDER));              \
        ImGui::BeginChild(id, ImVec2(cw, ch), true,                               \
            ImGuiWindowFlags_AlwaysVerticalScrollbar);                             \
        ImGui::PopStyleColor(2);                                                   \
        ImGui::PopStyleVar(2);                                                     \
        {                                                                          \
            ImVec2 _cp = ImGui::GetWindowPos();                                    \
            ImVec2 _cs = ImGui::GetWindowSize();                                   \
            ImDrawList* _dl = ImGui::GetWindowDrawList();                          \
            W::PanelHeader(_dl, _cp, _cs, title, enabled_bool);                   \
            ImGui::SetCursorPosY(28.f + 8.f);                                      \
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.f, 9.f));     \
        }                                                                          \
        ImGui::PushStyleColor(ImGuiCol_ScrollbarBg,          IM_COL32(0,0,0,0));  \
        ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab,        IM_COL32(0,0,0,0));  \
        ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, IM_COL32(0,0,0,0));  \
        ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive,  IM_COL32(0,0,0,0));  \
    }

#define END_PANEL()                        \
    {                                      \
        ImGui::PopStyleColor(4);           \
        ImGui::PopStyleVar();              \
        ImGui::EndChild();                 \
    }


static int s_mainTab = 0;


static void TabAimbot(float cw, float ch)
{
    const float PAD = 8.f, GAP = 6.f;
    float col_w = (cw - PAD * 2.f - GAP) * 0.5f;
    ImGui::SetCursorPos({ PAD, ImGui::GetCursorPosY() });

    BEGIN_PANEL("##p_aimbot", "Aimbot", VARS_BOOL(bRagebot), col_w, ch - PAD)
        W::Toggle("Enabled", &VARS_BOOL(bRagebot));
    W::Toggle("Auto Shoot", &VARS_BOOL(bRagebotAutoShoot));
    W::Toggle("Auto Wall", &VARS_BOOL(bRagebotAutoWall));

    W::Section("targeting");
    W::Toggle("Draw FOV", &VARS_BOOL(bDrawFov));
    ImGui::ColorEdit4("FOV Color", &C_GET(Color_t, Vars.colFov));
    {
        float v = VARS_FLOAT(flAimRange);
        if (W::SliderRow("FOV", &v, 1.f, 360.f, "\xc2\xb0", 1.f))
            VARS_FLOAT(flAimRange) = v;
    }

    W::Section("accuracy");
    {
        float v = VARS_FLOAT(flRagebotMinDamage);
        if (W::SliderRow("Min damage", &v, 1.f, 100.f, "", 1.f))
            VARS_FLOAT(flRagebotMinDamage) = v;
    }
    {
        float v = VARS_FLOAT(flRageHitChance);
        if (W::SliderRow("Hitchance", &v, 0.f, 100.f, "%", 1.f))
            VARS_FLOAT(flRageHitChance) = v;
    }
    {
        static const char* modes[] = { "seed", "hit chance" };
        W::Combo("##mode", "Spread mode", modes, IM_ARRAYSIZE(modes), VARS_INT(nRagebotSpreadMode));
    }
    END_PANEL()

        ImGui::SameLine(0.f, GAP);

    BEGIN_PANEL("##p_triggerbot", "Triggerbot", VARS_BOOL(bTriggerBot), col_w, ch - PAD)
        W::Toggle("Enabled", &VARS_BOOL(bTriggerBot));
    W::Toggle("Auto Wall", &VARS_BOOL(bTriggerAutoWall));
    W::Toggle("Always active", &VARS_BOOL(bTriggerAlways));
    W::HotkeyRow("Trigger key", &VARS_UINT(nTriggerKey));

    W::Section("timing");
    {
        int v = VARS_INT(nReactionTime);
        if (W::SliderRowInt("Reaction time", &v, 0, 500, "ms")) VARS_INT(nReactionTime) = v;
    }
    {
        int v = VARS_INT(nTriggerDelay);
        if (W::SliderRowInt("Delay", &v, 0, 500, "ms")) VARS_INT(nTriggerDelay) = v;
    }
    END_PANEL()
}


static void TabVisuals(float cw, float ch)
{
    const float PAD = 8.f, GAP = 6.f;
    float col_w = (cw - PAD * 2.f - GAP) * 0.5f;
    ImGui::SetCursorPos({ PAD, ImGui::GetCursorPosY() });

    BEGIN_PANEL("##p_esp", "Players", VARS_BOOL(bBox), col_w, ch - PAD)
        W::Section("boxes");
    W::Toggle("Corner box", &VARS_BOOL(bBox));
    W::Toggle("Health bar", &VARS_BOOL(bHealthBar));
    W::Toggle("Name ESP", &VARS_BOOL(bName));
    W::Toggle("Weapon", &VARS_BOOL(bWeapon));
    W::Toggle("Skeleton", &VARS_BOOL(bSkeleton));

    W::Section("colors");
    ImGui::ColorEdit4("skeleton color", &C_GET(Color_t, Vars.colSkeleton1));
    ImGui::ColorEdit4("health color", &C_GET(Color_t, Vars.colHealth1));
    ImGui::ColorEdit4("health color 2", &C_GET(Color_t, Vars.colHealth2));
    ImGui::ColorEdit4("box color", &C_GET(Color_t, Vars.colBox1));
    ImGui::ColorEdit4("weapon color", &C_GET(Color_t, Vars.colWeaponName));
    ImGui::ColorEdit4("playername color", &C_GET(Color_t, Vars.colPlayerName));

    W::Section("health text");
    W::Toggle("health text", &VARS_BOOL(bHealthText));

    W::Section("chams");
    W::Toggle("Chams", &VARS_BOOL(bVisualChams));
    W::Toggle("Weapon chams", &VARS_BOOL(bVisualChamsWeapon));
    ImGui::ColorEdit4("chams color", &C_GET(Color_t, Vars.colVisualChams));
    {
        static const char* mats[] = { "White","Illuminate","Latex","Ghost" };
        W::Combo("##cham_mat", "Material", mats, IM_ARRAYSIZE(mats),
            C_GET(int, Vars.nVisualChamMaterial));
    }
    END_PANEL()

        ImGui::SameLine(0.f, GAP);

    BEGIN_PANEL("##p_world", "World", VARS_BOOL(bVisualChams), col_w, ch - PAD)
        W::Section("world");
    W::Toggle("Inferno Radius", &VARS_BOOL(bInfernoRadius));
    W::Toggle("Smoke Radius", &VARS_BOOL(bSmokeRadius));
    ImGui::ColorEdit4("smoke fill", &C_GET(Color_t, Vars.colSmokeFill));
    ImGui::ColorEdit4("smoke outline", &C_GET(Color_t, Vars.colSmokeOutline));
    ImGui::ColorEdit4("inferno fill", &C_GET(Color_t, Vars.colInfernoFill));
    ImGui::ColorEdit4("inferno outline", &C_GET(Color_t, Vars.colInfernoOutline));
    END_PANEL()
}


static void TabMisc(float cw, float ch)
{
    const float PAD = 8.f, GAP = 6.f;
    float col_w = (cw - PAD * 2.f - GAP) * 0.5f;
    ImGui::SetCursorPos({ PAD, ImGui::GetCursorPosY() });

    BEGIN_PANEL("##p_misc_move", "Movement", true, col_w, ch - PAD)
        W::Section("movement");
    W::Toggle("Bunny hop", &VARS_BOOL(bBhop));

    W::Section("viewmodel");
    W::Toggle("Viewmodel", &VARS_BOOL(viewmodell));
    {
        float v = VARS_FLOAT(FFov);
        if (W::SliderRow("FOV", &v, 20.f, 180.f, "\xc2\xb0", 1.f)) VARS_FLOAT(FFov) = v;
    }
    {
        float v = VARS_FLOAT(viewx);
        if (W::SliderRow("View X", &v, -20.f, 20.f, "", 0.5f)) VARS_FLOAT(viewx) = v;
    }
    {
        float v = VARS_FLOAT(viewy);
        if (W::SliderRow("View Y", &v, -20.f, 20.f, "", 0.5f)) VARS_FLOAT(viewy) = v;
    }
    {
        float v = VARS_FLOAT(viewz);
        if (W::SliderRow("View Z", &v, -20.f, 20.f, "", 0.5f)) VARS_FLOAT(viewz) = v;
    }
    {
        float v = VARS_FLOAT(viewfov);
        if (W::SliderRow("View FOV", &v, 40.f, 120.f, "\xc2\xb0", 1.f)) VARS_FLOAT(viewfov) = v;
    }
    END_PANEL()

        ImGui::SameLine(0.f, GAP);

    BEGIN_PANEL("##p_misc_other", "Other", true, col_w, ch - PAD)
        W::Section("utility");
    W::Toggle("Anti-flash", &VARS_BOOL(bAntiFlash));
    W::Toggle("Anti-smoke", &VARS_BOOL(bAntiSmoke));
    END_PANEL()
}
static void TabSettings(float cw, float ch)
{
    const float PAD = 12.f;
    const float LIST_W = 220.f;
    const float RIGHT_W = cw - LIST_W - PAD * 3.f;
    const float FULL_H = ch - PAD * 2.f;

    ImGui::SetCursorPos(ImVec2(PAD, PAD));
    ImGui::BeginGroup();
    {
     
        const float SRH = 28.f;
        ImVec2 srp = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();

        dl->AddRect(srp,
            { srp.x + LIST_W, srp.y + SRH },
            MC::BORDER, 0.f, 0, 1.f);

        float fh = ImGui::GetFontSize();
        dl->AddText({ srp.x + 8.f, srp.y + (SRH - fh) * 0.5f },
            MC::TEXT_MUTED, CS_XOR("?"));

        ImGui::SetCursorPos({ PAD + 22.f, PAD + (SRH - fh) * 0.5f });
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
        ImGui::PushItemWidth(LIST_W - 22.f - 4.f);
        static char szSearch[128] = {};
        ImGui::InputTextWithHint(CS_XOR("##cfg_search"),
            CS_XOR("Search configs..."),
            szSearch, sizeof(szSearch));
        ImGui::PopItemWidth();
        ImGui::PopStyleColor(2);

        ImGui::SetCursorPos({ PAD, PAD + SRH + 6.f });

        if (T::nSelectedConfig == ~1U)
        {
            for (std::size_t i = 0; i < C::vecFileNames.size(); i++) {
                if (CRT::StringCompare(C::vecFileNames[i],
                    CS_XOR(CS_CONFIGURATION_DEFAULT_FILE_NAME CS_CONFIGURATION_FILE_EXTENSION)) == 0)
                {
                    T::nSelectedConfig = i; break;
                }
            }
        }


        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.f, 4.f));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, MC::V4(MC::BG_PANEL));
        ImGui::PushStyleColor(ImGuiCol_Border, MC::V4(MC::BORDER));
        ImGui::BeginChild(CS_XOR("##cfg_list_scroll"),
            { LIST_W, FULL_H - SRH - 6.f }, true,
            ImGuiWindowFlags_NoScrollbar);
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar();

        for (std::size_t i = 0; i < C::vecFileNames.size(); i++)
        {
            char sz[MAX_PATH] = {};
            CRT::StringUnicodeToMultiByte(sz, CS_ARRAYSIZE(sz), C::vecFileNames[i]);
            _strlwr_s(sz, MAX_PATH);

            if (szSearch[0] != '\0' && !strstr(sz, szSearch))
                continue;

            bool selected = (T::nSelectedConfig == i);
            ImVec2 itemPos = ImGui::GetCursorScreenPos();
            ImDrawList* idl = ImGui::GetWindowDrawList();
            float itemW = ImGui::GetContentRegionAvail().x;
            float itemH = ImGui::GetFontSize() + 10.f;

            if (selected)
                idl->AddRectFilled(itemPos,
                    { itemPos.x + itemW, itemPos.y + itemH },
                    NL::BG_ITEM_SEL, 0.f); 

            idl->AddText(
                { itemPos.x + 6.f, itemPos.y + (itemH - ImGui::GetFontSize()) * 0.5f },
                selected ? MC::ACCENT : MC::TEXT_MUTED, CS_XOR("*"));

            idl->AddText(
                { itemPos.x + 22.f, itemPos.y + (itemH - ImGui::GetFontSize()) * 0.5f },
                selected ? MC::TEXT : MC::TEXT_DIM, sz);

            ImGui::Dummy({ itemW, itemH });
            ImRect r(itemPos, { itemPos.x + itemW, itemPos.y + itemH });
            if (r.Contains(ImGui::GetMousePos()) && ImGui::IsMouseClicked(0))
                T::nSelectedConfig = i;
        }
        ImGui::EndChild();
    }
    ImGui::EndGroup();

    ImGui::SameLine(0.f, PAD);

    
    ImGui::BeginGroup();
    {
        const float BTN_H = 30.f;
        const float BTN_W = (RIGHT_W - 6.f * 2.f) / 3.f;
        float startY = PAD; 

        ImGui::SetCursorPosY(startY);

      
        if (W::IconBtn(CS_XOR("Load"), ImVec2(BTN_W, BTN_H), true))
            C::LoadFile(T::nSelectedConfig);
        ImGui::SameLine(0.f, 6.f);
        if (W::IconBtn(CS_XOR("Save"), ImVec2(BTN_W, BTN_H)))
            C::SaveFile(T::nSelectedConfig);
        ImGui::SameLine(0.f, 6.f);
        if (W::IconBtn(CS_XOR("Delete"), ImVec2(BTN_W, BTN_H), false, true))
            ImGui::OpenPopup(CS_XOR("confirm##rm"));

        ImGui::SameLine(0.f, 6.f);


        if (W::IconBtn(CS_XOR("New"), { BTN_W, BTN_H }))
            ImGui::OpenPopup(CS_XOR("new_cfg##pop"));

        ImGui::SetCursorPosY(startY + BTN_H + 12.f);

   
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 sep = ImGui::GetCursorScreenPos();
        dl->AddLine(sep, { sep.x + RIGHT_W, sep.y }, MC::BORDER, 1.f);
        ImGui::Dummy({ RIGHT_W, 1.f });
        ImGui::Spacing();

  
        ImGui::Dummy({ RIGHT_W, FULL_H - BTN_H - 20.f });
    }
    ImGui::EndGroup();


    ImGui::SetNextWindowSize({ 260.f, 0.f }, ImGuiCond_Always);
    if (ImGui::BeginPopupModal(CS_XOR("confirm##rm"), nullptr,
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar))
    {
        ImGui::Spacing();
        ImGui::TextColored(MC::V4(MC::TEXT), CS_XOR(" Delete selected config?"));
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        if (W::IconBtn(CS_XOR("Yes, delete"), { 120.f, 28.f }, false, true))
        {
            if (T::nSelectedConfig < C::vecFileNames.size())
            {
                C::RemoveFile(T::nSelectedConfig);
                T::nSelectedConfig = C::vecFileNames.empty() ? ~0U : 0;
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine(0.f, 6.f);
        if (W::IconBtn(CS_XOR("Cancel"), { 120.f, 28.f }))
            ImGui::CloseCurrentPopup();
        ImGui::Spacing();
        ImGui::EndPopup();
    }

    
    ImGui::SetNextWindowSize({ 300.f, 0.f }, ImGuiCond_Always);
    if (ImGui::BeginPopupModal(CS_XOR("new_cfg##pop"), nullptr,
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar))
    {
        ImGui::Spacing();
        ImGui::TextColored(MC::V4(MC::TEXT), CS_XOR(" New config name:"));
        ImGui::Spacing();
        ImGui::PushItemWidth(-1.f);
        bool enter = ImGui::InputTextWithHint(CS_XOR("##new_name"),
            CS_XOR("config name..."),
            T::szConfigFile, sizeof(T::szConfigFile),
            ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::PopItemWidth();
        ImGui::Spacing();
        if (enter || W::IconBtn(CS_XOR("Create"), { 140.f, 28.f }, true))
        {
            if (CRT::StringLength(T::szConfigFile) > 0)
            {
                CRT::WString_t ws(T::szConfigFile);
                if (C::CreateFile(ws.Data()))
                    T::nSelectedConfig = C::vecFileNames.size() - 1;
                CRT::MemorySet(T::szConfigFile, 0, sizeof(T::szConfigFile));
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine(0.f, 6.f);
        if (W::IconBtn(CS_XOR("Cancel"), { 140.f, 28.f }))
        {
            CRT::MemorySet(T::szConfigFile, 0, sizeof(T::szConfigFile));
            ImGui::CloseCurrentPopup();
        }
        ImGui::Spacing();
        ImGui::EndPopup();
    }
}

void MENU::RenderMainWindow()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();

    animMenuDimBackground.Update(io.DeltaTime, style.AnimationSpeed);
    if (!bMainWindowOpened) return;

    constexpr float WIN_W = 700.f;
    constexpr float WIN_H = 520.f;
    constexpr float SIDEBAR_W = 180.f;
    constexpr float LOGO_H = 62.f;
    constexpr float TOPBAR_H = 45.f;
    constexpr float FOOTER_H = 58.f;
    constexpr float WIN_ROUND = 10.f;
    constexpr float ITEM_H = 36.f;
    constexpr float SUB_H = 28.f;

    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, animMenuDimBackground.GetValue(1.f));

    ImGui::SetNextWindowPos(
        { io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f },
        ImGuiCond_Once, { 0.5f, 0.5f });
    ImGui::SetNextWindowSize({ WIN_W, WIN_H }, ImGuiCond_Always);

    if (ImGuiWindow* pw = ImGui::FindWindowByName("cheat##main"))
    {
        ImVec2 p = pw->Pos; bool fix = false;
        if (p.x < 0.f) { p.x = 0.f;                    fix = true; }
        if (p.x + WIN_W > io.DisplaySize.x) { p.x = io.DisplaySize.x - WIN_W; fix = true; }
        if (p.y < 0.f) { p.y = 0.f;                    fix = true; }
        if (p.y + WIN_H > io.DisplaySize.y) { p.y = io.DisplaySize.y - WIN_H; fix = true; }
        if (fix) ImGui::SetNextWindowPos(p, ImGuiCond_Always);
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, WIN_ROUND);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));

    ImGui::Begin("cheat##main", &bMainWindowOpened,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBackground);

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);

    ImVec2      wp = ImGui::GetWindowPos();
    ImVec2      ws = ImGui::GetWindowSize();
    ImDrawList* bg = ImGui::GetWindowDrawList();

 
    for (int i = 0; i < 12; i++)
    {
        float a = 55.f - i * 4.5f;
        bg->AddRect({ wp.x - i, wp.y - i }, { wp.x + ws.x + i, wp.y + ws.y + i },
            IM_COL32(0, 0, 0, (int)a), WIN_ROUND + i, 0, 1.f);
    }

    
    bg->AddRectFilled(wp, { wp.x + ws.x, wp.y + ws.y },
        IM_COL32(0x0d, 0x0d, 0x0d, 0xFF), WIN_ROUND);
    bg->AddRect(wp, { wp.x + ws.x, wp.y + ws.y },
        IM_COL32(0x22, 0x22, 0x22, 0xFF), WIN_ROUND, 0, 1.f);


    bg->AddRectFilled(wp, { wp.x + SIDEBAR_W, wp.y + ws.y },
        IM_COL32(0x0a, 0x0a, 0x0a, 0xFF), WIN_ROUND, ImDrawFlags_RoundCornersLeft);
    bg->AddLine({ wp.x + SIDEBAR_W, wp.y },
        { wp.x + SIDEBAR_W, wp.y + ws.y }, IM_COL32(0x22, 0x22, 0x22, 0xFF), 1.f);

    
    bg->AddLine({ wp.x, wp.y + LOGO_H },
        { wp.x + SIDEBAR_W, wp.y + LOGO_H }, IM_COL32(0x22, 0x22, 0x22, 0xFF), 1.f);

 
    bg->AddRectFilled({ wp.x + SIDEBAR_W, wp.y }, { wp.x + ws.x, wp.y + TOPBAR_H },
        IM_COL32(0x0d, 0x0d, 0x0d, 0xFF), WIN_ROUND, ImDrawFlags_RoundCornersTopRight);
    bg->AddLine({ wp.x + SIDEBAR_W, wp.y + TOPBAR_H },
        { wp.x + ws.x,     wp.y + TOPBAR_H }, IM_COL32(0x22, 0x22, 0x22, 0xFF), 1.f);


    bg->AddLine({ wp.x, wp.y + ws.y - FOOTER_H },
        { wp.x + SIDEBAR_W, wp.y + ws.y - FOOTER_H }, IM_COL32(0x22, 0x22, 0x22, 0xFF), 1.f);

    {
        const float LOGO_SZ = 28.f, LR = 5.f;
        ImVec2 lMin(wp.x + 16.f, wp.y + (LOGO_H - LOGO_SZ) * 0.5f);
        ImVec2 lMax(lMin.x + LOGO_SZ, lMin.y + LOGO_SZ);

        bg->AddRectFilled(lMin, lMax, IM_COL32(0x18, 0x18, 0x18, 0xFF), LR);
        bg->AddRect(lMin, lMax, IM_COL32(0x38, 0x38, 0x38, 0x88), LR, 0, 1.f);

        const char* logoTxt = "M";
        ImVec2 lsz = ImGui::CalcTextSize(logoTxt);
        bg->AddText({ lMin.x + (LOGO_SZ - lsz.x) * 0.5f,
                      lMin.y + (LOGO_SZ - lsz.y) * 0.5f },
            NL::GRAD_R, logoTxt);

        float fh = ImGui::GetFontSize();
        float tx = lMax.x + 10.f;
        float ty = wp.y + LOGO_H * 0.5f;
        bg->AddText({ tx, ty - fh * 0.65f }, MC::TEXT, "marusense");
        bg->AddText({ tx, ty + fh * 0.05f }, MC::TEXT_DIM, "v1.0.0");
    }

  
    {
        static const char* tabNames[] = {
            "Aimbot","Visuals","Misc","Skins","Scripts","Settings","Hotkeys"
        };

        char topTitle[64] = {};
        ImFormatString(topTitle, sizeof(topTitle), "%s", tabNames[s_mainTab]);

        float fh = ImGui::GetFontSize();
        float dotX = wp.x + SIDEBAR_W + 16.f;
        float dotY = wp.y + TOPBAR_H * 0.5f;

        bg->AddCircleFilled({ dotX + 4.f, dotY }, 4.f, IM_COL32(0x60, 0x60, 0x60, 0xFF));
        bg->AddText({ dotX + 14.f, dotY - fh * 0.5f }, MC::TEXT, topTitle);
    }


    {
        constexpr float AVA_R = 15.f;
        ImVec2 avaCenter(wp.x + 16.f + AVA_R, wp.y + ws.y - FOOTER_H * 0.5f);

        bg->AddCircleFilled(avaCenter, AVA_R, NL::AVATAR_BG);
        bg->AddCircle(avaCenter, AVA_R, NL::AVATAR_BDR, 32, 1.2f);

        const char* ava = "M";
        ImVec2 asz = ImGui::CalcTextSize(ava);
        bg->AddText({ avaCenter.x - asz.x * 0.5f, avaCenter.y - asz.y * 0.5f },
            MC::TEXT, ava);

        float fh = ImGui::GetFontSize();
        float tx = wp.x + 16.f + AVA_R * 2.f + 10.f;
        float ty = wp.y + ws.y - FOOTER_H * 0.5f;
        bg->AddText({ tx, ty - fh * 0.65f }, MC::TEXT, "marusense");
        bg->AddText({ tx, ty + fh * 0.05f }, MC::TEXT_DIM, "expires: N/A");
    }


    ImGui::SetCursorPos({ 0.f, LOGO_H + 8.f });
    ImGui::BeginChild("##sidebar",
        { SIDEBAR_W, ws.y - LOGO_H - FOOTER_H - 8.f },
        false,
        ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);

    {
        struct NavItem { const char* label; int idx; };
        static const NavItem NAV[] = {
            { "Aimbot",   0 },
            { "Visuals",  1 },
            { "Misc",     2 },
            { "Skins",    3 },
            { "Scripts",  4 },
            { "Settings", 5 },
            { "Hotkeys",  6 },
        };

        ImDrawList* dl = ImGui::GetWindowDrawList();
        const float fh = ImGui::GetFontSize();

        for (const auto& n : NAV)
        {
            bool   active = (s_mainTab == n.idx);
            ImVec2 p = ImGui::GetCursorScreenPos();
            bool   hov = ImRect(p, { p.x + SIDEBAR_W, p.y + ITEM_H })
                .Contains(ImGui::GetMousePos());

            if (active)
            {
              
                dl->AddRectFilled(
                    p,
                    { p.x + SIDEBAR_W - 6.f, p.y + ITEM_H },
                    IM_COL32(0x00, 0xb4, 0xd8, 0x28),  
                    2.f
                );

           
                dl->AddRectFilled(
                    { p.x, p.y + 4.f },
                    { p.x + 2.f, p.y + ITEM_H - 4.f },
                    IM_COL32(0x58, 0x58, 0x68, 0xFF),
                    2.f
                );
            }
            else if (hov)
                dl->AddRectFilled(p, { p.x + SIDEBAR_W - 6.f, p.y + ITEM_H },
                    NL::BG_ITEM_HOV, 6.f);

            ImU32 col = active ? MC::TEXT : (hov ? MC::TEXT : MC::TEXT_DIM);
            dl->AddText({ p.x + 16.f, p.y + (ITEM_H - fh) * 0.5f },
                col, n.label);

            char nid[32];
            ImFormatString(nid, sizeof(nid), "##nav_%d", n.idx);
            if (ImGui::InvisibleButton(nid, { SIDEBAR_W, ITEM_H }))
                s_mainTab = n.idx;
        }
    }

    ImGui::EndChild();


    ImGui::SetCursorPos({ SIDEBAR_W + 16.f, TOPBAR_H + 16.f });
    ImGui::BeginChild("##content",
        { WIN_W - SIDEBAR_W - 32.f, WIN_H - TOPBAR_H - 32.f },
        false,
        ImGuiWindowFlags_NoBackground);

    {
        const float cw = WIN_W - SIDEBAR_W - 32.f;
        const float ch = WIN_H - TOPBAR_H - 32.f;

        switch (s_mainTab)
        {
        case 0: TabAimbot(cw, ch); break;
        case 1: TabVisuals(cw, ch); break;
        case 2: TabMisc(cw, ch); break;
        case 5: TabSettings(cw, ch); break;
        default:
        {
            ImVec2 tsz = ImGui::CalcTextSize("coming soon");
            ImGui::SetCursorPos({ (cw - tsz.x) * 0.5f, (ch - tsz.y) * 0.5f });
            ImGui::TextColored(MC::V4(MC::TEXT_DIM), "coming soon");
        }
        break;
        }
    }

    ImGui::EndChild();
    ImGui::End();
    ImGui::PopStyleVar();
}


void MENU::UpdateStyle(ImGuiStyle* pStyle)
{
    ImGuiStyle& s = pStyle ? *pStyle : ImGui::GetStyle();

    s.WindowPadding = ImVec2(0.f, 0.f);
    s.FramePadding = ImVec2(10.f, 6.f);
    s.ItemSpacing = ImVec2(8.f, 8.f);
    s.ItemInnerSpacing = ImVec2(6.f, 4.f);
    s.CellPadding = ImVec2(6.f, 6.f);
    s.TouchExtraPadding = ImVec2(0.f, 0.f);
    s.IndentSpacing = 20.f;
    s.ScrollbarSize = 6.f;
    s.GrabMinSize = 10.f;
    s.WindowRounding = 18.f;
    s.ChildRounding = 16.f;
    s.FrameRounding = 12.f;
    s.PopupRounding = 14.f;
    s.ScrollbarRounding = 12.f;
    s.GrabRounding = 12.f;
    s.TabRounding = 12.f;
    s.LogSliderDeadzone = 4.f;
    s.WindowBorderSize = 1.f;
    s.ChildBorderSize = 1.f;
    s.PopupBorderSize = 1.f;
    s.FrameBorderSize = 1.f;
    s.TabBorderSize = 0.f;
    s.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    s.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    s.SelectableTextAlign = ImVec2(0.f, 0.5f);
    s.DisplaySafeAreaPadding = ImVec2(4.f, 4.f);

    const ImVec4 acc = MC::V4(MC::ACCENT);
    const ImVec4 achi = MC::V4(MC::ACCENT_HI);
    const ImVec4 bg0 = MC::V4(MC::BG_WINDOW);
    const ImVec4 bg1 = MC::V4(MC::BG_PANEL);
    const ImVec4 bdr = MC::V4(MC::BORDER);
    const ImVec4 txt = MC::V4(MC::TEXT);
    const ImVec4 dim = MC::V4(MC::TEXT_DIM);
    const ImVec4 inp = MC::V4(MC::INPUT_BG);

    auto alpha = [](ImVec4 c, float a) { return ImVec4(c.x, c.y, c.z, a); };

    s.Colors[ImGuiCol_Text] = txt;
    s.Colors[ImGuiCol_TextDisabled] = dim;
    s.Colors[ImGuiCol_WindowBg] = bg0;
    s.Colors[ImGuiCol_ChildBg] = bg1;
    s.Colors[ImGuiCol_PopupBg] = MC::V4(MC::INPUT_BG);
    s.Colors[ImGuiCol_Border] = alpha(bdr, 0.70f);
    s.Colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);
    s.Colors[ImGuiCol_FrameBg] = inp;
    s.Colors[ImGuiCol_FrameBgHovered] = alpha(acc, 0.18f);
    s.Colors[ImGuiCol_FrameBgActive] = alpha(acc, 0.28f);
    s.Colors[ImGuiCol_TitleBg] = bg0;
    s.Colors[ImGuiCol_TitleBgActive] = bg0;
    s.Colors[ImGuiCol_TitleBgCollapsed] = bg0;
    s.Colors[ImGuiCol_Button] = inp;
    s.Colors[ImGuiCol_ButtonHovered] = alpha(acc, 0.22f);
    s.Colors[ImGuiCol_ButtonActive] = alpha(acc, 0.34f);
    s.Colors[ImGuiCol_Header] = alpha(acc, 0.12f);
    s.Colors[ImGuiCol_HeaderHovered] = alpha(acc, 0.20f);
    s.Colors[ImGuiCol_HeaderActive] = alpha(acc, 0.30f);
    s.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0, 0, 0, 0);
    s.Colors[ImGuiCol_ScrollbarGrab] = alpha(acc, 0.28f);
    s.Colors[ImGuiCol_ScrollbarGrabHovered] = alpha(acc, 0.42f);
    s.Colors[ImGuiCol_ScrollbarGrabActive] = acc;
    s.Colors[ImGuiCol_CheckMark] = achi;
    s.Colors[ImGuiCol_SliderGrab] = acc;
    s.Colors[ImGuiCol_SliderGrabActive] = achi;
    s.Colors[ImGuiCol_Separator] = alpha(bdr, 0.55f);
    s.Colors[ImGuiCol_SeparatorHovered] = alpha(acc, 0.50f);
    s.Colors[ImGuiCol_SeparatorActive] = acc;
    s.Colors[ImGuiCol_ResizeGrip] = ImVec4(0, 0, 0, 0);
    s.Colors[ImGuiCol_ResizeGripHovered] = alpha(acc, 0.34f);
    s.Colors[ImGuiCol_ResizeGripActive] = acc;
    s.Colors[ImGuiCol_Tab] = alpha(acc, 0.12f);
    s.Colors[ImGuiCol_TabHovered] = alpha(acc, 0.30f);
    s.Colors[ImGuiCol_TabActive] = alpha(acc, 0.45f);
    s.Colors[ImGuiCol_TabUnfocused] = alpha(acc, 0.08f);
    s.Colors[ImGuiCol_TabUnfocusedActive] = alpha(acc, 0.22f);
    s.Colors[ImGuiCol_TextSelectedBg] = alpha(acc, 0.30f);
    s.Colors[ImGuiCol_PlotLines] = achi;
    s.Colors[ImGuiCol_PlotHistogram] = achi;
    s.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.f, 0.f, 0.f, 0.55f);
    s.Colors[ImGuiCol_NavHighlight] = alpha(acc, 0.80f);

    s.AnimationSpeed =
        (C_GET(float, Vars.flAnimationSpeed) > 0.f)
        ? C_GET(float, Vars.flAnimationSpeed) / 10.f
        : 0.5f;
}

#undef VARS_BOOL
#undef VARS_INT
#undef VARS_FLOAT
#undef VARS_COL
#undef VARS_UINT
#undef BEGIN_PANEL
#undef END_PANEL