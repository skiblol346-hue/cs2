#include "config.hpp"
#include "../../gui.hpp"
#include "../../framework/shell/shell.hpp"
#include "../../framework/keybind/keybind.hpp"
#include "../../framework/styling.hpp"
#include <Windows.h>
#include "core/variables.h"
#include "core/config.h"

namespace pages
{
    void RenderConfig() noexcept
    {
        if (ksd::BeginChild("Presets", { 0, 0, 4.5f, 10 }))
        {
            static int selectedPreset = 0;

            static char displayNames[32][64];
            static const char* displayPtrs[32];
            int count = (int)C::vecFileNames.size();
            if (count > 32) count = 32;
            for (int i = 0; i < count; i++)
            {
                WideCharToMultiByte(CP_UTF8, 0, C::vecFileNames[i], -1, displayNames[i], 64, NULL, NULL);
                char* dot = strrchr(displayNames[i], '.');
                if (dot) *dot = '\0';
                displayPtrs[i] = displayNames[i];
            }
            if (selectedPreset >= count) selectedPreset = 0;

            ksd::ListBox("##presets_list", &selectedPreset, displayPtrs, count);

            static char presetName[64] = "";
            ksd::InputText("##preset_name", presetName, sizeof(presetName));

            const float btnW = ksd::ContentWidth();
            if (ksd::Button("Load", ImVec2(btnW, 25.f)))
            {
                if (selectedPreset < count)
                    C::LoadFile(selectedPreset);
            }
            if (ksd::Button("Save", ImVec2(btnW, 25.f)))
            {
                if (selectedPreset < count)
                    C::SaveFile(selectedPreset);
            }
            if (ksd::Button("Create", ImVec2(btnW, 25.f)))
            {
                if (presetName[0])
                {
                    wchar_t wName[64];
                    MultiByteToWideChar(CP_UTF8, 0, presetName, -1, wName, 64);
                    C::CreateFile(wName);
                    C::Refresh();
                    presetName[0] = '\0';
                }
            }
            if (ksd::Button("Delete", ImVec2(btnW, 25.f)))
            {
                if (selectedPreset < count)
                {
                    C::RemoveFile(selectedPreset);
                    C::Refresh();
                    selectedPreset = 0;
                }
            }
        }
        ksd::EndChild();

        if (ksd::BeginChild("Settings", { 4.5f, 0, 4.5f, 10 }))
        {
            {
                const float rowY = ksd::CursorPos().y;
                ksd::Label("Menu key");
                static int menuKeyInt = static_cast<int>(C_GET(unsigned int, Vars.nMenuKey));
                ksd::DrawKeybindButton("##menukey_kb",
                    &menuKeyInt, nullptr, rowY);
                C_GET(unsigned int, Vars.nMenuKey) = static_cast<unsigned int>(menuKeyInt);
            }

            {
                const float rowY = ksd::CursorPos().y;
                ksd::Label("Panic key");
                static int panicKeyInt = static_cast<int>(C_GET(unsigned int, Vars.nPanicKey));
                ksd::DrawKeybindButton("##panickey_kb",
                    &panicKeyInt, nullptr, rowY);
                C_GET(unsigned int, Vars.nPanicKey) = static_cast<unsigned int>(panicKeyInt);
            }

            {
                const float colorY = ksd::CursorPos().y;
                ksd::Label("Menu color");
                ksd::ColorButton("##menucolor_btn", vars::colorAccent,
                    ImVec2(ksd::PanelRightX() - 17.f, colorY + 2.f), ImVec2(16.f, 10.f));
            }

            ksd::SliderFloat("Animation speed",
                &C_GET(float, Vars.flAnimationSpeed), 0.f, 10.f, "%.1f");

            ksd::Label("Glass tuning");
            ksd::SliderInt("Outline alpha", &style::outlineAlpha, 0, 255, "%d");
            ksd::SliderFloat("Board opacity",  &style::glassWindow, 0.f, 1.f, "%.2f", 0.02f);
            ksd::SliderFloat("Panel opacity",  &style::glassPanel,  0.f, 1.f, "%.2f", 0.02f);
            ksd::SliderFloat("Widget opacity", &style::glassWidget, 0.f, 1.f, "%.2f", 0.02f);
            ksd::SliderFloat("Popup opacity",  &style::glassPopup,  0.f, 1.f, "%.2f", 0.02f);
        }
        ksd::EndChild();
    }
}
