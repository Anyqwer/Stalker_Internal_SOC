#include "Globals.hpp"

namespace Core
{
    HMODULE g_hModule = nullptr;
    HWND g_hGameWindow = nullptr;
    IDirect3DDevice9* g_pD3DDevice = nullptr;
    std::atomic<bool> g_bUnloadRequested{ false };

    uintptr_t g_xr3DABase = 0;
    uintptr_t g_xrCoreBase = 0;
    uintptr_t g_xrGameBase = 0;
    uintptr_t g_xrRenderBase = 0;

    bool g_bInitialized = false;
}
