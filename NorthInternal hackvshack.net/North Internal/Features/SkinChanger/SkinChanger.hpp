#pragma once
#include "../../Dependencies/ImGui/imgui.h"
#include <vector>
#include <string>
#include <cstdint>
#include <unordered_map>

enum ERarity : int {
    RARITY_CONSUMER = 0,
    RARITY_INDUSTRIAL = 1,
    RARITY_MIL_SPEC = 2,
    RARITY_RESTRICTED = 3,
    RARITY_CLASSIFIED = 4,
    RARITY_COVERT = 5,
    RARITY_GOLD = 6,
};

struct KnifeInfo {
    int         defIndex;
    const char* name;
    const char* modelPath;
};

struct SkinInfo {
    int         defIndex;
    int         paintKit;
    const char* weaponName;
    const char* skinName;
    ERarity     rarity;
};

class CSkinChanger {
public:
    void        ApplySkins();
    void        Render();

private:
    bool        m_showKnifePicker = false;
    int         m_knifePickerCategory = 0;
    char        m_knifeSearchBuf[128] = {};
    int         m_selectedKnifeIndex = -1;
    bool        m_showSkinPicker = false;
    char        m_skinSearchBuf[128] = {};
    bool        m_hasActiveSkin = false;
    int         m_activeDefIndex = 0;
    int         m_activePaintKit = 0;
    float       m_activeWear = 0.01f;
    int         m_activeSeed = 1;
    char        m_activeWeaponName[64] = {};
    char        m_activeSkinName[64] = {};
    ERarity     m_activeRarity = RARITY_GOLD;

    std::vector<KnifeInfo> m_knives;
    std::unordered_map<int, std::vector<SkinInfo>> m_skinsByDefIndex;
    struct Category { const char* name; int defIndex; };
    std::vector<Category> m_categories;

    void        BuildKnifeList();
    void        BuildSkinLists();
    void        BuildCategories();
    void        RenderKnifePicker();
    void        RenderSkinPicker();
    void        SetActiveSkin(int defIndex, int paintKit, float wear, int seed,
        const char* weaponName, const char* skinName, ERarity rarity);
    void        ClearActiveSkin();
};

extern CSkinChanger g_SkinChanger;