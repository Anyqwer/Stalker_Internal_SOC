#include "DirectInputHook.hpp"
#include "../core/Config.hpp"
#include "../core/Globals.hpp"
#include "../core/Logger.hpp"
#include <MinHook.h>
#include <imgui.h>

namespace Hooks
{
    HRESULT APIENTRY DirectInputHook::Hooked_GetDeviceState(IDirectInputDevice8A* pDevice, DWORD cbData, LPVOID lpvData)
    {
        HRESULT hr = s_oGetDeviceState(pDevice, cbData, lpvData);

        if (SUCCEEDED(hr) && lpvData)
        {
            if (g_Config.bShowMenu)
            {
                // If cheat menu is open, zero out state for the game so camera does not spin and weapons don't fire
                memset(lpvData, 0, cbData);
            }
        }

        return hr;
    }

    HRESULT APIENTRY DirectInputHook::Hooked_GetDeviceData(IDirectInputDevice8A* pDevice, DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags)
    {
        HRESULT hr = s_oGetDeviceData(pDevice, cbObjectData, rgdod, pdwInOut, dwFlags);

        if (SUCCEEDED(hr) && g_Config.bShowMenu && pdwInOut)
        {
            DWORD count = *pdwInOut;

            // Route raw clicks to ImGui if present in buffered data
            if (rgdod && count > 0 && Core::g_bInitialized)
            {
                ImGuiIO& io = ImGui::GetIO();
                for (DWORD i = 0; i < count; ++i)
                {
                    DWORD ofs = rgdod[i].dwOfs;
                    DWORD data = rgdod[i].dwData;

                    // DIMOFS_BUTTON0 = 12 (Left Button), DIMOFS_BUTTON1 = 13 (Right Button), DIMOFS_BUTTON2 = 14 (Middle Button)
                    if (ofs == 12)
                    {
                        io.AddMouseButtonEvent(0, (data & 0x80) != 0);
                    }
                    else if (ofs == 13)
                    {
                        io.AddMouseButtonEvent(1, (data & 0x80) != 0);
                    }
                    else if (ofs == 14)
                    {
                        io.AddMouseButtonEvent(2, (data & 0x80) != 0);
                    }
                    else if (ofs == 8) // DIMOFS_Z: Mouse Wheel
                    {
                        int wheelDelta = static_cast<int>(static_cast<int32_t>(data));
                        float wheelV = 0.0f;
                        if (wheelDelta > 0)
                            wheelV = (wheelDelta >= 120) ? (static_cast<float>(wheelDelta) / 120.0f) : 1.0f;
                        else if (wheelDelta < 0)
                            wheelV = (wheelDelta <= -120) ? (static_cast<float>(wheelDelta) / 120.0f) : -1.0f;

                        io.AddMouseWheelEvent(0.0f, wheelV);
                    }
                }
            }

            // Suppress packets from reaching X-Ray game engine
            *pdwInOut = 0;
        }

        return hr;
    }

    bool DirectInputHook::Initialize()
    {
        if (s_bHooked) return true;

        HMODULE hDInput = GetModuleHandleA("dinput8.dll");
        if (!hDInput)
        {
            hDInput = LoadLibraryA("dinput8.dll");
        }

        if (!hDInput)
        {
            Core::Logger::Warning("dinput8.dll not loaded in process.");
            return false;
        }

        typedef HRESULT(WINAPI* tDirectInput8Create)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
        auto pDirectInput8Create = reinterpret_cast<tDirectInput8Create>(GetProcAddress(hDInput, "DirectInput8Create"));

        if (!pDirectInput8Create)
        {
            Core::Logger::Warning("Could not find DirectInput8Create in dinput8.dll");
            return false;
        }

        IDirectInput8A* pDI = nullptr;
        HRESULT hr = pDirectInput8Create(GetModuleHandleA(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8A, reinterpret_cast<void**>(&pDI), nullptr);
        if (FAILED(hr) || !pDI)
        {
            Core::Logger::Warning("Failed to create temporary IDirectInput8 instance.");
            return false;
        }

        IDirectInputDevice8A* pDummyMouse = nullptr;
        hr = pDI->CreateDevice(GUID_SysMouse, &pDummyMouse, nullptr);
        if (FAILED(hr) || !pDummyMouse)
        {
            pDI->Release();
            Core::Logger::Warning("Failed to create dummy DirectInput mouse device.");
            return false;
        }

        void** vTable = *reinterpret_cast<void***>(pDummyMouse);
        s_pTargetGetDeviceState = vTable[9];
        s_pTargetGetDeviceData = vTable[10];

        Core::Logger::Info("DirectInput8 GetDeviceState Target: 0x%p", s_pTargetGetDeviceState);
        Core::Logger::Info("DirectInput8 GetDeviceData Target:  0x%p", s_pTargetGetDeviceData);

        if (MH_CreateHook(s_pTargetGetDeviceState, &Hooked_GetDeviceState, reinterpret_cast<void**>(&s_oGetDeviceState)) == MH_OK)
        {
            MH_EnableHook(s_pTargetGetDeviceState);
        }

        if (MH_CreateHook(s_pTargetGetDeviceData, &Hooked_GetDeviceData, reinterpret_cast<void**>(&s_oGetDeviceData)) == MH_OK)
        {
            MH_EnableHook(s_pTargetGetDeviceData);
        }

        pDummyMouse->Release();
        pDI->Release();

        s_bHooked = true;
        Core::Logger::Success("DirectInput8 hooks successfully initialized!");
        return true;
    }

    void DirectInputHook::Shutdown()
    {
        if (!s_bHooked) return;

        if (s_pTargetGetDeviceState)
        {
            MH_DisableHook(s_pTargetGetDeviceState);
            s_pTargetGetDeviceState = nullptr;
        }

        if (s_pTargetGetDeviceData)
        {
            MH_DisableHook(s_pTargetGetDeviceData);
            s_pTargetGetDeviceData = nullptr;
        }

        s_bHooked = false;
        Core::Logger::Info("DirectInput8 hooks removed.");
    }
}
