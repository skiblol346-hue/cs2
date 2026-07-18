#include "Renderer.hpp"
#include "Offsets.hpp"
#include "../Dependencies/ImGui/imgui.h"
#include <windows.h>

static bool SafeReadVM(uintptr_t addr, ViewMatrix& out) {
    __try {
        if (addr < 0x10000 || addr > 0x7FFFFFFEFFFF) return false;
        out = *(ViewMatrix*)addr;
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool Renderer::WorldToScreen(const Vector3& world, Vector2& screen, const ViewMatrix& vm) {
    float w = vm.matrix[3][0] * world.x + vm.matrix[3][1] * world.y + vm.matrix[3][2] * world.z + vm.matrix[3][3];
    if (w < 0.1f) return false;

    float x = vm.matrix[0][0] * world.x + vm.matrix[0][1] * world.y + vm.matrix[0][2] * world.z + vm.matrix[0][3];
    float y = vm.matrix[1][0] * world.x + vm.matrix[1][1] * world.y + vm.matrix[1][2] * world.z + vm.matrix[1][3];

    ImVec2 ds = ImGui::GetIO().DisplaySize;
    if (ds.x <= 0 || ds.y <= 0) return false;

    screen.x = (ds.x * 0.5f) * (1.0f + x / w);
    screen.y = (ds.y * 0.5f) * (1.0f - y / w);

    return true;
}

ViewMatrix Renderer::GetViewMatrix() {
    uintptr_t clientBase = (uintptr_t)GetModuleHandleA("client.dll");
    if (!clientBase || Offsets::dwViewMatrix == 0) return ViewMatrix();

    ViewMatrix vm = {};
    SafeReadVM(clientBase + Offsets::dwViewMatrix, vm);
    return vm;
}