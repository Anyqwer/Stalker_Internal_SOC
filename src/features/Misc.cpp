#include "Misc.hpp"
#include "../core/Config.hpp"
#include "../core/Logger.hpp"
#include "../core/Memory.hpp"
#include "../xray/Offsets.hpp"
#include "../xray/XRaySDK.hpp"
#include <vector>

namespace Features
{
    void Misc::SetCameraMode(int camIndex)
    {
        if (camIndex < 0 || camIndex > 2) return;
        XRay::CObject* pActor = XRay::Engine::GetCurrentEntity();
        if (!pActor || IsBadReadPtr(pActor, 0x600)) return;

        uintptr_t actorAddr = reinterpret_cast<uintptr_t>(pActor);
        uint32_t curCam = *reinterpret_cast<uint32_t*>(actorAddr + 0x530);
        if (curCam == static_cast<uint32_t>(camIndex)) return;

        static uintptr_t s_fnCamSet = 0;
        static bool s_bScanned = false;

        if (!s_bScanned)
        {
            s_bScanned = true;
            HMODULE hXrGame = GetModuleHandleA("xrGame.dll");
            if (hXrGame)
            {
                uint8_t* pBase = reinterpret_cast<uint8_t*>(hXrGame);
                auto dos = reinterpret_cast<IMAGE_DOS_HEADER*>(pBase);
                if (dos->e_magic == IMAGE_DOS_SIGNATURE)
                {
                    auto nt = reinterpret_cast<IMAGE_NT_HEADERS*>(pBase + dos->e_lfanew);
                    size_t imgSize = nt->OptionalHeader.SizeOfImage;

                    const uint8_t pat[] = { 0x8B, 0x86, 0x30, 0x05, 0x00, 0x00, 0x57, 0x8B, 0xBC, 0x86, 0x24, 0x05, 0x00, 0x00 };
                    for (size_t i = 0; i < imgSize - sizeof(pat); ++i)
                    {
                        if (memcmp(pBase + i, pat, sizeof(pat)) == 0)
                        {
                            s_fnCamSet = reinterpret_cast<uintptr_t>(pBase + i);
                            Core::Logger::Success("Located CActor::cam_Set at 0x%p", s_fnCamSet);
                            break;
                        }
                    }
                }

                if (!s_fnCamSet)
                {
                    s_fnCamSet = reinterpret_cast<uintptr_t>(hXrGame) + 0x1DAFB0;
                }
            }
        }

        if (s_fnCamSet)
        {
            __try
            {
                __asm
                {
                    mov esi, actorAddr
                    mov ecx, camIndex
                    call s_fnCamSet
                }
                Core::Logger::Info("Camera switched to mode %d", camIndex);
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                *reinterpret_cast<uint32_t*>(actorAddr + 0x530) = camIndex;
            }
        }
        else
        {
            *reinterpret_cast<uint32_t*>(actorAddr + 0x530) = camIndex;
        }
    }

    int Misc::GetCurrentCameraMode()
    {
        XRay::CObject* pActor = XRay::Engine::GetCurrentEntity();
        if (!pActor || IsBadReadPtr(pActor, 0x600)) return 0;
        uintptr_t actorAddr = reinterpret_cast<uintptr_t>(pActor);
        uint32_t cur = *reinterpret_cast<uint32_t*>(actorAddr + 0x530);
        return (cur <= 2) ? static_cast<int>(cur) : 0;
    }

