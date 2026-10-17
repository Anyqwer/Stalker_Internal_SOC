#pragma once

#include "XRayTypes.hpp"
#include <d3d9.h>

namespace XRay
{
    class Math
    {
    public:
        // Project a 3D world position into 2D screen coordinates
        static bool WorldToScreen(const Fvector& worldPos, Fvector2& screenPos);

        // Project with an explicit view-projection matrix and viewport dimensions
        static bool WorldToScreen(const Fvector& worldPos, Fvector2& screenPos, const Fmatrix& fullTransform, float screenWidth, float screenHeight);

        // Calculate 2D distance between two screen coordinates
        static float GetDistance2D(const Fvector2& a, const Fvector2& b);

        // Calculate 3D distance between two world coordinates
        static float GetDistance3D(const Fvector& a, const Fvector& b);
    };
}
