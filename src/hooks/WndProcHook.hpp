#pragma once

#include <windows.h>

namespace Hooks
{
    class WndProcHook
    {
    public:
        static bool Initialize(HWND hWnd);
        static void Shutdown();

        static LRESULT CALLBACK Hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    private:
        static inline WNDPROC s_oWndProc = nullptr;
        static inline HWND s_hWnd = nullptr;
    };
}
