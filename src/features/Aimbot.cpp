#include "Aimbot.hpp"
#include "../core/Config.hpp"
#include "../core/Logger.hpp"
#include "../xray/Offsets.hpp"
#include "../xray/WorldToScreen.hpp"
#include <imgui.h>
#include <cmath>
#include <cstring>
#include <cctype>
#include <algorithm>

namespace Features
{
    void Aimbot::CalculateAngles(const XRay::Fvector& src, const XRay::Fvector& dst, float& pitch, float& yaw)
    {
        XRay::Fvector dir = dst - src;
        float hyp = std::sqrt(dir.x * dir.x + dir.z * dir.z);

        yaw = std::atan2(dir.x, dir.z);
        pitch = -std::atan2(dir.y, hyp);

        // Normalize yaw to [-PI, PI]
        while (yaw > 3.14159265f) yaw -= 6.2831853f;
        while (yaw < -3.14159265f) yaw += 6.2831853f;

        // Clamp pitch to camera limits [-1.48f, 1.48f]
        pitch = std::clamp(pitch, -1.48f, 1.48f);
    }

    static bool IsValidAimbotTarget(uintptr_t objAddr, XRay::CObject* pObj, XRay::CObject* pActor, float& outTargetHeight)
    {
        if (!pObj || pObj == pActor) return false;

        XRay::EntityRelation rel = XRay::Engine::ClassifyRelation(objAddr, pObj, pActor);
        if (rel == XRay::EntityRelation::Unknown) return false;

        bool isRage = g_Config.rage.bRageAimbot;
        bool targetEnemies  = isRage ? g_Config.rage.bTargetEnemies  : g_Config.aimbot.bTargetEnemies;
        bool targetNeutrals = isRage ? g_Config.rage.bTargetNeutrals : g_Config.aimbot.bTargetNeutrals;
        bool targetFriends  = isRage ? g_Config.rage.bTargetFriends  : g_Config.aimbot.bTargetFriends;
        bool targetMutants  = isRage ? g_Config.rage.bTargetMutants  : g_Config.aimbot.bTargetMutants;
        int targetBone      = isRage ? g_Config.rage.iRageBone       : g_Config.aimbot.iTargetBone;

        if (rel == XRay::EntityRelation::Mutant)
        {
            if (!targetMutants) return false;
            outTargetHeight = (targetBone == 0) ? 0.9f : 0.5f;
            return true;
        }
        else if (rel == XRay::EntityRelation::Enemy)
        {
            if (!targetEnemies) return false;
            outTargetHeight = (targetBone == 0) ? 1.65f : 1.1f;
            return true;
        }
        else if (rel == XRay::EntityRelation::Neutral)
        {
            if (!targetNeutrals) return false;
            outTargetHeight = (targetBone == 0) ? 1.65f : 1.1f;
            return true;
        }
        else if (rel == XRay::EntityRelation::Friend)
        {
            if (!targetFriends) return false;
            outTargetHeight = (targetBone == 0) ? 1.65f : 1.1f;
            return true;
        }

        return false;
    }

