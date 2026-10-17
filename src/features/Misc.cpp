#include "Misc.hpp"
#include "../core/Config.hpp"
#include "../core/Logger.hpp"
#include "../core/Memory.hpp"
#include "../xray/Offsets.hpp"
#include "../xray/XRaySDK.hpp"
#include <vector>

namespace Features
{
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
        static std::vector<uintptr_t> s_ammoAddresses;
        static std::vector<std::vector<uint8_t>> s_originalBytes;
        static bool s_scanned = false;

        if (!s_scanned)
        {
            s_scanned = true;
            HMODULE hXrGame = GetModuleHandleA("xrGame.dll");
            if (hXrGame)
            {
                uint8_t* pBase = reinterpret_cast<uint8_t*>(hXrGame);
                auto dos = reinterpret_cast<IMAGE_DOS_HEADER*>(pBase);
                if (dos->e_magic == IMAGE_DOS_SIGNATURE)
                {
                    auto nt = reinterpret_cast<IMAGE_NT_HEADERS*>(pBase + dos->e_lfanew);
                    size_t imgSize = nt->OptionalHeader.SizeOfImage;

                    // Search for 8B ?? A8 05 00 00 3B ?? 74 13 83 ?? FF in xrGame.dll
                    for (size_t i = 0; i < imgSize - 16; ++i)
                    {
                        if (pBase[i] == 0x8B && pBase[i+2] == 0xA8 && pBase[i+3] == 0x05 &&
                            pBase[i+4] == 0x00 && pBase[i+5] == 0x00 && pBase[i+6] == 0x3B &&
                            pBase[i+8] == 0x74 && pBase[i+9] == 0x13 && pBase[i+10] == 0x83 &&
                            pBase[i+12] == 0xFF)
                        {
                            uintptr_t patchAddr = reinterpret_cast<uintptr_t>(pBase + i + 10);
                            s_ammoAddresses.push_back(patchAddr);

                            std::vector<uint8_t> orig(pBase + i + 10, pBase + i + 13);
                            s_originalBytes.push_back(orig);

                            Core::Logger::Success("Found Weapon Ammo decrement at 0x%p (bytes: %02X %02X %02X)",
                                                  patchAddr, orig[0], orig[1], orig[2]);
                        }
                    }
                }
            }
        }

