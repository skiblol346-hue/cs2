#include "config.hpp"
#include "../../gui.hpp"
#include "../../framework/shell/shell.hpp"
#include "../../framework/keybind/keybind.hpp"
#include "../../framework/styling.hpp"

#include <Windows.h>

namespace vars
{
    int  menuKey           = VK_INSERT;
    int  menuAnimSpeed     = 100;
    bool menuConfirmations = false;
    int  menuDpiScale      = 0;
    int  menuAntiUntrusted = 0;
    bool menuStreamerMode  = false;
}

static void ResetToDefaults() noexcept
{
    vars::menuKey           = VK_INSERT;
    vars::menuAnimSpeed     = 100;
    vars::menuConfirmations = false;
    vars::menuDpiScale      = 0;
    vars::menuAntiUntrusted = 0;
    vars::menuStreamerMode  = false;

    vars::colorAccent[0] = 0.656f;
    vars::colorAccent[1] = 0.546f;
    vars::colorAccent[2] = 0.981f;
    vars::colorAccent[3] = 1.f;
}

namespace pages
{
    void RenderConfig() noexcept
    {
        static const char* kDpiOptions[] = {
            "Automatic", "100%", "125%", "150%", "175%", "200%"
        };
        static const char* kAntiOptions[] = { "Automatic", "Always", "Never" };

        if (ksd::BeginChild("Presets", { 0, 0, 3, 10 }))
        {
            static int  selectedPreset = 0;
            static const char* presets[] = { "hvh", "legit" };
            ksd::ListBox("##presets_list", &selectedPreset, presets, 2);

            static char presetName[64] = "hvh";
            ksd::InputText("##preset_name", presetName, sizeof(presetName));

            const float btnW = ksd::ContentWidth();
            if (ksd::Button("Load",   ImVec2(btnW, 25.f))) {}
            if (ksd::Button("Save",   ImVec2(btnW, 25.f))) {}
            if (ksd::Button("Delete", ImVec2(btnW, 25.f))) {}
            if (ksd::Button("Reset",  ImVec2(btnW, 25.f)))
                ResetToDefaults();
            if (ksd::Button("Import from clipboard", ImVec2(btnW, 25.f))) {}
            if (ksd::Button("Export to clipboard",   ImVec2(btnW, 25.f))) {}
        }
        ksd::EndChild();

        if (ksd::BeginChild("Settings", { 6, 0, 3, 10 }))
        {
            {
                const float rowY = ksd::CursorPos().y;
                ksd::Label("Menu key");
                static bool menuKeyOn = true;
                ksd::DrawKeybindButton("##menukey_kb",
                    &vars::menuKey, &menuKeyOn, rowY);
            }

            {
                const float colorY = ksd::CursorPos().y;
                ksd::Label("Menu color");
                ksd::ColorButton("##menucolor_btn", vars::colorAccent,
                    ImVec2(ksd::PanelRightX() - 17.f, colorY + 2.f), ImVec2(16.f, 10.f));
            }

            ksd::SliderInt("Menu animation speed",
                &vars::menuAnimSpeed, 0, 100, "%d%%");

            ksd::Checkbox("Menu confirmations", &vars::menuConfirmations);

            ksd::SingleSelect("DPI scale",
                &vars::menuDpiScale,       kDpiOptions,  6);
            ksd::SingleSelect("Anti-untrusted",
                &vars::menuAntiUntrusted,  kAntiOptions, 3);

            ksd::Checkbox("Streamer mode", &vars::menuStreamerMode);

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
