#include "View.hpp"
#include "../Menu/Menu.hpp"
#include "../../SDK/Offsets.hpp"
#include <cmath>

CView g_View;

template<typename T>
static bool SafeRead(uintptr_t addr, T& out) {
    if (!addr) return false;
    __try {
        out = *reinterpret_cast<T*>(addr);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

template<typename T>
static bool SafeWrite(uintptr_t addr, const T& val) {
    if (!addr) return false;
    __try {
        *reinterpret_cast<T*>(addr) = val;
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static uintptr_t GetCameraServices(uintptr_t localPawn) {
    if (!localPawn || !Schemas::C_CSPlayerPawn::m_pCameraServices)
        return 0;
    uintptr_t camSvc = 0;
    SafeRead<uintptr_t>(localPawn + Schemas::C_CSPlayerPawn::m_pCameraServices, camSvc);
    return camSvc;
}

static uintptr_t GetViewModel(uintptr_t clientBase, uintptr_t localPawn) {
    if (!Schemas::C_CSPlayerPawnBase::m_pViewModelServices)
        return 0;

    uintptr_t vmServices = 0;
    if (!SafeRead<uintptr_t>(localPawn + Schemas::C_CSPlayerPawnBase::m_pViewModelServices, vmServices) || !vmServices)
        return 0;

    if (!Schemas::CPlayer_ViewModelServices::m_hViewModel)
        return 0;

    uint32_t vmHandle = 0;
    if (!SafeRead<uint32_t>(vmServices + Schemas::CPlayer_ViewModelServices::m_hViewModel, vmHandle))
        return 0;

    if (vmHandle == 0 || vmHandle == 0xFFFFFFFF)
        return 0;

    uintptr_t entityList = 0;
    if (!SafeRead<uintptr_t>(clientBase + Offsets::dwEntityList, entityList) || !entityList)
        return 0;

    uint32_t idx = vmHandle & 0x7FFF;
    uintptr_t listEntry = 0;
    if (!SafeRead<uintptr_t>(entityList + 0x8 * ((idx >> 9) + 1), listEntry) || !listEntry)
        return 0;

    uintptr_t vm = 0;
    SafeRead<uintptr_t>(listEntry + 0x78 * (idx & 0x1FF), vm);
    return vm;
}

void CView::Run(uintptr_t clientBase, uintptr_t localPawn) {
    if (!clientBase || !localPawn)
        return;

    int hp = 0;
    if (!SafeRead<int>(localPawn + Schemas::C_BaseEntity::m_iHealth, hp) || hp <= 0)
        return;

    ApplyFOV(localPawn);
    ApplyViewModelFOV(clientBase, localPawn);
    ApplyViewModelOffset(clientBase, localPawn);
    ApplyThirdPerson(localPawn);
    HandleFreeLook(clientBase);
}

void CView::ApplyFOV(uintptr_t localPawn) {
    uintptr_t camSvc = GetCameraServices(localPawn);
    if (!camSvc || !Schemas::CCSPlayerBase_CameraServices::m_iFOV)
        return;

    if (cam_fov > 0.0f) {
        SafeWrite<uint32_t>(camSvc + Schemas::CCSPlayerBase_CameraServices::m_iFOV, (uint32_t)cam_fov);
    }
    else {
        uint32_t current = 0;
        SafeRead<uint32_t>(camSvc + Schemas::CCSPlayerBase_CameraServices::m_iFOV, current);
        if (current != 0 && current != 90)
            SafeWrite<uint32_t>(camSvc + Schemas::CCSPlayerBase_CameraServices::m_iFOV, 0);
    }
}

void CView::ApplyViewModelFOV(uintptr_t clientBase, uintptr_t localPawn) {
    if (vm_fov == 60.0f)
        return;

    uintptr_t vm = GetViewModel(clientBase, localPawn);
    if (!vm || !Schemas::C_BaseViewModel::m_flFOV)
        return;

    SafeWrite<float>(vm + Schemas::C_BaseViewModel::m_flFOV, vm_fov);
}

void CView::ApplyViewModelOffset(uintptr_t clientBase, uintptr_t localPawn) {
    if (!vm_custom_origin && !vm_custom_angles)
        return;

    uintptr_t vm = GetViewModel(clientBase, localPawn);
    if (!vm)
        return;

    uintptr_t sceneNode = 0;
    if (!SafeRead<uintptr_t>(vm + Schemas::C_BaseEntity::m_pGameSceneNode, sceneNode) || !sceneNode)
        return;

    if (vm_custom_origin && Schemas::CGameSceneNode::m_vecAbsOrigin) {
        uintptr_t addr = sceneNode + Schemas::CGameSceneNode::m_vecAbsOrigin;
        float pos[3] = {};
        SafeRead<float>(addr, pos[0]);
        SafeRead<float>(addr + 4, pos[1]);
        SafeRead<float>(addr + 8, pos[2]);

        pos[0] += vm_origin_x;
        pos[1] += vm_origin_y;
        pos[2] += vm_origin_z;

        SafeWrite<float>(addr, pos[0]);
        SafeWrite<float>(addr + 4, pos[1]);
        SafeWrite<float>(addr + 8, pos[2]);
    }

    if (vm_custom_angles && Schemas::CGameSceneNode::m_vecAbsOrigin) {
        uintptr_t angAddr = sceneNode + Schemas::CGameSceneNode::m_vecAbsOrigin + 12;
        SafeWrite<float>(angAddr, vm_pitch);
        SafeWrite<float>(angAddr + 4, vm_yaw);
        SafeWrite<float>(angAddr + 8, vm_roll);
    }
}

void CView::ApplyThirdPerson(uintptr_t localPawn) {
    if (!Schemas::C_CSPlayerPawnBase::m_pObserverServices)
        return;

    uintptr_t obsSvc = 0;
    if (!SafeRead<uintptr_t>(localPawn + Schemas::C_CSPlayerPawnBase::m_pObserverServices, obsSvc) || !obsSvc)
        return;

    if (tp_enable) {
        bTPWasActive = true;

        if (Schemas::CPlayer_ObserverServices::m_iObserverMode)
            SafeWrite<uint32_t>(obsSvc + Schemas::CPlayer_ObserverServices::m_iObserverMode, 1);

        uintptr_t camSvc = GetCameraServices(localPawn);
        if (camSvc && Schemas::CCSPlayerBase_CameraServices::m_flThirdPersonDistance)
            SafeWrite<float>(camSvc + Schemas::CCSPlayerBase_CameraServices::m_flThirdPersonDistance, tp_max_dist);
    }
    else {
        if (bTPWasActive) {
            if (Schemas::CPlayer_ObserverServices::m_iObserverMode)
                SafeWrite<uint32_t>(obsSvc + Schemas::CPlayer_ObserverServices::m_iObserverMode, 0);
            bTPWasActive = false;
        }
    }
}

void CView::HandleFreeLook(uintptr_t clientBase) {
    if (!cam_free_look || !Offsets::dwViewAngles)
        return;

    bool held = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;

    if (held && !bFreeLookHeld) {
        SafeRead<float>(clientBase + Offsets::dwViewAngles, flStoredPitch);
        SafeRead<float>(clientBase + Offsets::dwViewAngles + 4, flStoredYaw);
        bFreeLookHeld = true;
    }

    if (!held && bFreeLookHeld) {
        SafeWrite<float>(clientBase + Offsets::dwViewAngles, flStoredPitch);
        SafeWrite<float>(clientBase + Offsets::dwViewAngles + 4, flStoredYaw);
        bFreeLookHeld = false;
    }
}