    void Misc::HandleThirdPerson()
    {
        // Auto-bind console commands once so F1, F2, F3 work out of the box in-game
        static bool s_bConsoleBound = false;
        if (!s_bConsoleBound)
        {
            s_bConsoleBound = true;
            auto Exec = [](const char* cmd) {
                HMODULE h3DA = GetModuleHandleA("XR_3DA.exe");
                if (!h3DA) return;
                void** ppConsole = reinterpret_cast<void**>(GetProcAddress(h3DA, "?Console@@3PAVCConsole@@A"));
                if (!ppConsole) ppConsole = reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(h3DA) + 0x10BBBC);
                typedef void (__thiscall* tExecute)(void* thisPtr, const char* cmd);
                static tExecute fnExecute = nullptr;
                if (!fnExecute) {
                    fnExecute = reinterpret_cast<tExecute>(GetProcAddress(h3DA, "?Execute@CConsole@@QAEXPBD@Z"));
                    if (!fnExecute) fnExecute = reinterpret_cast<tExecute>(reinterpret_cast<uintptr_t>(h3DA) + 0xBA030);
                }
                if (ppConsole && *ppConsole && fnExecute) fnExecute(*ppConsole, cmd);
            };
            Exec("bind cam_1 kF1");
            Exec("bind cam_2 kF2");
            Exec("bind cam_3 kF3");
        }

        if (g_Config.bShowMenu) return;