    XRay::CObject* Aimbot::FindBestTarget(XRay::Fvector& outTargetWorld, XRay::Fvector2& outTargetScreenPos)
    {
        if (!XRay::Engine::IsInGame()) return nullptr;

        XRay::CObject* pActor = XRay::Engine::GetCurrentEntity();
        if (!pActor) return nullptr;

        ImGuiIO& io = ImGui::GetIO();
        XRay::Fvector2 screenCenter(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);

        XRay::CObject* pBestObj = nullptr;
        float bestMetric = 1e6f;
        XRay::Fvector camPos = XRay::Engine::GetCameraPosition();

        bool isRage = g_Config.rage.bRageAimbot;
        bool is360 = isRage && g_Config.rage.b360Fov;

        auto CheckCandidate = [&](XRay::CObject* pObj)
        {
            if (!pObj || pObj == pActor) return;
            if (IsBadReadPtr(pObj, 0x100)) return;

            uintptr_t objAddr = reinterpret_cast<uintptr_t>(pObj);

            float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
            if (!Core::Memory::SafeRead(objAddr + 0x80, posX) ||
                !Core::Memory::SafeRead(objAddr + 0x84, posY) ||
                !Core::Memory::SafeRead(objAddr + 0x88, posZ))
                return;

            if (posX == 0.0f && posY == 0.0f && posZ == 0.0f) return;

            float defaultHeight = 1.65f;
            if (!IsValidAimbotTarget(objAddr, pObj, pActor, defaultHeight))
                return;

            // Check health: ignore dead bodies
            uintptr_t pConditions = 0;
            if (Core::Memory::SafeRead(objAddr + 0x220, pConditions) && pConditions && !IsBadReadPtr(reinterpret_cast<void*>(pConditions), 0x20))
            {
                float hp = 1.0f;
                if (Core::Memory::SafeRead(pConditions + 0x04, hp))
                {
                    if (hp <= 0.0f) return;
                }
            }

            // Extract dynamic bone world position with multi-bone visibility fallback
            XRay::Fvector targetWorld;
            bool bFoundVisibleBone = false;
            int targetBone = isRage ? g_Config.rage.iRageBone : g_Config.aimbot.iTargetBone;
            bool doVisCheck = isRage ? g_Config.rage.bVisCheck : g_Config.aimbot.bVisCheck;

            if (doVisCheck)
            {
                int bonesToCheck[4] = { targetBone, 2, 1, 3 };
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
                    return; // Occluded behind solid static walls
            }
            else
            {
                if (!XRay::Engine::GetBoneWorldPosition(pObj, targetBone, targetWorld))
                {
                    targetWorld = XRay::Fvector(posX, posY + defaultHeight, posZ);
                }
            }

            // Distance calculation
            XRay::Fvector diff = targetWorld - camPos;
            float dist3D = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
            if (dist3D < 0.5f || dist3D > 250.0f) return;

            int priority = isRage ? g_Config.rage.iTargetPriority : g_Config.aimbot.iTargetPriority;

            if (is360)
            {
                // 360 Rage mode: acquire targets in full 360 sphere around the player
                // Priority 1 = Nearest 3D Distance (Always closest enemy), Priority 0 = Crosshair angle
                float metric = dist3D;
                XRay::Fvector2 screenPos;
                bool onScreen = XRay::Math::WorldToScreen(targetWorld, screenPos);

                if (priority == 0 && onScreen)
                {
                    metric = XRay::Math::GetDistance2D(screenCenter, screenPos);
                }

                if (metric < bestMetric)
                {
                    bestMetric = metric;
                    pBestObj = pObj;
                    outTargetWorld = targetWorld;
                    outTargetScreenPos = onScreen ? screenPos : XRay::Fvector2(-100.0f, -100.0f);
                }
            }
            else
            {
                // Legit / Screen FOV check mode
                XRay::Fvector2 screenPos;
                if (XRay::Math::WorldToScreen(targetWorld, screenPos))
                {
                    float distFromCrosshair = XRay::Math::GetDistance2D(screenCenter, screenPos);
                    float maxFov = isRage ? 180.0f : g_Config.aimbot.fFov;
                    if (distFromCrosshair <= maxFov)
                    {
                        float metric = (priority == 1) ? dist3D : distFromCrosshair;
                        if (metric < bestMetric)
                        {
                            bestMetric = metric;
                            pBestObj = pObj;
                            outTargetWorld = targetWorld;
                            outTargetScreenPos = screenPos;
                        }
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

        // Check if level and player are fully valid and alive
        if (!XRay::Engine::IsInGame()) return;

        XRay::CObject* pActor = XRay::Engine::GetCurrentEntity();
        if (!pActor || IsBadReadPtr(pActor, 0x600)) return;

        uintptr_t actorAddr = reinterpret_cast<uintptr_t>(pActor);
        uint32_t curCamIdx = *reinterpret_cast<uint32_t*>(actorAddr + 0x530);
        if (curCamIdx > 2) curCamIdx = 0;

        uintptr_t pCam = *reinterpret_cast<uintptr_t*>(actorAddr + 0x524 + curCamIdx * 4);
        if (!pCam || IsBadReadPtr(reinterpret_cast<void*>(pCam), 0x50))
            return;

        XRay::Fvector camPos = XRay::Engine::GetCameraPosition();
        uintptr_t pFirstCam = *reinterpret_cast<uintptr_t*>(actorAddr + 0x524);
        XRay::Fvector aimSrc = camPos;
        if (pFirstCam && !IsBadReadPtr(reinterpret_cast<void*>(pFirstCam + 0x4C), sizeof(XRay::Fvector)))
        {
            XRay::Fvector eyePos = *reinterpret_cast<XRay::Fvector*>(pFirstCam + 0x4C);
            if (eyePos.MagnitudeSq() > 1.0f)
            {
                aimSrc = eyePos;
            }
        }

        ImGuiIO& io = ImGui::GetIO();
        XRay::Fvector2 screenCenter(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);

        // 1. RAGE AIMBOT & AUTO-SHOOT
        if (g_Config.rage.bRageAimbot)
        {
            XRay::Fvector targetWorld;
            XRay::Fvector2 targetScreen;
            XRay::CObject* pTarget = FindBestTarget(targetWorld, targetScreen);

            if (pTarget)
            {
                float targetPitch = 0.0f, targetYaw = 0.0f;
                CalculateAngles(aimSrc, targetWorld, targetPitch, targetYaw);

                // In CCameraBase: +0x14 is YAW, +0x18 is PITCH!
                *reinterpret_cast<float*>(pCam + 0x14) = targetYaw;
                *reinterpret_cast<float*>(pCam + 0x18) = targetPitch;

                // Synchronize primary eye camera for shooting when in 3rd person
                if (pFirstCam && pFirstCam != pCam && !IsBadReadPtr(reinterpret_cast<void*>(pFirstCam), 0x30))
                {
                    *reinterpret_cast<float*>(pFirstCam + 0x14) = targetYaw;
                    *reinterpret_cast<float*>(pFirstCam + 0x18) = targetPitch;
                }

                if (g_Config.rage.bAutoShoot)
                {
                    static ULONGLONG s_lastRageShot = 0;
                    ULONGLONG now = GetTickCount64();
                    float delay = g_Config.rage.bRapidFire ? g_Config.rage.fRapidFireDelay : 30.0f;
                    if (now - s_lastRageShot >= static_cast<ULONGLONG>(delay))
                    {
                        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                        s_lastRageShot = now;
                    }
                }
            }
        }
        // 2. LEGIT AIMBOT
        else if (g_Config.aimbot.bEnabled)
        {
            int aimKey = g_Config.aimbot.bindKey.vk_code != 0 ? g_Config.aimbot.bindKey.vk_code : VK_RBUTTON;
            if (GetAsyncKeyState(aimKey) & 0x8000)
            {
                XRay::Fvector targetWorld;
                XRay::Fvector2 targetScreen;
                XRay::CObject* pTarget = FindBestTarget(targetWorld, targetScreen);

                if (pTarget)
                {
                    float targetPitch = 0.0f, targetYaw = 0.0f;
                    CalculateAngles(aimSrc, targetWorld, targetPitch, targetYaw);

                    // In CCameraBase: +0x14 is YAW, +0x18 is PITCH!
                    float curYaw   = *reinterpret_cast<float*>(pCam + 0x14);
                    float curPitch = *reinterpret_cast<float*>(pCam + 0x18);

                    float deltaYaw = targetYaw - curYaw;
                    while (deltaYaw > 3.14159265f) deltaYaw -= 6.2831853f;
                    while (deltaYaw < -3.14159265f) deltaYaw += 6.2831853f;

                    float deltaPitch = targetPitch - curPitch;

                    // Smooth angular interpolation
                    float smooth = (g_Config.aimbot.fSmoothness > 1.0f) ? g_Config.aimbot.fSmoothness : 1.0f;
                    float newYaw = curYaw + deltaYaw / smooth;
                    float newPitch = curPitch + deltaPitch / smooth;

                    newPitch = std::clamp(newPitch, -1.48f, 1.48f);
                    while (newYaw > 3.14159265f) newYaw -= 6.2831853f;
                    while (newYaw < -3.14159265f) newYaw += 6.2831853f;

                    *reinterpret_cast<float*>(pCam + 0x14) = newYaw;
                    *reinterpret_cast<float*>(pCam + 0x18) = newPitch;

                    if (pFirstCam && pFirstCam != pCam && !IsBadReadPtr(reinterpret_cast<void*>(pFirstCam), 0x30))
                    {
                        *reinterpret_cast<float*>(pFirstCam + 0x14) = newYaw;
                        *reinterpret_cast<float*>(pFirstCam + 0x18) = newPitch;
                    }
                }
            }
        }

        // 3. TRIGGERBOT
        if (g_Config.aimbot.bTriggerbot && !g_Config.rage.bRageAimbot)
        {
            bool triggerActive = g_Config.aimbot.bTriggerbotAlways;
            if (!triggerActive && g_Config.aimbot.keyTriggerbot.vk_code != 0)
            {
                triggerActive = (GetAsyncKeyState(g_Config.aimbot.keyTriggerbot.vk_code) & 0x8000) != 0;
            }

            if (triggerActive)
            {
                XRay::Fvector targetWorld;
                XRay::Fvector2 targetScreen;
                XRay::CObject* pTarget = FindBestTarget(targetWorld, targetScreen);

                if (pTarget)
                {
                    float targetPitch = 0.0f, targetYaw = 0.0f;
                    CalculateAngles(aimSrc, targetWorld, targetPitch, targetYaw);

                    // In CCameraBase: +0x14 is YAW, +0x18 is PITCH!
                    float curYaw   = *reinterpret_cast<float*>(pCam + 0x14);
                    float curPitch = *reinterpret_cast<float*>(pCam + 0x18);

                    float deltaYaw = targetYaw - curYaw;
                    while (deltaYaw > 3.14159265f) deltaYaw -= 6.2831853f;
                    while (deltaYaw < -3.14159265f) deltaYaw += 6.2831853f;

                    float deltaPitch = std::abs(targetPitch - curPitch);
                    deltaYaw = std::abs(deltaYaw);

                    // Angular alignment cone (approx 2.5 degrees = 0.045 rad)
                    bool shouldFire = (deltaYaw < 0.045f && deltaPitch < 0.045f);

                    if (!shouldFire && targetScreen.x > 0)
                    {
                        float distScreen = XRay::Math::GetDistance2D(screenCenter, targetScreen);
                        if (distScreen <= 25.0f) shouldFire = true;
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

        // 4. STANDALONE RAPID FIRE
        if (g_Config.rage.bRapidFire && !g_Config.rage.bRageAimbot)
        {
            if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
            {
                static ULONGLONG s_lastRapid = 0;
                ULONGLONG now = GetTickCount64();
                if (now - s_lastRapid >= static_cast<ULONGLONG>(g_Config.rage.fRapidFireDelay))
                {
                    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                    s_lastRapid = now;
                }
            }
        }

        // 5. BLINK (FORWARD TELEPORT)
        if (g_Config.rage.bBlink && g_Config.rage.keyBlink.vk_code != 0)
        {
            if (GetAsyncKeyState(g_Config.rage.keyBlink.vk_code) & 1)
            {
                XRay::Fvector actorCamPos = XRay::Engine::GetCameraPosition();
                XRay::Fvector camDir = XRay::Engine::GetCameraDirection();
                XRay::Fvector newPos = actorCamPos + camDir * g_Config.rage.fBlinkDist;
                XRay::Engine::TeleportActor(newPos);
            }
        }

        // 6. BACKSTAB (TELEPORT BEHIND TARGET)
        if (g_Config.rage.bBackstab && g_Config.rage.keyBackstab.vk_code != 0)
        {
            if (GetAsyncKeyState(g_Config.rage.keyBackstab.vk_code) & 1)
            {
                XRay::Fvector targetWorld;
                XRay::Fvector2 dummy;
                XRay::CObject* pTarget = FindBestTarget(targetWorld, dummy);
                if (pTarget)
                {
                    uintptr_t tAddr = reinterpret_cast<uintptr_t>(pTarget);
                    float tx = *reinterpret_cast<float*>(tAddr + 0x80);
                    float ty = *reinterpret_cast<float*>(tAddr + 0x84);
                    float tz = *reinterpret_cast<float*>(tAddr + 0x88);

                    float fwdX = *reinterpret_cast<float*>(tAddr + 0x60);
                    float fwdZ = *reinterpret_cast<float*>(tAddr + 0x68);
                    float len = std::sqrt(fwdX * fwdX + fwdZ * fwdZ);
                    if (len > 0.01f) { fwdX /= len; fwdZ /= len; } else { fwdX = 0; fwdZ = 1; }

                    XRay::Fvector behindPos(tx - fwdX * 2.0f, ty, tz - fwdZ * 2.0f);
                    XRay::Engine::TeleportActor(behindPos);
                }
            }
        }
    }
}
