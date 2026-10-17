#pragma once

#include <windows.h>
#include <d3d9.h>
#include <atomic>
#include <string>

namespace Core
{
    // Module handle of our injected DLL
    extern HMODULE g_hModule;

    // Handle to the S.T.A.L.K.E.R. game window
    extern HWND g_hGameWindow;

    // Direct3D 9 Device pointer acquired from hook
    extern IDirect3DDevice9* g_pD3DDevice;

    // Atomic flag signaling DLL unload request
    extern std::atomic<bool> g_bUnloadRequested;

    // Base addresses of key X-Ray engine modules
    extern uintptr_t g_xr3DABase;
    extern uintptr_t g_xrCoreBase;
    extern uintptr_t g_xrGameBase;
    extern uintptr_t g_xrRenderBase;

    // Flag indicating whether hooks & ImGui are initialized
    extern bool g_bInitialized;
}
