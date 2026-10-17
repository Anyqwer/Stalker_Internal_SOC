#pragma once

#include "../xray/XRaySDK.hpp"
#include "../xray/XRayTypes.hpp"

namespace Features
{
    class Aimbot
    {
    public:
        // Update aimbot logic every frame
        static void Update();

    private:
        // Find best target meeting FOV and priority criteria
        static XRay::CObject* FindBestTarget(XRay::Fvector2& outTargetScreenPos);

        // Calculate pitch and yaw to target in radians/degrees
        static void CalculateAngles(const XRay::Fvector& src, const XRay::Fvector& dst, float& pitch, float& yaw);
    };
}
