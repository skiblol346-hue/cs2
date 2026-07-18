#pragma once
#include <cstdint>
#include <windows.h>

class CView {
public:
    void Run(uintptr_t clientBase, uintptr_t localPawn);

private:
    void ApplyFOV(uintptr_t localPawn);
    void ApplyViewModelFOV(uintptr_t clientBase, uintptr_t localPawn);
    void ApplyViewModelOffset(uintptr_t clientBase, uintptr_t localPawn);
    void ApplyThirdPerson(uintptr_t localPawn);
    void HandleFreeLook(uintptr_t clientBase);

    bool bTPWasActive = false;
    bool bFreeLookHeld = false;
    float flStoredPitch = 0.0f;
    float flStoredYaw = 0.0f;
};

extern CView g_View;