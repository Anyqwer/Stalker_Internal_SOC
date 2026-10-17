#include "Aimbot.hpp"
#include "../core/Config.hpp"
#include "../core/Logger.hpp"
#include "../xray/Offsets.hpp"
#include "../xray/WorldToScreen.hpp"
#include <imgui.h>
#include <cmath>
#include <cstring>
#include <cctype>

namespace Features
{
    void Aimbot::CalculateAngles(const XRay::Fvector& src, const XRay::Fvector& dst, float& pitch, float& yaw)
    {
        XRay::Fvector dir = dst - src;
        float hyp = std::sqrt(dir.x * dir.x + dir.z * dir.z);

        yaw = std::atan2(dir.x, dir.z);
        pitch = -std::atan2(dir.y, hyp);
    }

    static bool IsValidAimbotTarget(uintptr_t objAddr, XRay::CObject* pObj, XRay::CObject* pActor, float& outTargetHeight)
    {
        if (!pObj || pObj == pActor) return false;

        const char* sect = XRay::SafeReadSharedStr(objAddr + 0xAC);
        const char* name = XRay::SafeReadSharedStr(objAddr + 0xA8);
        const char* visual = XRay::SafeReadSharedStr(objAddr + 0xB0);

        char sectLower[128] = { 0 };
        char visualLower[128] = { 0 };
        char nameLower[128] = { 0 };

        for (size_t i = 0; i < sizeof(sectLower) - 1 && sect[i]; ++i)
            sectLower[i] = static_cast<char>(tolower(static_cast<unsigned char>(sect[i])));
        for (size_t i = 0; i < sizeof(visualLower) - 1 && visual[i]; ++i)
            visualLower[i] = static_cast<char>(tolower(static_cast<unsigned char>(visual[i])));
        for (size_t i = 0; i < sizeof(nameLower) - 1 && name[i]; ++i)
            nameLower[i] = static_cast<char>(tolower(static_cast<unsigned char>(name[i])));

        // Skip player
        if (strstr(sectLower, "actor") || strstr(nameLower, "single_player"))
            return false;

        // Check mutant
        bool isMutant = (strstr(visualLower, "monster") || strstr(visualLower, "mutant") ||
                         strncmp(sectLower, "m_", 2) == 0 ||
                         strstr(sectLower, "boar") || strstr(sectLower, "flesh") || strstr(sectLower, "dog") ||
                         strstr(sectLower, "bloodsucker") || strstr(sectLower, "snork") ||
                         strstr(sectLower, "controller") || strstr(sectLower, "gigant"));

        if (isMutant)
        {
            if (!g_Config.aimbot.bTargetMutants) return false;
            outTargetHeight = (g_Config.aimbot.iTargetBone == 0) ? 0.9f : 0.5f;
            return true;
        }

        // Check human (Stalkers, Bandits, Soldiers, etc.)
        bool isHuman = (strstr(visualLower, "actor") || strstr(visualLower, "stalker") || strstr(visualLower, "hero") ||
                        strstr(sectLower, "stalker") || strstr(sectLower, "bandit") || strstr(sectLower, "soldier") ||
                        strstr(sectLower, "military") || strstr(sectLower, "monolith") || strstr(sectLower, "merc") ||
                        strstr(sectLower, "dolg") || strstr(sectLower, "freedom") || strstr(sectLower, "zombied") ||
                        strstr(sectLower, "novice") || strncmp(sectLower, "esc_", 4) == 0);

        if (isHuman)
        {
            if (!g_Config.aimbot.bTargetEnemies) return false;
            outTargetHeight = (g_Config.aimbot.iTargetBone == 0) ? 1.65f : 1.1f;
            return true;
        }

        return false;
    }

    XRay::CObject* Aimbot::FindBestTarget(XRay::Fvector2& outTargetScreenPos)
    {
        XRay::CObject* pActor = XRay::Engine::GetCurrentEntity();

        ImGuiIO& io = ImGui::GetIO();
        XRay::Fvector2 screenCenter(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);

        XRay::CObject* pBestObj = nullptr;
        float bestFovDistance = g_Config.aimbot.fFov;

        auto CheckCandidate = [&](XRay::CObject* pObj)
        {
            if (!pObj || pObj == pActor) return;

            uintptr_t objAddr = reinterpret_cast<uintptr_t>(pObj);

            float posX = *reinterpret_cast<float*>(objAddr + 0x80);
            float posY = *reinterpret_cast<float*>(objAddr + 0x84);
            float posZ = *reinterpret_cast<float*>(objAddr + 0x88);

            if (posX == 0.0f && posY == 0.0f && posZ == 0.0f) return;

            float targetHeight = 1.65f;
            if (!IsValidAimbotTarget(objAddr, pObj, pActor, targetHeight))
                return;

            XRay::Fvector targetWorld(posX, posY + targetHeight, posZ);
            XRay::Fvector2 screenPos;

            if (XRay::Math::WorldToScreen(targetWorld, screenPos))
            {
                float distFromCrosshair = XRay::Math::GetDistance2D(screenCenter, screenPos);
                if (distFromCrosshair < bestFovDistance)
                {
                    bestFovDistance = distFromCrosshair;
                    pBestObj = pObj;
                    outTargetScreenPos = screenPos;
                }
            }
        };

        __try
        {
            XRay::xr_vector<XRay::CObject*>* pActiveObjs = XRay::Engine::GetActiveObjects();
            size_t activeCount = pActiveObjs ? pActiveObjs->size() : 0;

            if (activeCount > 0 && activeCount < 20000)
            {
                for (size_t i = 0; i < activeCount; ++i)
                {
                    CheckCandidate(pActiveObjs->at(i));
                }
            }
            else
            {
                uint32_t totalCount = XRay::Engine::GetTotalObjectCount();
                if (totalCount > 0 && totalCount < 20000)
                {
                    for (uint32_t i = 0; i < totalCount; ++i)
                    {
                        CheckCandidate(XRay::Engine::GetObjectByIndex(i));
                    }
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            return nullptr;
        }

        return pBestObj;
    }

    void Aimbot::Update()
    {
        if (!g_Config.aimbot.bEnabled) return;

        // Check if aiming key (Right Mouse Button) is pressed
        if (!(GetAsyncKeyState(VK_RBUTTON) & 0x8000))
            return;

        // Don't aim while in menu
        if (g_Config.bShowMenu) return;

        XRay::Fvector2 targetScreen;
        XRay::CObject* pTarget = FindBestTarget(targetScreen);

        if (pTarget)
        {
            ImGuiIO& io = ImGui::GetIO();
            float centerX = io.DisplaySize.x * 0.5f;
            float centerY = io.DisplaySize.y * 0.5f;

            float deltaX = targetScreen.x - centerX;
            float deltaY = targetScreen.y - centerY;

            // Apply smoothing
            float smooth = (g_Config.aimbot.fSmoothness > 1.0f) ? g_Config.aimbot.fSmoothness : 1.0f;
            deltaX /= smooth;
            deltaY /= smooth;

            // Send relative mouse movement to guide crosshair to target
            mouse_event(MOUSEEVENTF_MOVE, static_cast<DWORD>(deltaX), static_cast<DWORD>(deltaY), 0, 0);
        }
    }
}
