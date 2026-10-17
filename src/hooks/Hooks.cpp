#include "Hooks.hpp"
#include "D3D9Hook.hpp"
#include "WndProcHook.hpp"
#include "DirectInputHook.hpp"
#include "../core/Globals.hpp"
#include "../core/Config.hpp"
#include "../core/Logger.hpp"
#include "../ui/Style.hpp"
#include "../ui/Menu.hpp"
#include "../features/Visuals.hpp"
#include "../features/Aimbot.hpp"
#include "../features/Misc.hpp"
#include <MinHook.h>
#include <imgui.h>
#include <backends/imgui_impl_dx9.h>
#include <backends/imgui_impl_win32.h>

namespace Hooks
{
    static HWND FindRealGameWindow(IDirect3DDevice9* pDevice)
    {
        HWND hWnd = nullptr;

        // 1. Try SwapChain
        IDirect3DSwapChain9* pSwapChain = nullptr;
        if (SUCCEEDED(pDevice->GetSwapChain(0, &pSwapChain)) && pSwapChain)
        {
            D3DPRESENT_PARAMETERS pp = {};
            if (SUCCEEDED(pSwapChain->GetPresentParameters(&pp)) && pp.hDeviceWindow)
            {
                if (IsWindow(pp.hDeviceWindow))
                    hWnd = pp.hDeviceWindow;
            }
            pSwapChain->Release();
        }

        // 2. Try Creation parameters
        if (!hWnd)
        {
            D3DDEVICE_CREATION_PARAMETERS cp = {};
            if (SUCCEEDED(pDevice->GetCreationParameters(&cp)) && cp.hFocusWindow)
            {
                if (IsWindow(cp.hFocusWindow))
                    hWnd = cp.hFocusWindow;
            }
        }

        // 3. Try Window Class "SWND" (X-Ray standard)
        if (!hWnd)
        {
            HWND hSWND = FindWindowA("SWND", nullptr);
            if (hSWND && IsWindow(hSWND))
                hWnd = hSWND;
        }

        // 4. Try EnumWindows for current process
        if (!hWnd)
        {
            struct EnumCtx {
                DWORD pid;
                HWND result;
            } ctx = { GetCurrentProcessId(), nullptr };

            EnumWindows([](HWND h, LPARAM lp) -> BOOL {
                auto pCtx = reinterpret_cast<EnumCtx*>(lp);
                DWORD pid = 0;
                GetWindowThreadProcessId(h, &pid);
                if (pid == pCtx->pid && IsWindowVisible(h))
                {
                    char title[256];
                    GetWindowTextA(h, title, sizeof(title));
                    if (strlen(title) > 0 && !strstr(title, "Debug Console"))
                    {
                        pCtx->result = h;
                        return FALSE;
                    }
                }
                return TRUE;
            }, reinterpret_cast<LPARAM>(&ctx));

            hWnd = ctx.result;
        }

        return hWnd;
    }

