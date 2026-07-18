#pragma once
#include <cstdint>

namespace Offsets {
    inline uintptr_t dwEntityList;
    inline uintptr_t dwLocalPlayerPawn;
    inline uintptr_t dwLocalPlayerController;
    inline uintptr_t dwViewMatrix;
    inline uintptr_t dwViewAngles;
    inline uintptr_t dwNetworkGameClient;
    inline uintptr_t dwNetworkGameClient_deltaTick;

    bool Initialize();
    bool Refresh();
}

namespace Schemas {
    namespace C_BaseEntity {
        inline uintptr_t m_iHealth;
        inline uintptr_t m_iTeamNum;
        inline uintptr_t m_pGameSceneNode;
        inline uintptr_t m_nSubclassID;
    }
    namespace C_BasePlayerPawn {
        inline uintptr_t m_vOldOrigin;
        inline uintptr_t m_pWeaponServices;
        inline uintptr_t m_fFlags;
        inline uintptr_t m_vecVelocity;
    }
    namespace C_CSPlayerPawnBase {
        inline uintptr_t m_iIDEntIndex;
        inline uintptr_t m_pViewModelServices;
        inline uintptr_t m_vecViewOffset;
        inline uintptr_t m_pObserverServices;
    }
    namespace CGameSceneNode {
        inline uintptr_t m_vecAbsOrigin;
        inline uintptr_t m_modelState;
    }
    namespace CModelState {
        inline uintptr_t m_boneArray;
    }
    namespace C_CSPlayerPawn {
        inline uintptr_t m_bIsScoped;
        inline uintptr_t m_ArmorValue;
        inline uintptr_t m_flFlashOverlayAlpha;
        inline uintptr_t m_pCameraServices;
    }
    namespace CCSPlayerController {
        inline uintptr_t m_hPlayerPawn;
        inline uintptr_t m_hPawn;
        inline uintptr_t m_sSanitizedPlayerName;
        inline uintptr_t m_steamID;
    }
    namespace CPlayer_WeaponServices {
        inline uintptr_t m_hMyWeapons;
        inline uintptr_t m_hActiveWeapon;
    }
    namespace C_AttributeContainer {
        inline uintptr_t m_Item;
    }
    namespace C_EconEntity {
        inline uintptr_t m_AttributeManager;
        inline uintptr_t m_nFallbackPaintKit;
        inline uintptr_t m_flFallbackWear;
        inline uintptr_t m_nFallbackSeed;
        inline uintptr_t m_nFallbackStatTrak;
        inline uintptr_t m_OriginalOwnerXuidLow;
        inline uintptr_t m_OriginalOwnerXuidHigh;
    }
    namespace C_EconItemView {
        inline uintptr_t m_iItemDefinitionIndex;
        inline uintptr_t m_iEntityQuality;
        inline uintptr_t m_iItemIDHigh;
        inline uintptr_t m_iItemIDLow;
        inline uintptr_t m_iAccountID;
    }
    namespace C_BaseModelEntity {
        inline uintptr_t m_CBodyComponent;
    }
    namespace CCSPlayerBase_CameraServices {
        inline uintptr_t m_iFOV;
        inline uintptr_t m_iFOVStart;
        inline uintptr_t m_flFOVRate;
        inline uintptr_t m_flThirdPersonDistance;
    }
    namespace CPlayer_ViewModelServices {
        inline uintptr_t m_hViewModel;
    }
    namespace C_BaseViewModel {
        inline uintptr_t m_vecOrigin;
        inline uintptr_t m_angRotation;
        inline uintptr_t m_flFOV;
    }
    namespace CPlayer_ObserverServices {
        inline uintptr_t m_iObserverMode;
    }
    namespace C_CSWeaponBase {
        inline uintptr_t m_hWeaponWorldModel;
    }
}

namespace Functions {
    inline uintptr_t fnSetModel;
}