#include "SkinChanger.hpp"
#include "../Menu/Menu.hpp"
#include "../Menu/Icons.h"
#include "../../Dependencies/ImGui/imgui.h"
#include "../../Dependencies/ImGui/imgui_internal.h"
#include "../../SDK/Offsets.hpp"
#include "../../SDK/EntityList.hpp"
#include "../../Core/PatternScanner.hpp"
#include <cstdio>
#include <windows.h>
#include <map>
#include <string>
#include <vector>
#include <cctype>
#include <cstring>

static const std::map<uint16_t, const char*> g_KnifeViewModels = {
    {500,"models/weapons/v_knife_bayonet.vmdl"},
    {502,"models/weapons/v_knife_karambit.vmdl"},
    {505,"models/weapons/v_knife_m9_bayonet.vmdl"},
    {507,"models/weapons/v_knife_tactical.vmdl"},
    {508,"models/weapons/v_knife_falchion.vmdl"},
    {509,"models/weapons/v_knife_push.vmdl"},
    {512,"models/weapons/v_knife_survival_bowie.vmdl"},
    {514,"models/weapons/v_knife_gypsy_jackknife.vmdl"},
    {515,"models/weapons/v_knife_butterfly.vmdl"},
    {516,"models/weapons/v_knife_stiletto.vmdl"},
    {517,"models/weapons/v_knife_widowmaker.vmdl"},
    {518,"models/weapons/v_knife_skeleton.vmdl"},
    {519,"models/weapons/v_knife_outdoor.vmdl"},
    {520,"models/weapons/v_knife_cord.vmdl"},
    {521,"models/weapons/v_knife_canis.vmdl"},
    {522,"models/weapons/v_knife_ursus.vmdl"},
    {523,"models/weapons/v_knife_css.vmdl"},
    {525,"models/weapons/v_knife_kukri.vmdl"},
};

static const std::map<uint16_t, const char*> g_KnifeWorldModels = {
    {500,"models/weapons/w_knife_bayonet.vmdl"},
    {502,"models/weapons/w_knife_karambit.vmdl"},
    {505,"models/weapons/w_knife_m9_bayonet.vmdl"},
    {507,"models/weapons/w_knife_tactical.vmdl"},
    {508,"models/weapons/w_knife_falchion.vmdl"},
    {509,"models/weapons/w_knife_push.vmdl"},
    {512,"models/weapons/w_knife_survival_bowie.vmdl"},
    {514,"models/weapons/w_knife_gypsy_jackknife.vmdl"},
    {515,"models/weapons/w_knife_butterfly.vmdl"},
    {516,"models/weapons/w_knife_stiletto.vmdl"},
    {517,"models/weapons/w_knife_widowmaker.vmdl"},
    {518,"models/weapons/w_knife_skeleton.vmdl"},
    {519,"models/weapons/w_knife_outdoor.vmdl"},
    {520,"models/weapons/w_knife_cord.vmdl"},
    {521,"models/weapons/w_knife_canis.vmdl"},
    {522,"models/weapons/w_knife_ursus.vmdl"},
    {523,"models/weapons/w_knife_css.vmdl"},
    {525,"models/weapons/w_knife_kukri.vmdl"},
};

static const std::map<uint16_t, uint64_t> g_SubclassIdMap = {
    {500,3933374535ULL},{502,3787235507ULL},{505,4046390180ULL},{506,2047704618ULL},
    {507,1731408398ULL},{508,1638561588ULL},{509,2282479884ULL},{512,3412259219ULL},
    {514,2511498851ULL},{515,1353709123ULL},{516,4269888884ULL},{517,1105782941ULL},
    {518,275962944ULL}, {519,1338637359ULL},{520,3230445913ULL},{521,3206681373ULL},
    {522,2595277776ULL},{523,4029975521ULL},{524,2463111489ULL},{525,365028728ULL},
    {526,3845286452ULL},
};

CSkinChanger g_SkinChanger;

static bool SRead(uintptr_t addr, void* out, size_t size) {
    __try { if (addr < 0x10000 || addr > 0x7FFFFFFEFFFF) return false; memcpy(out, (void*)addr, size); return true; }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}
static bool SWrite(uintptr_t addr, const void* data, size_t size) {
    __try { if (addr < 0x10000 || addr > 0x7FFFFFFEFFFF) return false; memcpy((void*)addr, data, size); return true; }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}
template<typename T> static bool SR(uintptr_t addr, T& out) { return SRead(addr, &out, sizeof(T)); }
template<typename T> static bool SW(uintptr_t addr, const T& val) { return SWrite(addr, &val, sizeof(T)); }

static bool IsKnifeDefIndex(uint16_t d) { return d == 42 || d == 59 || (d >= 500 && d <= 526); }
static bool IsHeap(uintptr_t a) { return a > 0x10000 && a < 0x7FFFFFFFFFFF00ULL; }

static bool IsInGame() {
    uintptr_t engineBase = (uintptr_t)GetModuleHandleA("engine2.dll");
    if (!engineBase) return false;
    if (!Offsets::dwNetworkGameClient || !Offsets::dwNetworkGameClient_deltaTick) return false;
    uintptr_t networkClient = 0;
    SR(engineBase + Offsets::dwNetworkGameClient, networkClient);
    if (!networkClient || !IsHeap(networkClient)) return false;
    int deltaTick = -1;
    SR(networkClient + Offsets::dwNetworkGameClient_deltaTick, deltaTick);
    return deltaTick != -1;
}

static uintptr_t GetEntityFromHandle(uintptr_t entityList, uint32_t handle) {
    if (!handle || handle == 0xFFFFFFFF) return 0;
    uint32_t idx = handle & 0x7FFF;
    uintptr_t listEntry = 0;
    if (!SR(entityList + 0x10 + 0x8LL * (idx >> 9), listEntry) || !listEntry) return 0;
    uintptr_t ent = 0;
    if (!SR(listEntry + 0x70LL * (idx & 0x1FF), ent) || !ent || ent < 0x10000) return 0;
    return ent;
}

