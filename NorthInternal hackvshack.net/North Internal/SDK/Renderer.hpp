#pragma once
#include "Vector.hpp"

namespace Renderer {
    bool WorldToScreen(const Vector3& world, Vector2& screen, const ViewMatrix& vm);
    ViewMatrix GetViewMatrix();
    void SetRenderSize(int w, int h);
    void UpdateScale();
}