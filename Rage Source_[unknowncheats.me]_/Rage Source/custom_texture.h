#pragma once
#include <d3d11.h>
#include <vector>
#include <unordered_set>
#include <cstdint>
#include <cstdio>
#include "stb_image.h"

// ============================================================
// Custom Texture Replacement
// 
// Hooks PSSetShaderResources to intercept texture bindings.
// Provides a "browse mode" to cycle through textures until
// the user finds the weapon skin, then locks it for replacement.
//
// Usage:
//   1. Load a custom image (PNG/JPG)
//   2. Enable + Start Browse mode
//   3. Press F10/F11 to cycle — watch your weapon change
//   4. When your weapon shows the custom image, press F12 to lock
// ============================================================

namespace CustomTexture
{
    // --- D3D11 Resources ---
    inline ID3D11ShaderResourceView* customSRV = nullptr;
    inline ID3D11Texture2D* customTex = nullptr;

    // --- Settings ---
    inline bool enabled = false;
    inline bool browseMode = false;
    inline char imagePath[260] = "";
    inline bool imageLoaded = false;
    inline int filterWidth = 0;   // 0 = any width
    inline int filterHeight = 0;  // 0 = any height
    inline int minSize = 64;      // minimum texture dimension
    inline bool noFilter = true;  // accept ALL 2D textures (easiest for finding skins)

    // --- Browse State ---
    inline std::vector<ID3D11ShaderResourceView*> candidates;
    inline std::unordered_set<ID3D11ShaderResourceView*> seenSet;
    inline int browseIndex = 0;
    inline ID3D11ShaderResourceView* lockedSRV = nullptr;
    inline int candidateCount = 0;

    // --- Debug Counters ---
    inline int hookCalls = 0;
    inline int totalUniqueSRVs = 0;

    // --- Hook State ---
    using tPSSetShaderResources = void(__stdcall*)(
        ID3D11DeviceContext*, UINT, UINT, ID3D11ShaderResourceView* const*);
    inline tPSSetShaderResources oFunc = nullptr;
    inline uintptr_t* pVtableSlot = nullptr;
    inline bool hooked = false;

    // -------------------------------------------------------
    // Load a custom image from disk → D3D11 texture + SRV
    // -------------------------------------------------------
    inline bool LoadCustomImage(ID3D11Device* device, const char* path)
    {
        // Release old
        if (customSRV)  { customSRV->Release();  customSRV = nullptr; }
        if (customTex)  { customTex->Release();  customTex = nullptr; }
        imageLoaded = false;

        if (!device || !path || !path[0]) return false;

        int w, h, ch;
        unsigned char* pixels = stbi_load(path, &w, &h, &ch, 4);
        if (!pixels)
        {
            printf("[CustomTex] stbi_load failed: %s\n", path);
            return false;
        }

        D3D11_TEXTURE2D_DESC td = {};
        td.Width  = (UINT)w;
        td.Height = (UINT)h;
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        td.SampleDesc.Count = 1;
        td.Usage  = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA init = {};
        init.pSysMem = pixels;
        init.SysMemPitch = w * 4;

        HRESULT hr = device->CreateTexture2D(&td, &init, &customTex);
        stbi_image_free(pixels);

        if (FAILED(hr))
        {
            printf("[CustomTex] CreateTexture2D failed: 0x%08lX\n", hr);
            return false;
        }

        hr = device->CreateShaderResourceView(customTex, nullptr, &customSRV);
        if (FAILED(hr))
        {
            printf("[CustomTex] CreateSRV failed: 0x%08lX\n", hr);
            customTex->Release(); customTex = nullptr;
            return false;
        }

        imageLoaded = true;
        printf("[CustomTex] Loaded: %s (%dx%d)\n", path, w, h);
        return true;
    }

    // -------------------------------------------------------
    // Check if an SRV's underlying texture matches our filter
    // -------------------------------------------------------
    inline bool IsCandidate(ID3D11ShaderResourceView* srv)
    {
        if (!srv) return false;

        ID3D11Resource* res = nullptr;
        srv->GetResource(&res);
        if (!res) return false;

        D3D11_RESOURCE_DIMENSION dim;
        res->GetType(&dim);

        bool match = false;
        if (dim == D3D11_RESOURCE_DIMENSION_TEXTURE2D)
        {
            auto* tex = static_cast<ID3D11Texture2D*>(res);
            D3D11_TEXTURE2D_DESC d;
            tex->GetDesc(&d);

            if (noFilter)
            {
                // Accept ALL 2D textures above minimum size
                match = (d.Width >= (UINT)minSize && d.Height >= (UINT)minSize);
            }
            else
            {
                bool sizeOk = true;
                if (filterWidth  > 0) sizeOk = sizeOk && (d.Width  == (UINT)filterWidth);
                if (filterHeight > 0) sizeOk = sizeOk && (d.Height == (UINT)filterHeight);
                bool bigEnough = (d.Width >= (UINT)minSize && d.Height >= (UINT)minSize);
                match = sizeOk && bigEnough;
            }
        }

        res->Release();
        return match;
    }

