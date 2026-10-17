#pragma once

#include <imgui.h>
#include "../xray/XRayTypes.hpp"
#include "../xray/XRaySDK.hpp"

namespace Features
{
    class Visuals
    {
    public:
        // Main render callback called inside ImGui frame
        static void Render();

    private:
        // Drawing helpers
        static void DrawBox2D(float x, float y, float w, float h, ImU32 color, bool bCornerOnly);
        static void DrawHealthBar(float x, float y, float h, float health01);
        static void DrawSnapline(const XRay::Fvector2& screenPos, ImU32 color, int snaplinePos);
        static void DrawFovCircle(float fovRadius, ImU32 color);
        static void DrawCustomCrosshair(ImU32 color);

        // Object processing
        static void ProcessObject(XRay::CObject* pObj, ImDrawList* pDrawList, const XRay::Fvector& localPos);
    };
}