    HRESULT APIENTRY Manager::Hooked_EndScene(IDirect3DDevice9* pDevice)
    {
        if (!pDevice)
            return s_oEndScene(pDevice);

        // One-time initialization of ImGui, WndProc, and DirectInput hooks
        if (!Core::g_bInitialized)
        {
            Core::g_pD3DDevice = pDevice;
            Core::g_hGameWindow = FindRealGameWindow(pDevice);

            Core::Logger::Info("Detected S.T.A.L.K.E.R. Game Window HWND: 0x%p", Core::g_hGameWindow);

            if (Core::g_hGameWindow)
            {
                WndProcHook::Initialize(Core::g_hGameWindow);
            }
            else
            {
                Core::Logger::Error("Failed to find valid game window HWND!");
            }

            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.IniFilename = nullptr; // Don't create external imgui.ini

            ImGui_ImplWin32_Init(Core::g_hGameWindow);
            ImGui_ImplDX9_Init(pDevice);

            UI::Style::ApplyStalkerTheme();

            // Hook DirectInput to prevent game camera spinning and shooting when menu is open
            DirectInputHook::Initialize();

            Core::g_bInitialized = true;
            Core::Logger::Success("ImGui Direct3D9 pipeline and DirectInput hook initialized successfully!");
        }

        // Global hardware hotkey check (works regardless of window focus / exclusive mode)
        if (GetAsyncKeyState(g_Config.settings.iMenuKey) & 1)
        {
            g_Config.bShowMenu = !g_Config.bShowMenu;
            Core::Logger::Info("Menu visibility: %s", g_Config.bShowMenu ? "OPEN" : "CLOSED");

            if (g_Config.bShowMenu)
            {
                ClipCursor(nullptr);
            }
        }

        if (GetAsyncKeyState(g_Config.settings.iUnloadKey) & 1)
        {
            Core::Logger::Warning("Eject key pressed! Initiating DLL shutdown...");
            Core::g_bUnloadRequested.store(true);
        }

        // Render cheat frame
        if (Core::g_bInitialized)
        {
            ImGuiIO& io = ImGui::GetIO();

            // CRITICAL: Force ImGui to draw its own software cursor when menu is open!
            io.MouseDrawCursor = g_Config.bShowMenu;

            // Ensure cursor clipping is released so mouse can move across full screen
            if (g_Config.bShowMenu)
            {
                ClipCursor(nullptr);
            }

            ImGui_ImplDX9_NewFrame();
            ImGui_ImplWin32_NewFrame();

            // Ensure mouse clicks and coordinates are fed to ImGui when menu is open
            if (g_Config.bShowMenu)
            {
                POINT mousePt;
                if (GetCursorPos(&mousePt))
                {
                    ScreenToClient(Core::g_hGameWindow, &mousePt);
                    io.AddMousePosEvent(static_cast<float>(mousePt.x), static_cast<float>(mousePt.y));
                }

                io.AddMouseButtonEvent(0, (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0);
                io.AddMouseButtonEvent(1, (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0);
                io.AddMouseButtonEvent(2, (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0);
            }

            ImGui::NewFrame();

            // Run features
            Features::Aimbot::Update();
            Features::Misc::Update();
            Features::Visuals::Render();
            UI::Menu::Render();

            ImGui::EndFrame();
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
        }

        return s_oEndScene(pDevice);
    }

    HRESULT APIENTRY Manager::Hooked_Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
    {
        if (Core::g_bInitialized)
        {
            ImGui_ImplDX9_InvalidateDeviceObjects();
        }

        HRESULT hr = s_oReset(pDevice, pPresentationParameters);

        if (SUCCEEDED(hr) && Core::g_bInitialized)
        {
            ImGui_ImplDX9_CreateDeviceObjects();
        }

        return hr;
    }

    bool Manager::Initialize()
    {
        Core::Logger::Info("Initializing MinHook...");

        if (MH_Initialize() != MH_OK)
        {
            Core::Logger::Error("Failed to initialize MinHook!");
            return false;
        }

        void* vTable[119];
        if (!D3D9Hook::GetVTable(vTable, 119))
        {
            Core::Logger::Error("Failed to retrieve Direct3D9 virtual method table!");
            return false;
        }

        s_pTargetEndScene = vTable[D3D9Hook::VMTIndex::EndScene];
        s_pTargetReset = vTable[D3D9Hook::VMTIndex::Reset];

        Core::Logger::Info("D3D9 EndScene Target: 0x%p", s_pTargetEndScene);
        Core::Logger::Info("D3D9 Reset Target:    0x%p", s_pTargetReset);

        if (MH_CreateHook(s_pTargetEndScene, &Hooked_EndScene, reinterpret_cast<void**>(&s_oEndScene)) != MH_OK)
        {
            Core::Logger::Error("Failed to create hook for EndScene!");
            return false;
        }

        if (MH_CreateHook(s_pTargetReset, &Hooked_Reset, reinterpret_cast<void**>(&s_oReset)) != MH_OK)
        {
            Core::Logger::Error("Failed to create hook for Reset!");
            return false;
        }

        if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
        {
            Core::Logger::Error("Failed to enable MinHook hooks!");
            return false;
        }

        Core::Logger::Success("DirectX 9 EndScene & Reset hooks successfully enabled!");
        return true;
    }

    void Manager::Shutdown()
    {
        Core::Logger::Info("Shutting down hooks and unlinking ImGui...");

        // Disable DirectInput hooks
        DirectInputHook::Shutdown();

        // Disable MinHook hooks
        MH_DisableHook(MH_ALL_HOOKS);
        MH_Uninitialize();

        // Restore window procedure
        WndProcHook::Shutdown();

        // Cleanup ImGui
        if (Core::g_bInitialized)
        {
            ImGui_ImplDX9_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
            Core::g_bInitialized = false;
        }

        Core::Logger::Success("Hooks and rendering cleanly terminated.");
    }
}