    // -------------------------------------------------------
    // PSSetShaderResources hook — the hot path
    // -------------------------------------------------------
    inline void __stdcall hkPSSetShaderResources(
        ID3D11DeviceContext* ctx, UINT StartSlot, UINT NumViews,
        ID3D11ShaderResourceView* const* ppSRVs)
    {
        hookCalls++;

        if (enabled && ppSRVs && NumViews > 0)
        {
            // Browse mode: discover new candidate textures
            // (runs even without customSRV loaded, for counting)
            if (browseMode)
            {
                for (UINT i = 0; i < NumViews; i++)
                {
                    if (!ppSRVs[i]) continue;
                    if (seenSet.count(ppSRVs[i]) == 0)
                    {
                        seenSet.insert(ppSRVs[i]);
                        totalUniqueSRVs++;
                        if (IsCandidate(ppSRVs[i]))
                            candidates.push_back(ppSRVs[i]);
                    }
                }
                candidateCount = (int)candidates.size();
            }

            // Replacement logic (only if we have a custom texture loaded)
            if (customSRV)
            {
                ID3D11ShaderResourceView* target = nullptr;
                if (browseMode && browseIndex >= 0 && browseIndex < (int)candidates.size())
                    target = candidates[browseIndex];
                else if (!browseMode && lockedSRV)
                    target = lockedSRV;

                if (target)
                {
                    for (UINT i = 0; i < NumViews; i++)
                    {
                        if (ppSRVs[i] == target)
                        {
                            ID3D11ShaderResourceView* mod[128];
                            UINT count = (NumViews < 128) ? NumViews : 128;
                            for (UINT j = 0; j < count; j++)
                                mod[j] = (ppSRVs[j] == target) ? customSRV : ppSRVs[j];

                            oFunc(ctx, StartSlot, count, mod);
                            return;
                        }
                    }
                }
            }
        }

        oFunc(ctx, StartSlot, NumViews, ppSRVs);
    }

    // -------------------------------------------------------
    // Browse controls
    // -------------------------------------------------------
    inline void BrowseNext()
    {
        if (candidates.empty()) return;
        browseIndex = (browseIndex + 1) % (int)candidates.size();
        printf("[CustomTex] Browse: %d / %d\n", browseIndex + 1, (int)candidates.size());
    }

    inline void BrowsePrev()
    {
        if (candidates.empty()) return;
        browseIndex--;
        if (browseIndex < 0) browseIndex = (int)candidates.size() - 1;
        printf("[CustomTex] Browse: %d / %d\n", browseIndex + 1, (int)candidates.size());
    }

    inline void LockCurrent()
    {
        if (browseIndex >= 0 && browseIndex < (int)candidates.size())
        {
            lockedSRV = candidates[browseIndex];
            browseMode = false;
            printf("[CustomTex] LOCKED texture #%d (ptr=0x%p)\n", browseIndex, lockedSRV);
        }
    }

    inline void Unlock()
    {
        lockedSRV = nullptr;
        printf("[CustomTex] Unlocked\n");
    }

    inline void ResetBrowse()
    {
        seenSet.clear();
        candidates.clear();
        browseIndex = 0;
        candidateCount = 0;
        lockedSRV = nullptr;
    }

    // -------------------------------------------------------
    // Init — hook PSSetShaderResources via vtable swap
    // -------------------------------------------------------
    inline bool Init(ID3D11DeviceContext* context)
    {
        if (!context || hooked) return false;

        uintptr_t* vtable = *reinterpret_cast<uintptr_t**>(context);
        pVtableSlot = &vtable[8]; // PSSetShaderResources = vtable index 8

        DWORD oldProt;
        VirtualProtect(pVtableSlot, sizeof(uintptr_t), PAGE_READWRITE, &oldProt);
        oFunc = reinterpret_cast<tPSSetShaderResources>(*pVtableSlot);
        *pVtableSlot = reinterpret_cast<uintptr_t>(&hkPSSetShaderResources);
        VirtualProtect(pVtableSlot, sizeof(uintptr_t), oldProt, &oldProt);

        hooked = true;
        printf("[CustomTex] PSSetShaderResources hooked (vtable[8])\n");
        return true;
    }

    // -------------------------------------------------------
    // Shutdown — restore vtable, release resources
    // -------------------------------------------------------
    inline void Shutdown()
    {
        if (pVtableSlot && oFunc)
        {
            DWORD oldProt;
            VirtualProtect(pVtableSlot, sizeof(uintptr_t), PAGE_READWRITE, &oldProt);
            *pVtableSlot = reinterpret_cast<uintptr_t>(oFunc);
            VirtualProtect(pVtableSlot, sizeof(uintptr_t), oldProt, &oldProt);
            oFunc = nullptr;
        }
        hooked = false;

        if (customSRV) { customSRV->Release(); customSRV = nullptr; }
        if (customTex) { customTex->Release(); customTex = nullptr; }

        seenSet.clear();
        candidates.clear();
        lockedSRV = nullptr;
    }

    // -------------------------------------------------------
    // Handle keyboard shortcuts (call from WndProc or Tick)
    // -------------------------------------------------------
    inline void HandleInput()
    {
        if (!enabled || !imageLoaded) return;

        // F10 = prev texture, F11 = next texture, F12 = lock/unlock
        if (browseMode)
        {
            if (GetAsyncKeyState(VK_F10) & 1) BrowsePrev();
            if (GetAsyncKeyState(VK_F11) & 1) BrowseNext();
            if (GetAsyncKeyState(VK_F12) & 1) LockCurrent();
        }
        else
        {
            if (GetAsyncKeyState(VK_F12) & 1)
            {
                if (lockedSRV)
                    Unlock();
            }
        }
    }
}
