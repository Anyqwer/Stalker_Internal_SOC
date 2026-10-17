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
        float bestMetric = 1e6f;
        XRay::Fvector camPos = XRay::Engine::GetCameraPosition();

        auto CheckCandidate = [&](XRay::CObject* pObj)
        {
            if (!pObj || pObj == pActor) return;

            uintptr_t objAddr = reinterpret_cast<uintptr_t>(pObj);

            float posX = *reinterpret_cast<float*>(objAddr + 0x80);
            float posY = *reinterpret_cast<float*>(objAddr + 0x84);
            float posZ = *reinterpret_cast<float*>(objAddr + 0x88);

            if (posX == 0.0f && posY == 0.0f && posZ == 0.0f) return;

            float defaultHeight = 1.65f;
            if (!IsValidAimbotTarget(objAddr, pObj, pActor, defaultHeight))
                return;

            // Extract dynamic bone world position with multi-bone visibility fallback
            XRay::Fvector targetWorld;
            bool bFoundVisibleBone = false;

            if (g_Config.aimbot.bVisCheck)
            {
                // Prioritize user-chosen bone, then fallback to chest, neck, pelvis
                int bonesToCheck[4] = { g_Config.aimbot.iTargetBone, 2, 1, 3 };
                for (int b = 0; b < 4; ++b)
                {
                    int boneId = bonesToCheck[b];
                    XRay::Fvector bonePos;
                    if (XRay::Engine::GetBoneWorldPosition(pObj, boneId, bonePos))
                    {
                        if (XRay::Engine::IsVisible(camPos, bonePos, pActor))
                        {
                            targetWorld = bonePos;
                            bFoundVisibleBone = true;
                            break;
                        }
                    }
                }

                if (!bFoundVisibleBone)
                    return; // Completely occluded behind solid static walls
            }
            else
            {
                if (!XRay::Engine::GetBoneWorldPosition(pObj, g_Config.aimbot.iTargetBone, targetWorld))
                {
                    targetWorld = XRay::Fvector(posX, posY + defaultHeight, posZ);
                }
            }

            XRay::Fvector2 screenPos;
            if (XRay::Math::WorldToScreen(targetWorld, screenPos))
            {
                float distFromCrosshair = XRay::Math::GetDistance2D(screenCenter, screenPos);
                if (distFromCrosshair <= g_Config.aimbot.fFov)
                {
                    float metric = distFromCrosshair;
                    if (g_Config.aimbot.iTargetPriority == 1) // Distance priority
                    {
                        XRay::Fvector diff = targetWorld - camPos;
                        metric = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
                    }

                    if (metric < bestMetric)
                    {
                        bestMetric = metric;
                        pBestObj = pObj;
                        outTargetScreenPos = screenPos;
                    }
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
        // Don't aim while in menu
        if (g_Config.bShowMenu) return;

        ImGuiIO& io = ImGui::GetIO();
        float centerX = io.DisplaySize.x * 0.5f;
        float centerY = io.DisplaySize.y * 0.5f;
        XRay::Fvector2 screenCenter(centerX, centerY);

        // 1. AIMBOT
        if (g_Config.aimbot.bEnabled)
        {
            int aimKey = g_Config.aimbot.bindKey.vk_code != 0 ? g_Config.aimbot.bindKey.vk_code : VK_RBUTTON;
            if (GetAsyncKeyState(aimKey) & 0x8000)
            {
                XRay::Fvector2 targetScreen;
                XRay::CObject* pTarget = FindBestTarget(targetScreen);

                if (pTarget)
                {
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

        // 2. TRIGGERBOT
        if (g_Config.aimbot.bTriggerbot)
        {
            bool triggerActive = g_Config.aimbot.bTriggerbotAlways;
            if (!triggerActive && g_Config.aimbot.keyTriggerbot.vk_code != 0)
            {
                triggerActive = (GetAsyncKeyState(g_Config.aimbot.keyTriggerbot.vk_code) & 0x8000) != 0;
            }

            if (triggerActive)
            {
                XRay::Fvector2 targetScreen;
                XRay::CObject* pTarget = FindBestTarget(targetScreen);

                if (pTarget)
                {
                    // Check if target bone is closely aligned with crosshair
                    float distFromCenter = XRay::Math::GetDistance2D(screenCenter, targetScreen);
                    bool shouldFire = (distFromCenter <= 20.0f);

                    // Check other vital bones as well (Head, Chest, Pelvis) in case enemy is partially exposed
                    if (!shouldFire)
                    {
                        int checkBones[3] = { 0, 2, 3 }; // Head, Chest, Pelvis
                        for (int b : checkBones)
                        {
                            XRay::Fvector bWorld;
                            XRay::Fvector2 bScreen;
                            if (XRay::Engine::GetBoneWorldPosition(pTarget, b, bWorld) &&
                                XRay::Math::WorldToScreen(bWorld, bScreen))
                            {
                                if (XRay::Math::GetDistance2D(screenCenter, bScreen) <= 20.0f)
                                {
                                    shouldFire = true;
                                    break;
                                }
                            }
                        }
                    }

                    if (shouldFire)
                    {
                        static ULONGLONG s_lastTriggerTime = 0;
                        ULONGLONG now = GetTickCount64();
                        if (now - s_lastTriggerTime >= static_cast<ULONGLONG>(g_Config.aimbot.fTriggerbotDelay))
                        {
                            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                            s_lastTriggerTime = now;
                        }
                    }
                }
            }
        }
    }
}
