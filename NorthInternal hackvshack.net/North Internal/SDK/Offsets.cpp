#include "Offsets.hpp"
#include "../Core/PatternScanner.hpp"
#include <windows.h>
#include <wininet.h>
#include <string>
#include <regex>
#include <unordered_map>

#pragma comment(lib, "wininet.lib")

struct HttpResult {
    std::string body;
    std::string etag;
    std::string lastModified;
    int statusCode = 0;
};

static HttpResult HttpGet(const char* url, const char* ifNoneMatch = nullptr, const char* ifModifiedSince = nullptr) {
    HttpResult result;

    HINTERNET hInternet = InternetOpenA("North/1.0", INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0);
    if (!hInternet) return result;

    HINTERNET hConnect = InternetOpenUrlA(hInternet, url, nullptr, 0,
        INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_KEEP_CONNECTION, 0);
    if (!hConnect) {
        InternetCloseHandle(hInternet);
        return result;
    }

    DWORD statusCode = 0;
    DWORD statusSize = sizeof(statusCode);
    HttpQueryInfoA(hConnect, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &statusCode, &statusSize, nullptr);
    result.statusCode = static_cast<int>(statusCode);

    char headerBuf[512];
    DWORD headerSize = sizeof(headerBuf);
    if (HttpQueryInfoA(hConnect, HTTP_QUERY_ETAG, headerBuf, &headerSize, nullptr))
        result.etag = headerBuf;

    headerSize = sizeof(headerBuf);
    if (HttpQueryInfoA(hConnect, HTTP_QUERY_LAST_MODIFIED, headerBuf, &headerSize, nullptr))
        result.lastModified = headerBuf;

    if (result.statusCode == 200) {
        char buf[8192];
        DWORD bytesRead = 0;
        while (InternetReadFile(hConnect, buf, sizeof(buf) - 1, &bytesRead) && bytesRead > 0) {
            buf[bytesRead] = '\0';
            result.body.append(buf, bytesRead);
        }
    }

    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
    return result;
}

struct CachedFile {
    std::string content;
    std::string etag;
    std::string lastModified;
    bool valid = false;
};

static CachedFile FetchWithCache(const char* url, CachedFile& cache) {
    if (!cache.valid) {
        HttpResult res = HttpGet(url);
        if (res.statusCode == 200 && !res.body.empty()) {
            cache.content = std::move(res.body);
            cache.etag = std::move(res.etag);
            cache.lastModified = std::move(res.lastModified);
            cache.valid = true;
        }
        return cache;
    }

    HttpResult res = HttpGet(url,
        cache.etag.empty() ? nullptr : cache.etag.c_str(),
        cache.lastModified.empty() ? nullptr : cache.lastModified.c_str());

    if (res.statusCode == 304)
        return cache;

    if (res.statusCode == 200 && !res.body.empty()) {
        cache.content = std::move(res.body);
        cache.etag = std::move(res.etag);
        cache.lastModified = std::move(res.lastModified);
    }

    return cache;
}

static void ParseAllOffsets(const std::string& content, std::unordered_map<std::string, uintptr_t>& map) {
    static const std::regex pattern(R"(constexpr\s+std::\w+\s+(\w+)\s*=\s*(0x[0-9A-Fa-f]+))");
    std::sregex_iterator it(content.begin(), content.end(), pattern), end;
    while (it != end) {
        const std::smatch& m = *it;
        map[m[1].str()] = std::stoull(m[2].str(), nullptr, 16);
        ++it;
    }
}

