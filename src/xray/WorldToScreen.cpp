#include "WorldToScreen.hpp"
#include "Offsets.hpp"
#include <imgui.h>

namespace XRay
{
    bool Math::WorldToScreen(const Fvector& worldPos, Fvector2& screenPos, const Fmatrix& fullTransform, float screenWidth, float screenHeight)
    {
        // Calculate W component for perspective projection (row-major DirectX vector transform)
        float w = worldPos.x * fullTransform._14 +
                  worldPos.y * fullTransform._24 +
                  worldPos.z * fullTransform._34 +
                  fullTransform._44;

        // Behind the near camera clipping plane
        if (w <= 0.001f)
            return false;

        float invW = 1.0f / w;

        // Normalized Device Coordinates (NDC)
        float ndcX = (worldPos.x * fullTransform._11 +
                      worldPos.y * fullTransform._21 +
                      worldPos.z * fullTransform._31 +
                      fullTransform._41) * invW;

        float ndcY = (worldPos.x * fullTransform._12 +
                      worldPos.y * fullTransform._22 +
                      worldPos.z * fullTransform._32 +
                      fullTransform._42) * invW;

        // Map NDC [-1, 1] to screen pixel coordinates
        screenPos.x = (ndcX * 0.5f + 0.5f) * screenWidth;
        screenPos.y = (-ndcY * 0.5f + 0.5f) * screenHeight;

        // Allow generous off-screen range so bone connections, boxes and snaplines don't pop/cut off
        return (screenPos.x >= -3000.0f && screenPos.x <= screenWidth + 3000.0f &&
                screenPos.y >= -3000.0f && screenPos.y <= screenHeight + 3000.0f);
    }

    bool Math::WorldToScreen(const Fvector& worldPos, Fvector2& screenPos)
    {
        ImGuiIO& io = ImGui::GetIO();
        float width = io.DisplaySize.x;
        float height = io.DisplaySize.y;

        if (width <= 10.0f || height <= 10.0f)
            return false;

        Fmatrix fullTransform = Engine::GetFullTransform();
        return WorldToScreen(worldPos, screenPos, fullTransform, width, height);
    }

    float Math::GetDistance2D(const Fvector2& a, const Fvector2& b)
    {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    float Math::GetDistance3D(const Fvector& a, const Fvector& b)
    {
        return a.DistanceTo(b);
    }
}