typedef void(__fastcall* tSetModel)(uintptr_t, const char*);
static bool CallSetModel(uintptr_t entity, const char* model) {
    if (!entity || !model || !model[0] || !IsHeap(entity)) return false;
    if (!Functions::fnSetModel || !IsHeap(Functions::fnSetModel)) return false;
    __try { ((tSetModel)Functions::fnSetModel)(entity, model); return true; }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

static void InvalidateSceneNode(uintptr_t entity) {
    uintptr_t sceneNode = 0;
    uintptr_t cbody = Schemas::C_BaseModelEntity::m_CBodyComponent;
    if (cbody > 0x100) SR(entity + cbody, sceneNode);
    if (!IsHeap(sceneNode)) SR(entity + 0x310, sceneNode);
    if (IsHeap(sceneNode)) {
        uint64_t m = 1;
        SW(sceneNode + 0x1B0, m);
        SW(sceneNode + 0x160, m);
    }
}

static void UpdateSubclassID(uintptr_t weapon, uint16_t newDefIndex) {
    auto it = g_SubclassIdMap.find(newDefIndex);
    if (it == g_SubclassIdMap.end()) return;
    uintptr_t sceneNode = 0;
    uintptr_t cbody = Schemas::C_BaseModelEntity::m_CBodyComponent;
    if (cbody > 0x100) SR(weapon + cbody, sceneNode);
    if (!IsHeap(sceneNode)) SR(weapon + 0x310, sceneNode);
    if (!IsHeap(sceneNode)) return;
    SW(sceneNode + 0x78, it->second);
}

static uintptr_t GetViewModelEntity(uintptr_t localPawn, uintptr_t entityList) {
    uintptr_t vmServices = 0;
    uintptr_t vmServicesOff = Schemas::C_CSPlayerPawnBase::m_pViewModelServices;
    if (vmServicesOff > 0x100) SR(localPawn + vmServicesOff, vmServices);
    if (IsHeap(vmServices)) {
        uint32_t vmHandle = 0;
        uintptr_t vmHandleOff = Schemas::CPlayer_ViewModelServices::m_hViewModel;
        if (vmHandleOff > 0x10) {
            SR(vmServices + vmHandleOff, vmHandle);
            if (vmHandle && vmHandle != 0xFFFFFFFF) {
                uintptr_t vmEnt = GetEntityFromHandle(entityList, vmHandle);
                if (vmEnt) return vmEnt;
            }
        }
    }
    static const uintptr_t fallbackOffs[] = { 0xCB8, 0xCC0, 0xCC8, 0x11B8, 0x11C0, 0x11C8 };
    for (auto off : fallbackOffs) {
        uint32_t h = 0;
        SR(localPawn + off, h);
        if (!h || h == 0xFFFFFFFF) continue;
        uintptr_t vmEnt = GetEntityFromHandle(entityList, h);
        if (vmEnt) return vmEnt;
    }
    return 0;
}

namespace {
    struct EconItemAttribute {
        char pad_0000[0x30];
        uint16_t def_index;
        char pad_0032[2];
        float value;
        float init_value;
        int32_t refundable_currency;
        bool set_bonus;
        char pad_0041[7];
    };
    struct PtrGameVector {
        uint64_t size;
        uintptr_t ptr;
    };
    enum : uint16_t { ATTR_PAINT = 6, ATTR_PATTERN = 7, ATTR_WEAR = 8 };

    void* GameAlloc(size_t size) { return malloc(size); }
    void GameFree(void* ptr) { free(ptr); }

    void CreateAttributes(uintptr_t item, int paintKit, float wear, int seed) {
        if (!item || paintKit <= 0) return;
        PtrGameVector* vec = (PtrGameVector*)(item + 0x38);
        if (vec->size != 0 || vec->ptr != 0) return;
        constexpr size_t attrCount = 3;
        EconItemAttribute* attrs = (EconItemAttribute*)GameAlloc(attrCount * sizeof(EconItemAttribute));
        if (!attrs) return;
        memset(attrs, 0, attrCount * sizeof(EconItemAttribute));
        attrs[0].def_index = ATTR_PAINT;
        attrs[0].value = (float)paintKit;
        attrs[0].init_value = attrs[0].value;
        attrs[1].def_index = ATTR_PATTERN;
        attrs[1].value = (float)(seed >= 0 ? seed : 0);
        attrs[1].init_value = attrs[1].value;
        attrs[2].def_index = ATTR_WEAR;
        attrs[2].value = wear >= 0.0f ? wear : 0.01f;
        attrs[2].init_value = attrs[2].value;
        vec->size = attrCount;
        vec->ptr = (uintptr_t)attrs;
    }

    void RemoveAttributes(uintptr_t item) {
        if (!item) return;
        PtrGameVector* vec = (PtrGameVector*)(item + 0x38);
        if (vec->size == 0) return;
        GameFree((void*)vec->ptr);
        vec->size = 0;
        vec->ptr = 0;
    }

    void UpdateSkin(uintptr_t econEntity, bool force) {
        auto** vt = *(void***)econEntity;
        auto fn = (void(__fastcall*)(void*, bool))vt[110];
        fn((void*)econEntity, force);
    }

    void UpdateWeaponData(uintptr_t econEntity) {
        auto** vt = *(void***)econEntity;
        auto fn = (void* (__fastcall*)(void*))vt[195];
        fn((void*)econEntity);
    }
}

void CSkinChanger::BuildCategories() {
    m_categories.clear();
    m_categories.push_back({ "All", -1 });
    m_categories.push_back({ "Bayonet", 500 });
    m_categories.push_back({ "Karambit", 502 });
    m_categories.push_back({ "M9 Bayonet", 505 });
    m_categories.push_back({ "Butterfly", 515 });
    m_categories.push_back({ "Huntsman", 507 });
    m_categories.push_back({ "Falchion", 508 });
    m_categories.push_back({ "Shadow Daggers", 509 });
    m_categories.push_back({ "Bowie", 512 });
    m_categories.push_back({ "Navaja", 514 });
    m_categories.push_back({ "Stiletto", 516 });
    m_categories.push_back({ "Talon", 517 });
    m_categories.push_back({ "Skeleton", 518 });
    m_categories.push_back({ "Nomad", 519 });
    m_categories.push_back({ "Paracord", 520 });
    m_categories.push_back({ "Survival", 521 });
    m_categories.push_back({ "Ursus", 522 });
    m_categories.push_back({ "Classic", 523 });
    m_categories.push_back({ "Kukri", 525 });
}

void CSkinChanger::BuildKnifeList() {
    m_knives.clear();
    for (const auto& pair : g_KnifeViewModels) {
        uint16_t def = pair.first;
        const char* modelPath = pair.second;
        const char* name = "Unknown";
        switch (def) {
        case 500: name = "Bayonet"; break;
        case 502: name = "Karambit"; break;
        case 505: name = "M9 Bayonet"; break;
        case 507: name = "Huntsman Knife"; break;
        case 508: name = "Falchion Knife"; break;
        case 509: name = "Shadow Daggers"; break;
        case 512: name = "Bowie Knife"; break;
        case 514: name = "Navaja Knife"; break;
        case 515: name = "Butterfly Knife"; break;
        case 516: name = "Stiletto Knife"; break;
        case 517: name = "Talon Knife"; break;
        case 518: name = "Skeleton Knife"; break;
        case 519: name = "Nomad Knife"; break;
        case 520: name = "Paracord Knife"; break;
        case 521: name = "Survival Knife"; break;
        case 522: name = "Ursus Knife"; break;
        case 523: name = "Classic Knife"; break;
        case 525: name = "Kukri Knife"; break;
        default: continue;
        }
        m_knives.push_back({ (int)def, name, modelPath });
    }
}

void CSkinChanger::BuildSkinLists() {
    static SkinInfo rawSkins[] = {
        {500,  0,"Bayonet",        "Vanilla",       RARITY_GOLD},
        {500, 12,"Bayonet",        "Crimson Web",   RARITY_COVERT},
        {500, 38,"Bayonet",        "Fade",          RARITY_COVERT},
        {500, 44,"Bayonet",        "Case Hardened", RARITY_CLASSIFIED},
        {500, 59,"Bayonet",        "Slaughter",     RARITY_COVERT},
        {500,409,"Bayonet",        "Tiger Tooth",   RARITY_COVERT},
        {500,415,"Bayonet",        "Doppler",       RARITY_COVERT},
        {500,416,"Bayonet",        "Marble Fade",   RARITY_COVERT},
        {500,568,"Bayonet",        "Lore",          RARITY_COVERT},
        {500,617,"Bayonet",        "Gamma Doppler", RARITY_COVERT},
        {500,672,"Bayonet",        "Black Laminate",RARITY_COVERT},
        {500,735,"Bayonet",        "Autotronic",    RARITY_COVERT},
        {502,  0,"Karambit",       "Vanilla",       RARITY_GOLD},
        {502, 12,"Karambit",       "Crimson Web",   RARITY_COVERT},
        {502, 38,"Karambit",       "Fade",          RARITY_COVERT},
        {502, 44,"Karambit",       "Case Hardened", RARITY_CLASSIFIED},
        {502, 59,"Karambit",       "Slaughter",     RARITY_COVERT},
        {502,409,"Karambit",       "Tiger Tooth",   RARITY_COVERT},
        {502,415,"Karambit",       "Doppler",       RARITY_COVERT},
        {502,416,"Karambit",       "Marble Fade",   RARITY_COVERT},
        {502,568,"Karambit",       "Lore",          RARITY_COVERT},
        {502,617,"Karambit",       "Gamma Doppler", RARITY_COVERT},
        {502,672,"Karambit",       "Black Laminate",RARITY_COVERT},
        {502,735,"Karambit",       "Autotronic",    RARITY_COVERT},
        {505,  0,"M9 Bayonet",     "Vanilla",       RARITY_GOLD},
        {505, 12,"M9 Bayonet",     "Crimson Web",   RARITY_COVERT},
        {505, 38,"M9 Bayonet",     "Fade",          RARITY_COVERT},
        {505, 44,"M9 Bayonet",     "Case Hardened", RARITY_CLASSIFIED},
        {505, 59,"M9 Bayonet",     "Slaughter",     RARITY_COVERT},
        {505,409,"M9 Bayonet",     "Tiger Tooth",   RARITY_COVERT},
        {505,415,"M9 Bayonet",     "Doppler",       RARITY_COVERT},
        {505,416,"M9 Bayonet",     "Marble Fade",   RARITY_COVERT},
        {505,568,"M9 Bayonet",     "Lore",          RARITY_COVERT},
        {505,617,"M9 Bayonet",     "Gamma Doppler", RARITY_COVERT},
        {505,672,"M9 Bayonet",     "Black Laminate",RARITY_COVERT},
        {505,735,"M9 Bayonet",     "Autotronic",    RARITY_COVERT},
        {515,  0,"Butterfly Knife","Vanilla",       RARITY_GOLD},
        {515, 12,"Butterfly Knife","Crimson Web",   RARITY_COVERT},
        {515, 38,"Butterfly Knife","Fade",          RARITY_COVERT},
        {515, 44,"Butterfly Knife","Case Hardened", RARITY_CLASSIFIED},
        {515, 59,"Butterfly Knife","Slaughter",     RARITY_COVERT},
        {515,409,"Butterfly Knife","Tiger Tooth",   RARITY_COVERT},
        {515,415,"Butterfly Knife","Doppler",       RARITY_COVERT},
        {515,416,"Butterfly Knife","Marble Fade",   RARITY_COVERT},
        {515,568,"Butterfly Knife","Lore",          RARITY_COVERT},
        {515,617,"Butterfly Knife","Gamma Doppler", RARITY_COVERT},
        {515,672,"Butterfly Knife","Black Laminate",RARITY_COVERT},
        {507,  0,"Huntsman Knife", "Vanilla",       RARITY_GOLD},
        {507, 12,"Huntsman Knife", "Crimson Web",   RARITY_COVERT},
        {507, 38,"Huntsman Knife", "Fade",          RARITY_COVERT},
        {507, 44,"Huntsman Knife", "Case Hardened", RARITY_CLASSIFIED},
        {507, 59,"Huntsman Knife", "Slaughter",     RARITY_COVERT},
        {507,409,"Huntsman Knife", "Tiger Tooth",   RARITY_COVERT},
        {507,415,"Huntsman Knife", "Doppler",       RARITY_COVERT},
        {507,416,"Huntsman Knife", "Marble Fade",   RARITY_COVERT},
        {507,568,"Huntsman Knife", "Lore",          RARITY_COVERT},
        {507,617,"Huntsman Knife", "Gamma Doppler", RARITY_COVERT},
        {508,  0,"Falchion Knife", "Vanilla",       RARITY_GOLD},
        {508, 12,"Falchion Knife", "Crimson Web",   RARITY_COVERT},
        {508, 38,"Falchion Knife", "Fade",          RARITY_COVERT},
        {508, 44,"Falchion Knife", "Case Hardened", RARITY_CLASSIFIED},
        {508, 59,"Falchion Knife", "Slaughter",     RARITY_COVERT},
        {508,415,"Falchion Knife", "Doppler",       RARITY_COVERT},
        {508,416,"Falchion Knife", "Marble Fade",   RARITY_COVERT},
        {509,  0,"Shadow Daggers", "Vanilla",       RARITY_GOLD},
        {509, 12,"Shadow Daggers", "Crimson Web",   RARITY_COVERT},
        {509, 38,"Shadow Daggers", "Fade",          RARITY_COVERT},
        {509, 44,"Shadow Daggers", "Case Hardened", RARITY_CLASSIFIED},
        {509, 59,"Shadow Daggers", "Slaughter",     RARITY_COVERT},
        {509,415,"Shadow Daggers", "Doppler",       RARITY_COVERT},
        {509,416,"Shadow Daggers", "Marble Fade",   RARITY_COVERT},
        {512,  0,"Bowie Knife",    "Vanilla",       RARITY_GOLD},
        {512, 12,"Bowie Knife",    "Crimson Web",   RARITY_COVERT},
        {512, 38,"Bowie Knife",    "Fade",          RARITY_COVERT},
        {512, 44,"Bowie Knife",    "Case Hardened", RARITY_CLASSIFIED},
        {512, 59,"Bowie Knife",    "Slaughter",     RARITY_COVERT},
        {512,415,"Bowie Knife",    "Doppler",       RARITY_COVERT},
        {512,416,"Bowie Knife",    "Marble Fade",   RARITY_COVERT},
        {514,  0,"Navaja Knife",   "Vanilla",       RARITY_GOLD},
        {514, 12,"Navaja Knife",   "Crimson Web",   RARITY_COVERT},
        {514, 38,"Navaja Knife",   "Fade",          RARITY_COVERT},
        {514, 44,"Navaja Knife",   "Case Hardened", RARITY_CLASSIFIED},
        {514,415,"Navaja Knife",   "Doppler",       RARITY_COVERT},
        {514,416,"Navaja Knife",   "Marble Fade",   RARITY_COVERT},
        {516,  0,"Stiletto Knife", "Vanilla",       RARITY_GOLD},
        {516, 12,"Stiletto Knife", "Crimson Web",   RARITY_COVERT},
        {516, 38,"Stiletto Knife", "Fade",          RARITY_COVERT},
        {516, 44,"Stiletto Knife", "Case Hardened", RARITY_CLASSIFIED},
        {516,415,"Stiletto Knife", "Doppler",       RARITY_COVERT},
        {516,416,"Stiletto Knife", "Marble Fade",   RARITY_COVERT},
        {517,  0,"Talon Knife",    "Vanilla",       RARITY_GOLD},
        {517, 12,"Talon Knife",    "Crimson Web",   RARITY_COVERT},
        {517, 38,"Talon Knife",    "Fade",          RARITY_COVERT},
        {517, 44,"Talon Knife",    "Case Hardened", RARITY_CLASSIFIED},
        {517,415,"Talon Knife",    "Doppler",       RARITY_COVERT},
        {517,416,"Talon Knife",    "Marble Fade",   RARITY_COVERT},
        {518,  0,"Skeleton Knife", "Vanilla",       RARITY_GOLD},
        {518, 12,"Skeleton Knife", "Crimson Web",   RARITY_COVERT},
        {518, 38,"Skeleton Knife", "Fade",          RARITY_COVERT},
        {518, 44,"Skeleton Knife", "Case Hardened", RARITY_CLASSIFIED},
        {518,415,"Skeleton Knife", "Doppler",       RARITY_COVERT},
        {518,416,"Skeleton Knife", "Marble Fade",   RARITY_COVERT},
        {519,  0,"Nomad Knife",    "Vanilla",       RARITY_GOLD},
        {519, 12,"Nomad Knife",    "Crimson Web",   RARITY_COVERT},
        {519, 38,"Nomad Knife",    "Fade",          RARITY_COVERT},
        {519, 44,"Nomad Knife",    "Case Hardened", RARITY_CLASSIFIED},
        {519,415,"Nomad Knife",    "Doppler",       RARITY_COVERT},
        {519,416,"Nomad Knife",    "Marble Fade",   RARITY_COVERT},
        {520,  0,"Paracord Knife", "Vanilla",       RARITY_GOLD},
        {520, 12,"Paracord Knife", "Crimson Web",   RARITY_COVERT},
        {520, 38,"Paracord Knife", "Fade",          RARITY_COVERT},
        {520, 44,"Paracord Knife", "Case Hardened", RARITY_CLASSIFIED},
        {521,  0,"Survival Knife", "Vanilla",       RARITY_GOLD},
        {521, 12,"Survival Knife", "Crimson Web",   RARITY_COVERT},
        {521, 38,"Survival Knife", "Fade",          RARITY_COVERT},
        {521, 44,"Survival Knife", "Case Hardened", RARITY_CLASSIFIED},
        {522,  0,"Ursus Knife",    "Vanilla",       RARITY_GOLD},
        {522, 12,"Ursus Knife",    "Crimson Web",   RARITY_COVERT},
        {522, 38,"Ursus Knife",    "Fade",          RARITY_COVERT},
        {522, 44,"Ursus Knife",    "Case Hardened", RARITY_CLASSIFIED},
        {522,415,"Ursus Knife",    "Doppler",       RARITY_COVERT},
        {522,416,"Ursus Knife",    "Marble Fade",   RARITY_COVERT},
        {522,617,"Ursus Knife",    "Gamma Doppler", RARITY_COVERT},
        {523,  0,"Classic Knife",  "Vanilla",       RARITY_GOLD},
        {523, 12,"Classic Knife",  "Crimson Web",   RARITY_COVERT},
        {523, 38,"Classic Knife",  "Fade",          RARITY_COVERT},
        {523, 44,"Classic Knife",  "Case Hardened", RARITY_CLASSIFIED},
        {523,415,"Classic Knife",  "Doppler",       RARITY_COVERT},
        {523,416,"Classic Knife",  "Marble Fade",   RARITY_COVERT},
        {525,  0,"Kukri Knife",    "Vanilla",       RARITY_GOLD},
        {525, 12,"Kukri Knife",    "Crimson Web",   RARITY_COVERT},
        {525, 38,"Kukri Knife",    "Fade",          RARITY_COVERT},
        {525, 44,"Kukri Knife",    "Case Hardened", RARITY_CLASSIFIED},
        {525,415,"Kukri Knife",    "Doppler",       RARITY_COVERT},
        {525,416,"Kukri Knife",    "Marble Fade",   RARITY_COVERT},
    };
    const int count = sizeof(rawSkins) / sizeof(rawSkins[0]);
    m_skinsByDefIndex.clear();
    for (int i = 0; i < count; ++i) {
        m_skinsByDefIndex[rawSkins[i].defIndex].push_back(rawSkins[i]);
    }
}

void CSkinChanger::SetActiveSkin(int defIndex, int paintKit, float wear, int seed,
    const char* weaponName, const char* skinName, ERarity rarity) {
    m_activeDefIndex = defIndex;
    m_activePaintKit = paintKit;
    m_activeWear = wear;
    m_activeSeed = seed;
    strcpy_s(m_activeWeaponName, weaponName ? weaponName : "");
    strcpy_s(m_activeSkinName, skinName ? skinName : "");
    m_activeRarity = rarity;
    m_hasActiveSkin = true;
}

void CSkinChanger::ClearActiveSkin() {
    m_hasActiveSkin = false;
}

void CSkinChanger::ApplySkins() {
    static int callCount = 0;
    callCount++;
    const bool log = (callCount <= 5);

    if (!IsInGame()) { if (log) OutputDebugStringA("[Skin] Not in game\n"); return; }
    if (!m_hasActiveSkin) { if (log) OutputDebugStringA("[Skin] No active skin\n"); return; }

    uintptr_t clientBase = (uintptr_t)GetModuleHandleA("client.dll");
    if (!clientBase) { if (log) OutputDebugStringA("[Skin] clientBase null\n"); return; }

    uintptr_t localPawn = 0;
    SR(clientBase + Offsets::dwLocalPlayerPawn, localPawn);
    if (!IsHeap(localPawn)) { if (log) { char buf[64]; sprintf_s(buf, "[Skin] localPawn invalid 0x%p\n", (void*)localPawn); OutputDebugStringA(buf); } return; }

    int health = 0;
    SR(localPawn + Schemas::C_BaseEntity::m_iHealth, health);
    if (health <= 0) { if (log) { char buf[32]; sprintf_s(buf, "[Skin] health=%d\n", health); OutputDebugStringA(buf); } return; }

    uintptr_t weaponServices = 0;
    if (!SR(localPawn + Schemas::C_BasePlayerPawn::m_pWeaponServices, weaponServices) || !IsHeap(weaponServices)) {
        if (log) { char buf[64]; sprintf_s(buf, "[Skin] weaponServices invalid 0x%p\n", (void*)weaponServices); OutputDebugStringA(buf); }
        return;
    }

    uint32_t weaponCount = 0;
    uintptr_t myWeaponsOff = Schemas::CPlayer_WeaponServices::m_hMyWeapons;
    SR(weaponServices + myWeaponsOff, weaponCount);
    if (weaponCount == 0 || weaponCount > 256) { if (log) { char buf[32]; sprintf_s(buf, "[Skin] weaponCount=%d\n", weaponCount); OutputDebugStringA(buf); } return; }

    uintptr_t weaponArray = 0;
    SR(weaponServices + myWeaponsOff + 0x8, weaponArray);
    if (!IsHeap(weaponArray)) { if (log) OutputDebugStringA("[Skin] weaponArray invalid\n"); return; }

    uintptr_t entityList = 0;
    SR(clientBase + Offsets::dwEntityList, entityList);
    if (!IsHeap(entityList)) { if (log) OutputDebugStringA("[Skin] entityList invalid\n"); return; }

    const uintptr_t kAttrMgrOffset = 0x1180;

    for (uint32_t i = 0; i < weaponCount; i++) {
        uint32_t handle = 0;
        SR(weaponArray + i * sizeof(uint32_t), handle);
        if (!handle || handle == 0xFFFFFFFF) continue;

        uintptr_t weapon = GetEntityFromHandle(entityList, handle);
        if (!weapon) continue;

        uintptr_t attrMgr = 0;
        SR(weapon + kAttrMgrOffset, attrMgr);
        if (!IsHeap(attrMgr)) continue;

        uintptr_t item = attrMgr + 0x50;
        uint16_t defIdx = 0;
        SR(item + 0x1BA, defIdx);

        if (log) {
            char buf[128];
            sprintf_s(buf, "[Skin] weapon[%d] defIdx=%d (knife? %d)\n", i, defIdx, IsKnifeDefIndex(defIdx) ? 1 : 0);
            OutputDebugStringA(buf);
        }

        if (!IsKnifeDefIndex(defIdx)) continue;

        SW(item + 0x1BA, (uint16_t)m_activeDefIndex);
        SW(item + 0x1BC, (int32_t)3);
        SW(item + 0x1D0, (uint32_t)0xFFFFFFFF);
        SW(item + 0x1D4, (uint32_t)0xFFFFFFFF);
        SW(item + 0x1D8, (uint32_t)0);

        SW(weapon + 0x1658, (uint32_t)m_activePaintKit);
        SW(weapon + 0x1660, m_activeWear);
        SW(weapon + 0x165C, (uint32_t)m_activeSeed);
        SW(weapon + 0x1664, (uint32_t)0xFFFFFFFF);
        SW(weapon + 0x1650, (uint32_t)0);
        SW(weapon + 0x1654, (uint32_t)0);

        UpdateSubclassID(weapon, (uint16_t)m_activeDefIndex);
        InvalidateSceneNode(weapon);

        RemoveAttributes(item);
        if (m_activePaintKit > 0)
            CreateAttributes(item, m_activePaintKit, m_activeWear, m_activeSeed);

        auto vmIt = g_KnifeViewModels.find(m_activeDefIndex);
        auto wmIt = g_KnifeWorldModels.find(m_activeDefIndex);
        if (vmIt != g_KnifeViewModels.end() && wmIt != g_KnifeWorldModels.end()) {
            uint32_t worldModelHandle = 0;
            uintptr_t worldModelOffset = Schemas::C_CSWeaponBase::m_hWeaponWorldModel;
            if (worldModelOffset > 0) {
                SR(weapon + worldModelOffset, worldModelHandle);
                if (worldModelHandle && worldModelHandle != 0xFFFFFFFF) {
                    uintptr_t worldModelEnt = GetEntityFromHandle(entityList, worldModelHandle);
                    if (worldModelEnt) {
                        CallSetModel(worldModelEnt, wmIt->second);
                        InvalidateSceneNode(worldModelEnt);
                    }
                }
            }
            uintptr_t vmEnt = GetViewModelEntity(localPawn, entityList);
            if (vmEnt) {
                CallSetModel(vmEnt, vmIt->second);
                InvalidateSceneNode(vmEnt);
            }
        }

        UpdateSkin(weapon, true);
        UpdateWeaponData(weapon);
        SW(item + 0x200, (uintptr_t)0);

        if (log) OutputDebugStringA("[Skin] Applied successfully!\n");
        return;
    }
    if (log) OutputDebugStringA("[Skin] No knife found in weapons\n");
}

static ImU32 GetRarityColor(ERarity r) {
    switch (r) {
    case RARITY_CONSUMER:   return IM_COL32(176, 195, 217, 255);
    case RARITY_INDUSTRIAL: return IM_COL32(94, 152, 217, 255);
    case RARITY_MIL_SPEC:   return IM_COL32(75, 105, 255, 255);
    case RARITY_RESTRICTED: return IM_COL32(136, 71, 255, 255);
    case RARITY_CLASSIFIED: return IM_COL32(211, 44, 230, 255);
    case RARITY_COVERT:     return IM_COL32(235, 75, 75, 255);
    case RARITY_GOLD:       return IM_COL32(228, 174, 57, 255);
    }
    return IM_COL32(255, 255, 255, 255);
}
static const char* GetRarityLabel(ERarity r) {
    switch (r) {
    case RARITY_CONSUMER:   return "Consumer";
    case RARITY_INDUSTRIAL: return "Industrial";
    case RARITY_MIL_SPEC:   return "Mil-Spec";
    case RARITY_RESTRICTED: return "Restricted";
    case RARITY_CLASSIFIED: return "Classified";
    case RARITY_COVERT:     return "Covert";
    case RARITY_GOLD:       return "Contraband";
    }
    return "";
}
static bool StrContainsCI(const char* hay, const char* needle) {
    if (!needle || !needle[0]) return true;
    if (!hay) return false;
    size_t nL = strlen(needle), hL = strlen(hay);
    if (nL > hL) return false;
    for (size_t i = 0; i <= hL - nL; i++) {
        bool m = true;
        for (size_t j = 0; j < nL; j++)
            if (tolower((unsigned char)hay[i + j]) != tolower((unsigned char)needle[j])) { m = false; break; }
        if (m) return true;
    }
    return false;
}

void CSkinChanger::Render() {
    if (m_knives.empty()) {
        BuildCategories();
        BuildKnifeList();
        BuildSkinLists();
    }

    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 contentPos = ImGui::GetCursorScreenPos();
    const float cardW = 185.0f, cardH = 75.0f, padding = 8.0f;
    const float startX = contentPos.x + 15.0f, startY = contentPos.y + 15.0f;

    if (!m_hasActiveSkin) {
        ImVec2 addMin(startX, startY), addMax(startX + cardW, startY + cardH);
        draw->AddRectFilled(addMin, addMax, IM_COL32(38, 38, 38, 255), 4.0f);
        draw->AddRectFilled(ImVec2(startX, startY + cardH - 3.f), ImVec2(startX + cardW, startY + cardH), IM_COL32(32, 32, 32, 255));
        float addCx = startX + cardW * 0.5f, addCy = startY + (cardH - 3.f) * 0.5f;
        draw->AddCircleFilled(ImVec2(addCx, addCy), 18.f, IM_COL32(50, 50, 50, 255));
        draw->AddCircle(ImVec2(addCx, addCy), 18.f, IM_COL32(65, 65, 65, 255), 0, 1.f);
        ImGui::PushFont(g_Menu.LogoFont);
        ImVec2 ps = ImGui::CalcTextSize("+");
        draw->AddText(ImVec2(addCx - ps.x * .5f, addCy - ps.y * .5f), IM_COL32(255, 255, 255, 255), "+");
        ImGui::PopFont();
        ImGui::SetCursorScreenPos(addMin);
        if (ImGui::InvisibleButton("##add_knife", ImVec2(cardW, cardH))) {
            m_showKnifePicker = true;
            m_knifePickerCategory = 0;
            m_knifeSearchBuf[0] = '\0';
        }
        if (ImGui::IsItemHovered())
            draw->AddRect(addMin, addMax, IM_COL32(255, 50, 50, 160), 4.f, 0, 1.5f);
    }
    else {
        ImVec2 cMin(startX, startY), cMax(startX + cardW, startY + cardH);
        ImU32 rc = GetRarityColor(m_activeRarity);
        draw->AddRectFilled(cMin, cMax, IM_COL32(42, 42, 42, 255), 4.f);
        draw->AddRectFilled(ImVec2(startX, startY + cardH - 3.f), ImVec2(startX + cardW, startY + cardH), rc);
        float iconCx = startX + 28.f, iconCy = startY + (cardH - 3.f) * 0.5f;
        draw->AddCircleFilled(ImVec2(iconCx, iconCy), 16.f, IM_COL32(55, 55, 55, 255));
        char initBuf[2] = { m_activeWeaponName[0], 0 };
        ImVec2 is2 = ImGui::CalcTextSize(initBuf);
        draw->AddText(ImVec2(iconCx - is2.x * .5f, iconCy - is2.y * .5f), IM_COL32(200, 200, 200, 255), initBuf);
        draw->AddText(ImVec2(startX + 52.f, startY + 10.f), IM_COL32(220, 220, 220, 255), m_activeWeaponName);
        draw->AddText(ImVec2(startX + 52.f, startY + 27.f), rc, m_activeSkinName);
        draw->AddText(ImVec2(startX + 52.f, startY + 44.f), IM_COL32(110, 110, 110, 255), GetRarityLabel(m_activeRarity));

        ImGui::SetCursorScreenPos(cMin);
        if (ImGui::InvisibleButton("##active_knife", ImVec2(cardW, cardH))) {
            ClearActiveSkin();
        }
        if (ImGui::IsItemHovered()) {
            draw->AddRect(cMin, cMax, IM_COL32(255, 50, 50, 200), 4.f, 0, 1.5f);
            ImVec2 mp = ImGui::GetMousePos();
            draw->AddRectFilled(ImVec2(mp.x + 12, mp.y), ImVec2(mp.x + 116, mp.y + 22), IM_COL32(18, 18, 18, 240), 3.f);
            draw->AddText(ImVec2(mp.x + 16, mp.y + 4), IM_COL32(255, 80, 80, 255), "Click to remove");
        }

        const float btnX = startX + cardW + padding;
        ImVec2 btnMin(btnX, startY), btnMax(btnX + cardW, startY + cardH);
        draw->AddRectFilled(btnMin, btnMax, IM_COL32(38, 38, 38, 255), 4.f);
        draw->AddRectFilled(ImVec2(btnX, startY + cardH - 3.f), ImVec2(btnX + cardW, startY + cardH), IM_COL32(32, 32, 32, 255));
        const char* changeLbl = "Change skin";
        ImVec2 lsz = ImGui::CalcTextSize(changeLbl);
        draw->AddText(ImVec2(btnX + (cardW - lsz.x) * .5f, startY + (cardH - lsz.y) * .5f - 1.5f),
            IM_COL32(180, 180, 180, 255), changeLbl);
        ImGui::SetCursorScreenPos(btnMin);
        if (ImGui::InvisibleButton("##change_skin", ImVec2(cardW, cardH))) {
            m_showKnifePicker = true;
        }
        if (ImGui::IsItemHovered())
            draw->AddRect(btnMin, btnMax, IM_COL32(255, 50, 50, 160), 4.f, 0, 1.5f);
    }

    if (m_showKnifePicker) RenderKnifePicker();
    if (m_showSkinPicker) RenderSkinPicker();
}

void CSkinChanger::RenderKnifePicker() {
    ImVec2 ds = ImGui::GetIO().DisplaySize;
    const float W = 560.f, H = 460.f, CW = 138.f;
    ImGui::SetNextWindowPos(ImVec2((ds.x - W) * .5f, (ds.y - H) * .5f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(W, H));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(.085f, .085f, .085f, .98f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(.16f, .16f, .16f, 1.f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    bool open = true;
    if (ImGui::Begin("##KnifePicker", &open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove)) {
        ImDrawList* d = ImGui::GetWindowDrawList();
        ImVec2 wp = ImGui::GetWindowPos();
        d->AddRectFilled(wp, ImVec2(wp.x + W, wp.y + 38.f), IM_COL32(18, 18, 18, 255), 6.f);
        d->AddRectFilled(ImVec2(wp.x, wp.y + 32.f), ImVec2(wp.x + W, wp.y + 38.f), IM_COL32(18, 18, 18, 255));
        d->AddRectFilled(wp, ImVec2(wp.x + 3.f, wp.y + 38.f), IM_COL32(255, 50, 50, 255));
        d->AddText(ImVec2(wp.x + 14.f, wp.y + 11.f), IM_COL32(230, 230, 230, 255), "Select Knife");
        ImGui::SetCursorScreenPos(ImVec2(wp.x + W - 30.f, wp.y + 9.f));
        if (ImGui::InvisibleButton("##close_knife", ImVec2(22.f, 20.f))) m_showKnifePicker = false;
        d->AddText(ImVec2(wp.x + W - 25.f, wp.y + 11.f),
            ImGui::IsItemHovered() ? IM_COL32(255, 100, 100, 255) : IM_COL32(120, 120, 120, 255), "x");
        d->AddLine(ImVec2(wp.x, wp.y + 38.f), ImVec2(wp.x + W, wp.y + 38.f), IM_COL32(25, 25, 25, 255));

        ImGui::SetCursorScreenPos(ImVec2(wp.x, wp.y + 38.f));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(.065f, .065f, .065f, 1.f));
        ImGui::BeginChild("##knife_cat", ImVec2(CW, H - 38.f), false, ImGuiWindowFlags_NoScrollbar);
        for (int c = 0; c < (int)m_categories.size(); c++) {
            bool sel = (m_knifePickerCategory == c);
            ImVec2 cp = ImGui::GetCursorScreenPos();
            float ch = 30.f;
            if (sel) {
                d->AddRectFilled(cp, ImVec2(cp.x + CW, cp.y + ch), IM_COL32(28, 28, 28, 255));
                d->AddRectFilled(cp, ImVec2(cp.x + 2.5f, cp.y + ch), IM_COL32(255, 50, 50, 255));
            }
            d->AddText(ImVec2(cp.x + 13.f, cp.y + (ch - ImGui::GetFontSize()) * .5f),
                sel ? IM_COL32(255, 255, 255, 255) : IM_COL32(155, 155, 155, 255), m_categories[c].name);
            ImGui::SetCursorScreenPos(cp);
            char catId[24]; sprintf_s(catId, "##knifecat_%d", c);
            if (ImGui::InvisibleButton(catId, ImVec2(CW, ch))) {
                m_knifePickerCategory = c;
                m_knifeSearchBuf[0] = '\0';
            }
            if (ImGui::IsItemHovered() && !sel)
                d->AddRectFilled(cp, ImVec2(cp.x + CW, cp.y + ch), IM_COL32(255, 255, 255, 6));
            ImGui::SetCursorScreenPos(ImVec2(cp.x, cp.y + ch));
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();
        d->AddLine(ImVec2(wp.x + CW, wp.y + 38.f), ImVec2(wp.x + CW, wp.y + H), IM_COL32(22, 22, 22, 255));

        const float rX = wp.x + CW, rW = W - CW, searchH = 34.f;
        d->AddRectFilled(ImVec2(rX, wp.y + 38.f), ImVec2(wp.x + W, wp.y + 38.f + searchH), IM_COL32(15, 15, 15, 255));
        d->AddLine(ImVec2(rX, wp.y + 38.f + searchH), ImVec2(wp.x + W, wp.y + 38.f + searchH), IM_COL32(22, 22, 22, 255));
        ImGui::SetCursorScreenPos(ImVec2(rX + 8.f, wp.y + 44.f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(.14f, .14f, .14f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(.17f, .17f, .17f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(.19f, .19f, .19f, 1.f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.f);
        ImGui::SetNextItemWidth(rW - 16.f);
        ImGui::InputText("##knife_search", m_knifeSearchBuf, sizeof(m_knifeSearchBuf));
        if (m_knifeSearchBuf[0] == '\0') {
            ImVec2 fp = ImGui::GetItemRectMin();
            d->AddText(ImVec2(fp.x + 7.f, fp.y + 3.f), IM_COL32(65, 65, 65, 255), "Search knives...");
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);

        ImGui::SetCursorScreenPos(ImVec2(rX, wp.y + 38.f + searchH + 1.f));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(.085f, .085f, .085f, 1.f));
        ImGui::BeginChild("##knife_list", ImVec2(rW, H - 38.f - searchH - 1.f), false);
        const float rowH = 48.f;
        bool anyShown = false;
        int catDef = (m_knifePickerCategory > 0) ? m_categories[m_knifePickerCategory].defIndex : -1;
        for (int i = 0; i < (int)m_knives.size(); i++) {
            const auto& knife = m_knives[i];
            if (catDef != -1 && knife.defIndex != catDef) continue;
            if (m_knifeSearchBuf[0] && !StrContainsCI(knife.name, m_knifeSearchBuf)) continue;
            anyShown = true;
            ImVec2 rp = ImGui::GetCursorScreenPos();
            float listW = ImGui::GetContentRegionAvail().x;
            bool hov = ImGui::IsMouseHoveringRect(rp, ImVec2(rp.x + listW, rp.y + rowH));
            if (hov) d->AddRectFilled(rp, ImVec2(rp.x + listW, rp.y + rowH), IM_COL32(255, 255, 255, 8));
            d->AddText(ImVec2(rp.x + 14.f, rp.y + 14.f), IM_COL32(210, 210, 210, 255), knife.name);
            d->AddLine(ImVec2(rp.x + 10.f, rp.y + rowH - 1.f), ImVec2(rp.x + listW - 10.f, rp.y + rowH - 1.f), IM_COL32(20, 20, 20, 255));
            ImGui::SetCursorScreenPos(rp);
            char kid[32]; sprintf_s(kid, "##knife_%d", i);
            if (ImGui::InvisibleButton(kid, ImVec2(listW, rowH))) {
                m_selectedKnifeIndex = i;
                m_showKnifePicker = false;
                m_showSkinPicker = true;
                m_skinSearchBuf[0] = '\0';
            }
        }
        if (!anyShown) {
            float tw = ImGui::CalcTextSize("No knives found.").x;
            ImGui::SetCursorPos(ImVec2((rW - tw) * .5f, 70.f));
            ImGui::TextDisabled("No knives found.");
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }
    ImGui::End();
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
    if (!open) m_showKnifePicker = false;
}

void CSkinChanger::RenderSkinPicker() {
    if (m_selectedKnifeIndex < 0 || m_selectedKnifeIndex >= (int)m_knives.size()) {
        m_showSkinPicker = false;
        return;
    }
    const KnifeInfo& knife = m_knives[m_selectedKnifeIndex];
    auto it = m_skinsByDefIndex.find(knife.defIndex);
    if (it == m_skinsByDefIndex.end()) {
        m_showSkinPicker = false;
        return;
    }
    const auto& skins = it->second;

    ImVec2 ds = ImGui::GetIO().DisplaySize;
    const float W = 500.f, H = 400.f;
    ImGui::SetNextWindowPos(ImVec2((ds.x - W) * .5f, (ds.y - H) * .5f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(W, H));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(.085f, .085f, .085f, .98f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(.16f, .16f, .16f, 1.f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    bool open = true;
    if (ImGui::Begin("##SkinPicker", &open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove)) {
        ImDrawList* d = ImGui::GetWindowDrawList();
        ImVec2 wp = ImGui::GetWindowPos();
        d->AddRectFilled(wp, ImVec2(wp.x + W, wp.y + 38.f), IM_COL32(18, 18, 18, 255), 6.f);
        d->AddRectFilled(ImVec2(wp.x, wp.y + 32.f), ImVec2(wp.x + W, wp.y + 38.f), IM_COL32(18, 18, 18, 255));
        d->AddRectFilled(wp, ImVec2(wp.x + 3.f, wp.y + 38.f), IM_COL32(255, 50, 50, 255));
        d->AddText(ImVec2(wp.x + 14.f, wp.y + 11.f), IM_COL32(230, 230, 230, 255), "Select Skin for ");
        d->AddText(ImVec2(wp.x + 150.f, wp.y + 11.f), IM_COL32(255, 200, 50, 255), knife.name);
        ImGui::SetCursorScreenPos(ImVec2(wp.x + W - 30.f, wp.y + 9.f));
        if (ImGui::InvisibleButton("##close_skin", ImVec2(22.f, 20.f))) m_showSkinPicker = false;
        d->AddText(ImVec2(wp.x + W - 25.f, wp.y + 11.f),
            ImGui::IsItemHovered() ? IM_COL32(255, 100, 100, 255) : IM_COL32(120, 120, 120, 255), "x");
        d->AddLine(ImVec2(wp.x, wp.y + 38.f), ImVec2(wp.x + W, wp.y + 38.f), IM_COL32(25, 25, 25, 255));

        const float searchH = 34.f;
        d->AddRectFilled(ImVec2(wp.x, wp.y + 38.f), ImVec2(wp.x + W, wp.y + 38.f + searchH), IM_COL32(15, 15, 15, 255));
        ImGui::SetCursorScreenPos(ImVec2(wp.x + 8.f, wp.y + 44.f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(.14f, .14f, .14f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(.17f, .17f, .17f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(.19f, .19f, .19f, 1.f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.f);
        ImGui::SetNextItemWidth(W - 16.f);
        ImGui::InputText("##skin_search", m_skinSearchBuf, sizeof(m_skinSearchBuf));
        if (m_skinSearchBuf[0] == '\0') {
            ImVec2 fp = ImGui::GetItemRectMin();
            d->AddText(ImVec2(fp.x + 7.f, fp.y + 3.f), IM_COL32(65, 65, 65, 255), "Search skins...");
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);

        ImGui::SetCursorScreenPos(ImVec2(wp.x, wp.y + 38.f + searchH + 2.f));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(.085f, .085f, .085f, 1.f));
        ImGui::BeginChild("##skin_list", ImVec2(W, H - 38.f - searchH - 2.f), false);
        const float rowH = 48.f;
        bool anyShown = false;
        for (int i = 0; i < (int)skins.size(); i++) {
            const auto& s = skins[i];
            if (m_skinSearchBuf[0] && !StrContainsCI(s.skinName, m_skinSearchBuf)) continue;
            anyShown = true;
            ImU32 rc = GetRarityColor(s.rarity);
            ImVec2 rp = ImGui::GetCursorScreenPos();
            float listW = ImGui::GetContentRegionAvail().x;
            bool hov = ImGui::IsMouseHoveringRect(rp, ImVec2(rp.x + listW, rp.y + rowH));
            if (hov) d->AddRectFilled(rp, ImVec2(rp.x + listW, rp.y + rowH), IM_COL32(255, 255, 255, 8));
            d->AddRectFilled(ImVec2(rp.x, rp.y + 10.f), ImVec2(rp.x + 3.f, rp.y + rowH - 10.f), rc);
            d->AddText(ImVec2(rp.x + 14.f, rp.y + 10.f), IM_COL32(210, 210, 210, 255), s.skinName);
            d->AddText(ImVec2(rp.x + 14.f, rp.y + 27.f), rc, GetRarityLabel(s.rarity));
            d->AddLine(ImVec2(rp.x + 10.f, rp.y + rowH - 1.f), ImVec2(rp.x + listW - 10.f, rp.y + rowH - 1.f), IM_COL32(20, 20, 20, 255));
            ImGui::SetCursorScreenPos(rp);
            char sid[32]; sprintf_s(sid, "##skinitem_%d", i);
            if (ImGui::InvisibleButton(sid, ImVec2(listW, rowH))) {
                SetActiveSkin(knife.defIndex, s.paintKit, 0.01f, 1, knife.name, s.skinName, s.rarity);
                m_showSkinPicker = false;
            }
        }
        if (!anyShown) {
            float tw = ImGui::CalcTextSize("No skins found.").x;
            ImGui::SetCursorPos(ImVec2((W - tw) * .5f, 70.f));
            ImGui::TextDisabled("No skins found.");
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }
    ImGui::End();
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
    if (!open) m_showSkinPicker = false;
}