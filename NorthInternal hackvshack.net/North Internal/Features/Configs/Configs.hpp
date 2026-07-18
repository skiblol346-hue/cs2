#pragma once
#include <string>
#include <vector>
#include "../../Dependencies/ImGui/imgui.h"

struct ConfigEntry {
    std::string name;
    std::string filename;
    std::string modified;
    bool        isDefault;
};

class CConfigSystem {
public:
    void Initialize();
    void Render();

private:
    static std::string GetDir();
    std::string        GetTimestamp() const;
    void               EnsureDir() const;
    void               Refresh();
    bool               SaveToFile(const std::string& path);
    bool               LoadFromFile(const std::string& path);
    void               DeleteEntry(int idx);
    void               RenameEntry(int idx, const std::string& newName);
    std::string        Serialize() const;
    void               Deserialize(const std::string& data);

    static bool  RBool(const std::string& j, const std::string& k, bool  d = false);
    static int   RInt(const std::string& j, const std::string& k, int   d = 0);
    static float RFloat(const std::string& j, const std::string& k, float d = 0.f);
    static void  RBoolArray(const std::string& j, const std::string& k, bool* out, int n);
    static void  RFloatArray(const std::string& j, const std::string& k, float* out, int n);

    std::vector<ConfigEntry> m_list;
    int   m_loadedIdx = -1;
    float m_autoSaveAcc = 0.f;
    bool  m_initialized = false;

    char m_createBuf[64] = {};
    int  m_editIdx = -1;
    char m_editBuf[64] = {};
};

extern CConfigSystem g_Configs;