        if (!s_ammoAddresses.empty())
        {
            if (g_Config.misc.bInfiniteAmmo && !s_bApplied)
            {
                for (uintptr_t addr : s_ammoAddresses)
                {
                    Core::Memory::Nop(addr, 3);
                }
                s_bApplied = true;
                Core::Logger::Info("Infinite Ammo: Enabled (%zu NOP patches applied)", s_ammoAddresses.size());
            }
            else if (!g_Config.misc.bInfiniteAmmo && s_bApplied)
            {
                for (size_t i = 0; i < s_ammoAddresses.size(); ++i)
                {
                    Core::Memory::Patch(s_ammoAddresses[i], s_originalBytes[i].data(), 3);
                }
                s_bApplied = false;
                Core::Logger::Info("Infinite Ammo: Disabled (Original instructions restored)");
            }
        }
    }

    void Misc::HandleNoRecoil()
    {
        static bool s_bApplied = false;
        static uintptr_t s_recoilAddress = 0;
        static uint8_t s_originalBytes[6] = { 0 };
        static uint8_t s_patchedBytes[6] = { 0xE9, 0x29, 0x05, 0x00, 0x00, 0x90 }; // jmp +0x529; nop
        static bool s_scanned = false;

        if (!s_scanned)
        {
            s_scanned = true;
            HMODULE hXrGame = GetModuleHandleA("xrGame.dll");
            if (hXrGame)
            {
                uint8_t* pBase = reinterpret_cast<uint8_t*>(hXrGame);
                auto dos = reinterpret_cast<IMAGE_DOS_HEADER*>(pBase);
                if (dos->e_magic == IMAGE_DOS_SIGNATURE)
                {
                    auto nt = reinterpret_cast<IMAGE_NT_HEADERS*>(pBase + dos->e_lfanew);
                    size_t imgSize = nt->OptionalHeader.SizeOfImage;

                    // Pattern: 83 BB 9C 02 00 00 00 0F 84
                    for (size_t i = 0; i < imgSize - 16; ++i)
                    {
                        if (pBase[i] == 0x83 && pBase[i+1] == 0xBB && pBase[i+2] == 0x9C &&
                            pBase[i+3] == 0x02 && pBase[i+4] == 0x00 && pBase[i+5] == 0x00 &&
                            pBase[i+6] == 0x00 && pBase[i+7] == 0x0F && pBase[i+8] == 0x84)
                        {
                            s_recoilAddress = reinterpret_cast<uintptr_t>(pBase + i + 7);
                            memcpy(s_originalBytes, pBase + i + 7, 6);
                            Core::Logger::Success("Found Camera Recoil Effector branch at 0x%p", s_recoilAddress);
                            break;
                        }
                    }
                }
            }
        }

        if (s_recoilAddress)
        {
            if (g_Config.misc.bNoRecoil && !s_bApplied)
            {
                Core::Memory::Patch(s_recoilAddress, s_patchedBytes, 6);
                s_bApplied = true;
                Core::Logger::Info("No Recoil: Enabled (Camera kick bypassed)");
            }
            else if (!g_Config.misc.bNoRecoil && s_bApplied)
            {
                Core::Memory::Patch(s_recoilAddress, s_originalBytes, 6);
                s_bApplied = false;
                Core::Logger::Info("No Recoil: Disabled (Original restored)");
            }
        }
    }

    void Misc::HandleWeightLimit()
    {
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
                    if (*pMaxWalk != g_Config.misc.fWeightLimit)
                    {
                        *pMaxWalk = g_Config.misc.fWeightLimit;
                        *pMaxWeight = g_Config.misc.fWeightLimit;
                    }
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
            float targetRun = g_Config.misc.bSpeedhack ? (1.5f * g_Config.misc.fSpeedMultiplier) : 1.5f;
            float targetSprint = g_Config.misc.bSpeedhack ? (2.0f * g_Config.misc.fSpeedMultiplier) : 2.0f;

            float* pRunFactor = reinterpret_cast<float*>(actorAddr + 0x5a8);
            float* pSprintFactor = reinterpret_cast<float*>(actorAddr + 0x5c0);

            if (*pRunFactor != targetRun)
            {
                *pRunFactor = targetRun;
                *pSprintFactor = targetSprint;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
        }
    }

    void Misc::HandleBunnyhop()
    {
        if (!g_Config.misc.bBunnyhop || g_Config.bShowMenu) return;

        static bool s_bJumping = false;
        if (GetAsyncKeyState(VK_SPACE) & 0x8000)
        {
            if (s_bJumping)
            {
                keybd_event(VK_SPACE, 0, KEYEVENTF_KEYUP, 0);
                s_bJumping = false;
            }
            else
            {
                keybd_event(VK_SPACE, 0, 0, 0);
                s_bJumping = true;
            }
        }
        else
        {
            if (s_bJumping)
            {
                keybd_event(VK_SPACE, 0, KEYEVENTF_KEYUP, 0);
                s_bJumping = false;
            }
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
                    // Darken the sun lumscale and ambient in R2 deferred lighting
                    ExecuteConsoleCmd("r2_sun_lumscale 0.05");
                    ExecuteConsoleCmd("r2_sun_lumscale_amb 0.25");
                    ExecuteConsoleCmd("r2_sun_lumscale_hemi 0.35");
                }
                else
                {
                    // Restore standard lighting lumscales
                    ExecuteConsoleCmd("r2_sun_lumscale 1.0");
                    ExecuteConsoleCmd("r2_sun_lumscale_amb 1.0");
                    ExecuteConsoleCmd("r2_sun_lumscale_hemi 1.0");
                }
            }

            // 4. Skybox & Ambient memory overrides
            uintptr_t env0 = *reinterpret_cast<uintptr_t*>(pEnv + 0x28C);
            uintptr_t env1 = *reinterpret_cast<uintptr_t*>(pEnv + 0x290);

            if (g_Config.environment.bNightmode)
            {
                float nightSky[3] = {
                    g_Config.environment.colNightmode.x * 0.12f,
                    g_Config.environment.colNightmode.y * 0.16f,
                    g_Config.environment.colNightmode.z * 0.28f
                };
                float nightAmb[3] = {
                    g_Config.environment.colNightmode.x * 0.15f,
                    g_Config.environment.colNightmode.y * 0.18f,
                    g_Config.environment.colNightmode.z * 0.28f
                };
                float nightSun[3] = { 0.04f, 0.05f, 0.08f };

                // In CEnvironment: RenderSky directly reads 0x1A4!
                memcpy(reinterpret_cast<void*>(pEnv + 0x1A4), nightSky, sizeof(nightSky));
                memcpy(reinterpret_cast<void*>(pEnv + 0x194), nightSky, sizeof(nightSky));
                memcpy(reinterpret_cast<void*>(pEnv + 0x1F0), nightAmb, sizeof(nightAmb));
                memcpy(reinterpret_cast<void*>(pEnv + 0x1B4), nightSun, sizeof(nightSun));

                // Also override Current[0] and Current[1] so lerp in OnFrame() maintains night values
                if (env0 && !IsBadReadPtr(reinterpret_cast<void*>(env0), 0x100))
                {
                    memcpy(reinterpret_cast<void*>(env0 + 0x30), nightSky, sizeof(nightSky));
                    memcpy(reinterpret_cast<void*>(env0 + 0x20), nightSky, sizeof(nightSky));
                    memcpy(reinterpret_cast<void*>(env0 + 0x7C), nightAmb, sizeof(nightAmb));
                    memcpy(reinterpret_cast<void*>(env0 + 0x40), nightSun, sizeof(nightSun));
                }
                if (env1 && !IsBadReadPtr(reinterpret_cast<void*>(env1), 0x100))
                {
                    memcpy(reinterpret_cast<void*>(env1 + 0x30), nightSky, sizeof(nightSky));
                    memcpy(reinterpret_cast<void*>(env1 + 0x20), nightSky, sizeof(nightSky));
                    memcpy(reinterpret_cast<void*>(env1 + 0x7C), nightAmb, sizeof(nightAmb));
                    memcpy(reinterpret_cast<void*>(env1 + 0x40), nightSun, sizeof(nightSun));
                }
            }
            else
            {
                if (g_Config.environment.bCustomSkybox)
                {
                    float sky[3] = {
                        g_Config.environment.colSkybox.x,
                        g_Config.environment.colSkybox.y,
                        g_Config.environment.colSkybox.z
                    };
                    memcpy(reinterpret_cast<void*>(pEnv + 0x1A4), sky, sizeof(sky));
                    memcpy(reinterpret_cast<void*>(pEnv + 0x194), sky, sizeof(sky));
                    if (env0 && !IsBadReadPtr(reinterpret_cast<void*>(env0), 0x100))
                    {
                        memcpy(reinterpret_cast<void*>(env0 + 0x30), sky, sizeof(sky));
                        memcpy(reinterpret_cast<void*>(env0 + 0x20), sky, sizeof(sky));
                    }
                    if (env1 && !IsBadReadPtr(reinterpret_cast<void*>(env1), 0x100))
                    {
                        memcpy(reinterpret_cast<void*>(env1 + 0x30), sky, sizeof(sky));
                        memcpy(reinterpret_cast<void*>(env1 + 0x20), sky, sizeof(sky));
                    }
                }

                if (g_Config.environment.bCustomAmbient)
                {
                    float amb[3] = {
                        g_Config.environment.colAmbient.x,
                        g_Config.environment.colAmbient.y,
                        g_Config.environment.colAmbient.z
                    };
                    memcpy(reinterpret_cast<void*>(pEnv + 0x1F0), amb, sizeof(amb));
                    if (env0 && !IsBadReadPtr(reinterpret_cast<void*>(env0), 0x100))
                    {
                        memcpy(reinterpret_cast<void*>(env0 + 0x7C), amb, sizeof(amb));
                    }
                    if (env1 && !IsBadReadPtr(reinterpret_cast<void*>(env1), 0x100))
                    {
                        memcpy(reinterpret_cast<void*>(env1 + 0x7C), amb, sizeof(amb));
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
        HandleGodmode();
        HandleInfiniteAmmo();
        HandleNoRecoil();
        HandleWeightLimit();
        HandleSuperJump();
        HandleSpeedhack();
        HandleBunnyhop();
        HandleEnvironment();
    }
}
