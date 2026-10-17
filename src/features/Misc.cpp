#include "Misc.hpp"
#include "../core/Config.hpp"
#include "../core/Logger.hpp"
#include "../core/Memory.hpp"
#include "../xray/Offsets.hpp"
#include "../xray/XRaySDK.hpp"

namespace Features
{
    void Misc::HandleGodmode()
    {
        if (!g_Config.misc.bGodmode) return;

        XRay::CObject* pActor = XRay::Engine::GetCurrentEntity();
        if (!pActor) return;

        __try
        {
            auto pEntity = reinterpret_cast<XRay::CEntityAlive*>(pActor);
            pEntity->fHealth = 100.0f; // Maximize health
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
        }
    }

    void Misc::HandleInfiniteAmmo()
    {
        static bool s_bApplied = false;
        static uintptr_t s_ammoAddress = 0;
        static uint8_t s_originalBytes[6] = { 0 };

        if (!s_ammoAddress)
        {
            HMODULE hXrGame = GetModuleHandleA("xrGame.dll");
            if (hXrGame)
            {
                // In xrGame.dll (1.0006), CWeapon::OnMagazineEmpty / fire ammo decrement pattern:
                // "FF 4E ? 8B 4E ? 85 C9" (dec [esi+xx])
                s_ammoAddress = Core::Memory::FindPattern(hXrGame, "FF 4E ? 8B 4E ? 85 C9");
                if (s_ammoAddress)
                {
                    memcpy(s_originalBytes, reinterpret_cast<void*>(s_ammoAddress), 3);
                    Core::Logger::Success("Found Weapon Ammo decrement at: 0x%p", s_ammoAddress);
                }
            }
        }

        if (s_ammoAddress)
        {
            if (g_Config.misc.bInfiniteAmmo && !s_bApplied)
            {
                // NOP the 3-byte 'dec [esi+offset]' instruction
                Core::Memory::Nop(s_ammoAddress, 3);
                s_bApplied = true;
                Core::Logger::Info("Infinite Ammo: Enabled (NOP applied)");
            }
            else if (!g_Config.misc.bInfiniteAmmo && s_bApplied)
            {
                // Restore original instruction
                Core::Memory::Patch(s_ammoAddress, s_originalBytes, 3);
                s_bApplied = false;
                Core::Logger::Info("Infinite Ammo: Disabled (Original restored)");
            }
        }
    }

    void Misc::HandleNoRecoil()
    {
        // Placeholder / hook for camera recoil suppression
    }

    void Misc::HandleWeightLimit()
    {
        // Placeholder / hook for dynamic weight expansion
    }

    void Misc::Update()
    {
        HandleGodmode();
        HandleInfiniteAmmo();
        HandleNoRecoil();
        HandleWeightLimit();
    }
}
