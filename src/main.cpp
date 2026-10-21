#include <windows.h>
#include "core/Globals.hpp"
#include "core/Config.hpp"
#include "core/Logger.hpp"
#include "hooks/Hooks.hpp"
#include "xray/Offsets.hpp"

DWORD WINAPI CheatMainThread(LPVOID lpParam)
{
    // 1. Initialize colored debug console
    if (g_Config.settings.bShowConsole)
    {
        Core::Logger::InitConsole();
    }

    Core::Logger::Info("DLL injected into process ID: %d", GetCurrentProcessId());

    // 2. Wait for game modules to load completely
    Core::Logger::Info("Waiting for game engine modules to load...");
    while ((!GetModuleHandleA("XR_3DA.exe") && !GetModuleHandleA("xrEngine.exe")) ||
           (!GetModuleHandleA("xrRender_R1.dll") && !GetModuleHandleA("xrRender_R2.dll") &&
            !GetModuleHandleA("xrRender_R3.dll") && !GetModuleHandleA("xrRender_R4.dll") &&
            !GetModuleHandleA("d3d9.dll")))
    {
        Sleep(200);
    }

    Core::Logger::Success("Game modules detected! Setting up SDK & hooks...");

    // Store module bases
    HMODULE hEngine = GetModuleHandleA("xrEngine.exe");
    if (!hEngine) hEngine = GetModuleHandleA("XR_3DA.exe");
    Core::g_xr3DABase = reinterpret_cast<uintptr_t>(hEngine);
    Core::g_xrCoreBase = reinterpret_cast<uintptr_t>(GetModuleHandleA("xrCore.dll"));
    Core::g_xrGameBase = reinterpret_cast<uintptr_t>(GetModuleHandleA("xrGame.dll"));

    // 3. Initialize X-Ray Engine structures & pointers
    XRay::Engine::Initialize();

    // 4. Initialize DirectX 9 and WndProc Hooks
    if (!Hooks::Manager::Initialize())
    {
        Core::Logger::Error("Failed to initialize hooks! Aborting injection...");
        Sleep(2000);
        Core::Logger::ShutdownConsole();
        FreeLibraryAndExitThread(Core::g_hModule, 1);
        return 1;
    }

    Core::Logger::Success("=============================================");
    Core::Logger::Success("   S.T.A.L.K.E.R. Internal Cheat Active!    ");
    Core::Logger::Success("   Press [INSERT] to Toggle Menu             ");
    Core::Logger::Success("   Press [END] to Safely Eject DLL           ");
    Core::Logger::Success("=============================================");

    // 5. Main loop - wait until eject / unload is signaled
    while (!Core::g_bUnloadRequested.load())
    {
        Sleep(100);
    }

    // 6. Clean Unhook and Shutdown
    Core::Logger::Warning("Shutdown signal received! Cleaning up hooks...");
    Hooks::Manager::Shutdown();

    Sleep(500);
    Core::Logger::Info("Goodbye Stalker!");
    Core::Logger::ShutdownConsole();

    // Safely exit and unmap module from game process
    FreeLibraryAndExitThread(Core::g_hModule, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);
        Core::g_hModule = hModule;

        HANDLE hThread = CreateThread(
            nullptr,
            0,
            reinterpret_cast<LPTHREAD_START_ROUTINE>(CheatMainThread),
            hModule,
            0,
            nullptr
        );

        if (hThread)
        {
            CloseHandle(hThread);
        }
    }

    return TRUE;
}
