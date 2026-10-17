#include "WndProcHook.hpp"
#include "../core/Config.hpp"
#include "../core/Globals.hpp"
#include "../core/Logger.hpp"
#include <imgui.h>

// Forward declare ImGui Win32 handler
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Hooks
{
    bool WndProcHook::Initialize(HWND hWnd)
    {
        if (!hWnd)
        {
            Core::Logger::Error("Invalid HWND passed to WndProcHook!");
            return false;
        }

        s_hWnd = hWnd;
        s_oWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(s_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(Hooked_WndProc)));

        if (!s_oWndProc)
        {
            Core::Logger::Error("Failed to hook WndProc via SetWindowLongPtrA!");
            return false;
        }

        Core::Logger::Success("Hooked WndProc for HWND 0x%p (Original: 0x%p)", s_hWnd, s_oWndProc);
        return true;
    }

    void WndProcHook::Shutdown()
    {
        if (s_hWnd && s_oWndProc)
        {
            SetWindowLongPtrA(s_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(s_oWndProc));
            Core::Logger::Info("Restored original WndProc.");
            s_oWndProc = nullptr;
            s_hWnd = nullptr;
        }
    }

    LRESULT CALLBACK WndProcHook::Hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        // Toggle menu on user-defined key (default VK_INSERT)
        if (uMsg == WM_KEYDOWN)
        {
            if (wParam == static_cast<WPARAM>(g_Config.settings.iMenuKey))
            {
                g_Config.bShowMenu = !g_Config.bShowMenu;
                if (g_Config.bShowMenu)
                {
                    ClipCursor(nullptr);
                }
                return 0;
            }
            else if (wParam == static_cast<WPARAM>(g_Config.settings.iUnloadKey))
            {
                Core::Logger::Warning("Unload key pressed! Initiating DLL eject...");
                Core::g_bUnloadRequested.store(true);
                return 0;
            }
        }

        // Pass message to ImGui if initialized
        if (Core::g_bInitialized)
        {
            ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

            // Block input to the game when cheat menu is open
            if (g_Config.bShowMenu)
            {
                switch (uMsg)
                {
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP:
                case WM_MBUTTONDOWN:
                case WM_MBUTTONUP:
                case WM_MOUSEWHEEL:
                case WM_MOUSEMOVE:
                case WM_KEYDOWN:
                case WM_KEYUP:
                case WM_CHAR:
                    return 0; // Completely consume the message
                }
            }
        }

        return CallWindowProcA(s_oWndProc, hWnd, uMsg, wParam, lParam);
    }
}