static uintptr_t FindBoneArrayOffset() {
    static const char* patterns[] = {
        "48 8B 83 ? ? 00 00 48 85 C0 74 ? 4C 8B",
        "4C 8B 87 ? ? 00 00 4D 85 C0 74 ? 4C 8B",
        "48 8B 86 ? ? 00 00 48 85 C0 74 ? 4C 8B",
    };
    for (const char* pat : patterns) {
        uintptr_t hit = PatternScanner::FindPattern("client.dll", pat);
        if (!hit) continue;
        __try {
            uint32_t offset = *reinterpret_cast<const uint32_t*>(hit + 3);
            if (offset >= 0x40 && offset <= 0x200)
                return offset;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
    return 0x80;
}

static uintptr_t FindWeaponWorldModelOffset() {
    static const char* patterns[] = {
        "48 8B 8B ? ? 00 00 48 85 C9 74 ? 33 D2 E8",
        "48 8B 87 ? ? 00 00 48 85 C0 74 ? 48 8B 40",
    };
    for (const char* pat : patterns) {
        uintptr_t hit = PatternScanner::FindPattern("client.dll", pat);
        if (!hit) continue;
        __try {
            uint32_t offset = *reinterpret_cast<const uint32_t*>(hit + 3);
            if (offset >= 0x200 && offset <= 0x400)
                return offset;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
    return 0x2E0;
}

static CachedFile s_offsetsCache;
static CachedFile s_clientCache;

static bool ApplyOffsets(const std::unordered_map<std::string, uintptr_t>& offsetsMap,
    const std::unordered_map<std::string, uintptr_t>& clientMap) {
    auto getOff = [&](const char* name) -> uintptr_t {
        auto it = offsetsMap.find(name);
        return it != offsetsMap.end() ? it->second : 0;
        };
    auto getCli = [&](const char* name) -> uintptr_t {
        auto it = clientMap.find(name);
        return it != clientMap.end() ? it->second : 0;
        };
    auto invOr = [&](const char* name, uintptr_t fallback) -> uintptr_t {
        uintptr_t v = getCli(name);
        return v ? v : fallback;
        };

    Offsets::dwEntityList = getOff("dwEntityList");
    Offsets::dwLocalPlayerPawn = getOff("dwLocalPlayerPawn");
    Offsets::dwLocalPlayerController = getOff("dwLocalPlayerController");
    Offsets::dwViewMatrix = getOff("dwViewMatrix");
    Offsets::dwViewAngles = getOff("dwViewAngles");
    Offsets::dwNetworkGameClient = getOff("dwNetworkGameClient");
    if (!Offsets::dwNetworkGameClient) Offsets::dwNetworkGameClient = 0x90A1A0;
    Offsets::dwNetworkGameClient_deltaTick = getOff("dwNetworkGameClient_deltaTick");
    if (!Offsets::dwNetworkGameClient_deltaTick) Offsets::dwNetworkGameClient_deltaTick = 0x24C;

    Schemas::C_BaseEntity::m_iHealth = getCli("m_iHealth");
    Schemas::C_BaseEntity::m_iTeamNum = getCli("m_iTeamNum");
    Schemas::C_BaseEntity::m_pGameSceneNode = getCli("m_pGameSceneNode");
    Schemas::C_BaseEntity::m_nSubclassID = getCli("m_nSubclassID");
    Schemas::C_BasePlayerPawn::m_vOldOrigin = getCli("m_vOldOrigin");
    Schemas::C_BasePlayerPawn::m_pWeaponServices = getCli("m_pWeaponServices");
    Schemas::C_BasePlayerPawn::m_fFlags = getCli("m_fFlags");
    Schemas::C_BasePlayerPawn::m_vecVelocity = getCli("m_vecVelocity");
    Schemas::C_CSPlayerPawnBase::m_iIDEntIndex = getCli("m_iIDEntIndex");
    Schemas::C_CSPlayerPawnBase::m_pViewModelServices = getCli("m_pViewModelServices");
    Schemas::C_CSPlayerPawnBase::m_vecViewOffset = getCli("m_vecViewOffset");
    Schemas::C_CSPlayerPawnBase::m_pObserverServices = getCli("m_pObserverServices");
    Schemas::CGameSceneNode::m_vecAbsOrigin = getCli("m_vecAbsOrigin");
    Schemas::CGameSceneNode::m_modelState = getCli("m_modelState");
    Schemas::CCSPlayerController::m_hPlayerPawn = getCli("m_hPlayerPawn");
    Schemas::CCSPlayerController::m_hPawn = getCli("m_hPawn");
    Schemas::CCSPlayerController::m_sSanitizedPlayerName = getCli("m_sSanitizedPlayerName");
    Schemas::CCSPlayerController::m_steamID = getCli("m_steamID");
    Schemas::C_CSPlayerPawn::m_bIsScoped = getCli("m_bIsScoped");
    Schemas::C_CSPlayerPawn::m_ArmorValue = getCli("m_ArmorValue");
    Schemas::C_CSPlayerPawn::m_flFlashOverlayAlpha = getCli("m_flFlashOverlayAlpha");
    Schemas::C_CSPlayerPawn::m_pCameraServices = getCli("m_pCameraServices");
    Schemas::CCSPlayerBase_CameraServices::m_iFOV = getCli("m_iFOV");
    Schemas::CCSPlayerBase_CameraServices::m_iFOVStart = getCli("m_iFOVStart");
    Schemas::CCSPlayerBase_CameraServices::m_flFOVRate = getCli("m_flFOVRate");
    Schemas::CCSPlayerBase_CameraServices::m_flThirdPersonDistance = getCli("m_flThirdPersonDistance");
    Schemas::CPlayer_ViewModelServices::m_hViewModel = getCli("m_hViewModel");
    Schemas::C_BaseViewModel::m_vecOrigin = getCli("m_vecOrigin");
    Schemas::C_BaseViewModel::m_angRotation = getCli("m_angRotation");
    Schemas::C_BaseViewModel::m_flFOV = getCli("m_flFOV");
    Schemas::CPlayer_ObserverServices::m_iObserverMode = getCli("m_iObserverMode");
    Schemas::CPlayer_WeaponServices::m_hMyWeapons = getCli("m_hMyWeapons");
    Schemas::CPlayer_WeaponServices::m_hActiveWeapon = getCli("m_hActiveWeapon");
    Schemas::C_EconEntity::m_AttributeManager = 0x1180;
    Schemas::C_AttributeContainer::m_Item = getCli("m_Item");
    Schemas::C_EconItemView::m_iItemDefinitionIndex = getCli("m_iItemDefinitionIndex");
    Schemas::C_EconItemView::m_iEntityQuality = getCli("m_iEntityQuality");
    Schemas::C_EconItemView::m_iItemIDHigh = getCli("m_iItemIDHigh");
    Schemas::C_EconItemView::m_iItemIDLow = getCli("m_iItemIDLow");
    Schemas::C_EconItemView::m_iAccountID = getCli("m_iAccountID");
    Schemas::C_EconEntity::m_nFallbackPaintKit = getCli("m_nFallbackPaintKit");
    Schemas::C_EconEntity::m_flFallbackWear = getCli("m_flFallbackWear");
    Schemas::C_EconEntity::m_nFallbackSeed = getCli("m_nFallbackSeed");
    Schemas::C_EconEntity::m_nFallbackStatTrak = getCli("m_nFallbackStatTrak");
    Schemas::C_EconEntity::m_OriginalOwnerXuidLow = getCli("m_OriginalOwnerXuidLow");
    Schemas::C_EconEntity::m_OriginalOwnerXuidHigh = getCli("m_OriginalOwnerXuidHigh");
    Schemas::C_BaseModelEntity::m_CBodyComponent = getCli("m_CBodyComponent");
    Schemas::CModelState::m_boneArray = FindBoneArrayOffset();
    Schemas::C_CSWeaponBase::m_hWeaponWorldModel = FindWeaponWorldModelOffset();

    static const char* fnSetModelPatterns[] = {
        "48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 49 8B F8 48 8B F2 48 8B D9 48 85 D2 74",
        "48 89 5C 24 08 57 48 83 EC 20 48 8B FA 48 8B D9 48 85 D2 0F 84 ? ? ? ? 48 8B 0D",
        "40 53 48 83 EC 20 48 8B DA 48 85 D2 74 ? 48 8B 0D ? ? ? ? 48 85 C9 74",
    };
    for (const char* pat : fnSetModelPatterns) {
        uintptr_t found = PatternScanner::FindPattern("client.dll", pat);
        if (found) { Functions::fnSetModel = found; break; }
    }

    return Offsets::dwEntityList != 0 && Offsets::dwLocalPlayerPawn != 0;
}

bool Offsets::Initialize() {
    if (!reinterpret_cast<uintptr_t>(GetModuleHandleA("client.dll")))
        return false;

    static const char* URL_OFFSETS = "https://raw.githubusercontent.com/a2x/cs2-dumper/refs/heads/main/output/offsets.hpp";
    static const char* URL_CLIENT_DLL = "https://raw.githubusercontent.com/a2x/cs2-dumper/refs/heads/main/output/client_dll.hpp";

    CachedFile offsets = FetchWithCache(URL_OFFSETS, s_offsetsCache);
    CachedFile clientDll = FetchWithCache(URL_CLIENT_DLL, s_clientCache);

    std::unordered_map<std::string, uintptr_t> offsetsMap, clientMap;
    if (offsets.valid)   ParseAllOffsets(offsets.content, offsetsMap);
    if (clientDll.valid) ParseAllOffsets(clientDll.content, clientMap);

    return ApplyOffsets(offsetsMap, clientMap);
}

bool Offsets::Refresh() {
    if (!reinterpret_cast<uintptr_t>(GetModuleHandleA("client.dll")))
        return false;

    static const char* URL_OFFSETS = "https://raw.githubusercontent.com/a2x/cs2-dumper/refs/heads/main/output/offsets.hpp";
    static const char* URL_CLIENT_DLL = "https://raw.githubusercontent.com/a2x/cs2-dumper/refs/heads/main/output/client_dll.hpp";

    bool changed = false;
    auto checkFetch = [&](const char* url, CachedFile& cache) {
        std::string oldEtag = cache.etag;
        FetchWithCache(url, cache);
        if (cache.etag != oldEtag) changed = true;
        };
    checkFetch(URL_OFFSETS, s_offsetsCache);
    checkFetch(URL_CLIENT_DLL, s_clientCache);

    if (!changed) return true;
    std::unordered_map<std::string, uintptr_t> offsetsMap, clientMap;
    if (s_offsetsCache.valid) ParseAllOffsets(s_offsetsCache.content, offsetsMap);
    if (s_clientCache.valid)  ParseAllOffsets(s_clientCache.content, clientMap);
    return ApplyOffsets(offsetsMap, clientMap);
}