        // Hotkey toggle (e.g. F5)
        int tpKey = g_Config.misc.keyThirdPerson.vk_code != 0 ? g_Config.misc.keyThirdPerson.vk_code : VK_F5;
        if (GetAsyncKeyState(tpKey) & 1)
        {
            int cur = GetCurrentCameraMode();
            int next = (cur == 0) ? (g_Config.misc.iThirdPersonMode > 0 ? g_Config.misc.iThirdPersonMode : 1) : 0;
            SetCameraMode(next);
            g_Config.misc.bThirdPerson = (next != 0);
            Core::Logger::Info("Third-Person View hotkey pressed: %s", (next != 0) ? "ON" : "OFF");
        }
    }

    void Misc::HandleTrainerHotkeys()
    {
        if (g_Config.bShowMenu) return;

        // Godmode hotkey toggle
        if (g_Config.settings.keyGodmode.vk_code != 0 && (GetAsyncKeyState(g_Config.settings.keyGodmode.vk_code) & 1))
        {
            g_Config.misc.bGodmode = !g_Config.misc.bGodmode;
            Core::Logger::Info("Godmode toggled: %s", g_Config.misc.bGodmode ? "ON" : "OFF");
        }

        // Speedhack hotkey toggle
        if (g_Config.settings.keySpeedhack.vk_code != 0 && (GetAsyncKeyState(g_Config.settings.keySpeedhack.vk_code) & 1))
        {
            g_Config.misc.bSpeedhack = !g_Config.misc.bSpeedhack;
            Core::Logger::Info("Speedhack toggled: %s", g_Config.misc.bSpeedhack ? "ON" : "OFF");
        }

        // Super Jump hotkey toggle
        if (g_Config.settings.keySuperJump.vk_code != 0 && (GetAsyncKeyState(g_Config.settings.keySuperJump.vk_code) & 1))
        {
            g_Config.misc.bSuperJump = !g_Config.misc.bSuperJump;
            Core::Logger::Info("Super Jump toggled: %s", g_Config.misc.bSuperJump ? "ON" : "OFF");
        }
    }

    void Misc::HandleGodmode()
    {
        if (!g_Config.misc.bGodmode) return;

        XRay::CObject* pActor = XRay::Engine::GetCurrentEntity();
        if (!pActor) return;

        __try
        {
            uintptr_t actorAddr = reinterpret_cast<uintptr_t>(pActor);

            // In X-Ray CEntityAlive:
            // conditions pointer (CActorCondition*) is at offset 0x220
            uintptr_t pConditions = *reinterpret_cast<uintptr_t*>(actorAddr + 0x220);
            if (pConditions && !IsBadReadPtr(reinterpret_cast<void*>(pConditions), 0x160))
            {
                // CEntityCondition / CActorCondition exact reversed offsets:
                *reinterpret_cast<float*>(pConditions + 0x04) = 1.0f; // Health (100%)
                *reinterpret_cast<float*>(pConditions + 0x08) = 1.0f; // Max Health (100%)
                *reinterpret_cast<float*>(pConditions + 0x58) = 1.0f; // Power / Stamina (100%)
                *reinterpret_cast<float*>(pConditions + 0x5C) = 0.0f; // Radiation (0% - fully cleared)
                *reinterpret_cast<float*>(pConditions + 0x60) = 1.0f; // Psy Health (100%)
                *reinterpret_cast<float*>(pConditions + 0x78) = 0.0f; // Health delta/loss
                *reinterpret_cast<float*>(pConditions + 0x7C) = 0.0f; // Health restore delta
                *reinterpret_cast<float*>(pConditions + 0x80) = 0.0f; // Bleeding (0% - stopped)
                *reinterpret_cast<float*>(pConditions + 0x84) = 0.0f; // Radiation loss delta
                *reinterpret_cast<float*>(pConditions + 0xF8) = 0.0f; // Alcohol (0%)
                *reinterpret_cast<float*>(pConditions + 0x100) = 1.0f; // Satiety (100% full - no hunger!)
                *reinterpret_cast<uint8_t*>(pConditions + 0x13C) = 0; // Hungry flag cleared
                *reinterpret_cast<uint8_t*>(pConditions + 0x13D) = 0; // Limping flag cleared
                *reinterpret_cast<uint8_t*>(pConditions + 0x13E) = 0; // Bleeding flag cleared
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
        }
    }

    void Misc::HandleInfiniteAmmo()
    {
        static bool s_bApplied = false;
        static uintptr_t s_cartridgePopAddr = 0;
        static uintptr_t s_ammoElapsedAddr = 0;
        static uint8_t s_origPop[7] = { 0 };
        static uint8_t s_origElapsed[7] = { 0 };
        static bool s_init = false;

        if (!s_init)
        {
            s_init = true;
            HMODULE hXrGame = GetModuleHandleA("xrGame.dll");
            if (hXrGame)
            {
                uintptr_t base = reinterpret_cast<uintptr_t>(hXrGame);
                s_cartridgePopAddr = base + 0x21F1CF; // add dword ptr [edi + 0x5C0], -0x38 (pop cartridge)
                s_ammoElapsedAddr  = base + 0x21F1D6; // add dword ptr [edi + 0x57C], -1     (decrement ammo count)

                memcpy(s_origPop, reinterpret_cast<void*>(s_cartridgePopAddr), 7);
                memcpy(s_origElapsed, reinterpret_cast<void*>(s_ammoElapsedAddr), 7);
            }
        }

        if (s_cartridgePopAddr && s_ammoElapsedAddr)
        {
            if (g_Config.misc.bInfiniteAmmo && !s_bApplied)
            {
                Core::Memory::Nop(s_cartridgePopAddr, 7);
                Core::Memory::Nop(s_ammoElapsedAddr, 7);
                s_bApplied = true;
                Core::Logger::Success("Infinite Ammo: Enabled (NOP applied to xrGame.dll at 0x%p & 0x%p)",
                                      s_cartridgePopAddr, s_ammoElapsedAddr);
            }
            else if (!g_Config.misc.bInfiniteAmmo && s_bApplied)
            {
                Core::Memory::Patch(s_cartridgePopAddr, s_origPop, 7);
                Core::Memory::Patch(s_ammoElapsedAddr, s_origElapsed, 7);
                s_bApplied = false;
                Core::Logger::Info("Infinite Ammo: Disabled (Restored original bytes)");
            }
        }

        // Active weapon refill to maximum magazine capacity each frame
        if (g_Config.misc.bInfiniteAmmo)
        {
            XRay::CObject* pActor = XRay::Engine::GetCurrentEntity();
            if (pActor)
            {
                __try
                {
                    uintptr_t actorAddr = reinterpret_cast<uintptr_t>(pActor);
                    uintptr_t pInv = *reinterpret_cast<uintptr_t*>(actorAddr + 0x298);
                    if (pInv && !IsBadReadPtr(reinterpret_cast<void*>(pInv), 0x80))
                    {
                        uintptr_t pItem = *reinterpret_cast<uintptr_t*>(pInv + 0x4C); // m_pActiveItem
                        if (!pItem) pItem = *reinterpret_cast<uintptr_t*>(pInv + 0x48);
                        if (pItem && !IsBadReadPtr(reinterpret_cast<void*>(pItem), 0x600))
                        {
                            int magSize = *reinterpret_cast<int*>(pItem + 0x580);
                            if (magSize > 0 && magSize < 1000)
                            {
                                *reinterpret_cast<int*>(pItem + 0x57C) = magSize;
                            }
                        }
                    }
                }
                __except (EXCEPTION_EXECUTE_HANDLER)
                {
                }
            }
        }
    }

    void Misc::HandleNoRecoil()
    {
        static bool s_bApplied = false;
        static uintptr_t s_recoilAddr1 = 0;
        static uintptr_t s_recoilAddr2 = 0;
        static uint8_t s_origBytes1[17] = { 0 };
        static uint8_t s_origBytes2[17] = { 0 };
        // Verified stub: mov eax, [esp+4]; xor edx, edx; mov [eax], edx; mov [eax+4], edx; mov [eax+8], edx; ret 4
        static const uint8_t s_patchBytes[17] = {
            0x8B, 0x44, 0x24, 0x04,
            0x31, 0xD2,
            0x89, 0x10,
            0x89, 0x50, 0x04,
            0x89, 0x50, 0x08,
            0xC2, 0x04, 0x00
        };
        static bool s_init = false;

        if (!s_init)
        {
            s_init = true;
            HMODULE hXrGame = GetModuleHandleA("xrGame.dll");
            if (hXrGame)
            {
                uintptr_t base = reinterpret_cast<uintptr_t>(hXrGame);
                s_recoilAddr1 = base + 0x19F4A0; // CWeaponShotEffector::GetDeltaAngle (shot 1)
                s_recoilAddr2 = base + 0x19F4D0; // CWeaponShotEffector::GetDeltaAngle (shot 2)

                memcpy(s_origBytes1, reinterpret_cast<void*>(s_recoilAddr1), 17);
                memcpy(s_origBytes2, reinterpret_cast<void*>(s_recoilAddr2), 17);
            }
        }

        if (s_recoilAddr1 && s_recoilAddr2)
        {
            if (g_Config.misc.bNoRecoil && !s_bApplied)
            {
                Core::Memory::Patch(s_recoilAddr1, s_patchBytes, 17);
                Core::Memory::Patch(s_recoilAddr2, s_patchBytes, 17);
                s_bApplied = true;
                Core::Logger::Success("No Recoil: Enabled (Zero delta angle patch applied at 0x%p)", s_recoilAddr1);
            }
            else if (!g_Config.misc.bNoRecoil && s_bApplied)
            {
                Core::Memory::Patch(s_recoilAddr1, s_origBytes1, 17);
                Core::Memory::Patch(s_recoilAddr2, s_origBytes2, 17);
                s_bApplied = false;
                Core::Logger::Info("No Recoil: Disabled (Restored original bytes)");
            }
        }
    }

    void Misc::HandleWeightLimit()
    {
        static bool s_bApplied = false;
        static uintptr_t s_weightFnAddr = 0;
        static uint8_t s_origBytes[12] = { 0 };
        // Clean function entry bypass: and word ptr [ecx + 0xf0], 0xff7f; xor al, al; ret
        static const uint8_t s_patchBytes[12] = {
            0x66, 0x81, 0xA1, 0xF0, 0x00, 0x00, 0x00, 0x7F, 0xFF,
            0x30, 0xC0,
            0xC3
        };
        static bool s_init = false;

        if (!s_init)
        {
            s_init = true;
            HMODULE hXrGame = GetModuleHandleA("xrGame.dll");
            if (hXrGame)
            {
                // CActorCondition::IsCantWalkWeight function entry
                s_weightFnAddr = reinterpret_cast<uintptr_t>(hXrGame) + 0x1DD740;
                memcpy(s_origBytes, reinterpret_cast<void*>(s_weightFnAddr), 12);
            }
        }

        if (s_weightFnAddr)
        {
            if (g_Config.misc.bInfiniteWeight && !s_bApplied)
            {
                Core::Memory::Patch(s_weightFnAddr, s_patchBytes, 12);
                s_bApplied = true;
                Core::Logger::Success("Weight Limit: Bypassed IsCantWalkWeight at 0x%p", s_weightFnAddr);
            }
            else if (!g_Config.misc.bInfiniteWeight && s_bApplied)
            {
                Core::Memory::Patch(s_weightFnAddr, s_origBytes, 12);
                s_bApplied = false;
                Core::Logger::Info("Weight Limit: Restored IsCantWalkWeight");
            }
        }

        XRay::CObject* pActor = XRay::Engine::GetCurrentEntity();
        if (!pActor) return;

        __try
        {
            uintptr_t actorAddr = reinterpret_cast<uintptr_t>(pActor);
            uintptr_t pConditions = *reinterpret_cast<uintptr_t*>(actorAddr + 0x220);
            if (pConditions && !IsBadReadPtr(reinterpret_cast<void*>(pConditions), 0x160))
            {
                float* pMaxWalk = reinterpret_cast<float*>(pConditions + 0x138);
                float* pMaxWeight = reinterpret_cast<float*>(pConditions + 0x13C);

                if (g_Config.misc.bInfiniteWeight)
                {
                    *pMaxWalk = g_Config.misc.fWeightLimit;
                    *pMaxWeight = g_Config.misc.fWeightLimit;
                }
                else
                {
                    if (*pMaxWalk > 100.0f)
                    {
                        *pMaxWalk = 60.0f;
                        *pMaxWeight = 50.0f;
                    }
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
        }
    }

    void Misc::HandleSuperJump()
    {
        XRay::CObject* pActor = XRay::Engine::GetCurrentEntity();
        if (!pActor) return;

        __try
        {
            uintptr_t actorAddr = reinterpret_cast<uintptr_t>(pActor);
            float targetJump = g_Config.misc.bSuperJump ? g_Config.misc.fJumpSpeed : 6.0f;
            float* pJumpSpeed = reinterpret_cast<float*>(actorAddr + 0x5a4);

            if (*pJumpSpeed != targetJump)
            {
                *pJumpSpeed = targetJump;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
        }
    }

    void Misc::HandleSpeedhack()
    {
        XRay::CObject* pActor = XRay::Engine::GetCurrentEntity();
        if (!pActor) return;

        __try
        {
            uintptr_t actorAddr = reinterpret_cast<uintptr_t>(pActor);
            float mult = g_Config.misc.bSpeedhack ? g_Config.misc.fSpeedMultiplier : 1.0f;

            float targetWalkAccel = 12.0f * mult;
            float targetRun = 2.0f * mult;
            float targetSprint = 2.6f * mult;

            float* pWalkAccel = reinterpret_cast<float*>(actorAddr + 0x5a0);
            float* pRunFactor = reinterpret_cast<float*>(actorAddr + 0x5a8);
            float* pSprintFactor = reinterpret_cast<float*>(actorAddr + 0x5bc);

            if (*pWalkAccel != targetWalkAccel) *pWalkAccel = targetWalkAccel;
            if (*pRunFactor != targetRun) *pRunFactor = targetRun;
            if (*pSprintFactor != targetSprint) *pSprintFactor = targetSprint;
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
        }
    }

    void Misc::HandleEnvironment()
    {
        uintptr_t pEnv = XRay::Engine::GetEnvironment();
        if (!pEnv) return;

        auto ExecuteConsoleCmd = [](const char* cmd)
        {
            HMODULE h3DA = GetModuleHandleA("XR_3DA.exe");
            if (!h3DA) return;

            void** ppConsole = reinterpret_cast<void**>(GetProcAddress(h3DA, "?Console@@3PAVCConsole@@A"));
            if (!ppConsole)
                ppConsole = reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(h3DA) + 0x10BBBC);

            typedef void (__thiscall* tExecute)(void* thisPtr, const char* cmd);
            static tExecute fnExecute = nullptr;
            if (!fnExecute)
            {
                fnExecute = reinterpret_cast<tExecute>(GetProcAddress(h3DA, "?Execute@CConsole@@QAEXPBD@Z"));
                if (!fnExecute)
                    fnExecute = reinterpret_cast<tExecute>(reinterpret_cast<uintptr_t>(h3DA) + 0xBA030);
            }

            if (ppConsole && *ppConsole && fnExecute)
            {
                fnExecute(*ppConsole, cmd);
            }
        };

        __try
        {
            // 1. Time of day override
            if (g_Config.environment.bTimeOverride)
            {
                float timeSec = g_Config.environment.fTimeOfDay * 3600.0f;
                *reinterpret_cast<float*>(pEnv + 0x168) = timeSec;
                XRay::Engine::SetGameTime(timeSec, 1.0f);
            }

            // 2. Weather preset changer via Console->Execute
            static int s_lastWeather = -1;
            if (s_lastWeather != g_Config.environment.iSelectedWeather)
            {
                s_lastWeather = g_Config.environment.iSelectedWeather;
                const char* weatherPresets[] = { "", "default", "map", "rain", "radar", "yantar" };
                if (s_lastWeather > 0 && s_lastWeather < 6)
                {
                    char cmdBuf[128];
                    snprintf(cmdBuf, sizeof(cmdBuf), "sv_setweather %s", weatherPresets[s_lastWeather]);
                    ExecuteConsoleCmd(cmdBuf);
                }
            }

            // 3. Nightmode lighting control in R2 deferred lighting
            static bool s_lastNightmode = false;
            if (s_lastNightmode != g_Config.environment.bNightmode)
            {
                s_lastNightmode = g_Config.environment.bNightmode;
                if (s_lastNightmode)
                {
                    ExecuteConsoleCmd("r2_sun_lumscale 0.05");
                    ExecuteConsoleCmd("r2_sun_lumscale_amb 0.25");
                    ExecuteConsoleCmd("r2_sun_lumscale_hemi 0.35");
                }
                else
                {
                    ExecuteConsoleCmd("r2_sun_lumscale 1.0");
                    ExecuteConsoleCmd("r2_sun_lumscale_amb 1.0");
                    ExecuteConsoleCmd("r2_sun_lumscale_hemi 1.0");
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
        }
    }

    void Misc::HandleFovChanger()
    {
        if (!XRay::Engine::IsInGame()) return;

        __try
        {
            XRay::CRenderDevice* pDev = XRay::Engine::GetDevice();
            if (pDev && !IsBadReadPtr(pDev, 0x120))
            {
                uintptr_t devAddr = reinterpret_cast<uintptr_t>(pDev);
                float targetFov = g_Config.fov.bEnabled ? g_Config.fov.fFov : 67.5f;
                *reinterpret_cast<float*>(devAddr + 0x110) = targetFov;
            }

            XRay::CObject* pActor = XRay::Engine::GetCurrentEntity();
            if (pActor && !IsBadReadPtr(pActor, 0x540))
            {
                uintptr_t actorAddr = reinterpret_cast<uintptr_t>(pActor);
                float targetRad = (g_Config.fov.bEnabled ? g_Config.fov.fFov : 67.5f) * (3.14159265f / 180.0f);

                for (int i = 0; i < 3; ++i)
                {
                    uintptr_t pCam = *reinterpret_cast<uintptr_t*>(actorAddr + 0x524 + i * 4);
                    if (pCam && !IsBadReadPtr(reinterpret_cast<void*>(pCam), 0x30))
                    {
                        *reinterpret_cast<float*>(pCam + 0x20) = targetRad;
                    }
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
        }
    }

    void Misc::Update()
    {
        HandleTrainerHotkeys();

        if (!XRay::Engine::IsInGame()) return;

        HandleThirdPerson();
        HandleGodmode();
        HandleInfiniteAmmo();
        HandleNoRecoil();
        HandleWeightLimit();
        HandleSuperJump();
        HandleSpeedhack();
        HandleEnvironment();
        HandleFovChanger();
    }
}
