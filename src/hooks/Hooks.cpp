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
#include <algorithm>
#include <imgui.h>
#include <backends/imgui_impl_dx9.h>
#include <backends/imgui_impl_win32.h>
#include "../xray/Offsets.hpp"

namespace Hooks
{
    static IDirect3DTexture9* s_pTexVisible = nullptr;
    static IDirect3DTexture9* s_pTexHidden = nullptr;
    static uint32_t s_lastColVis = 0;
    static uint32_t s_lastColHid = 0;



    static void ReleaseChamsTextures()
    {
        if (s_pTexVisible) { s_pTexVisible->Release(); s_pTexVisible = nullptr; }
        if (s_pTexHidden) { s_pTexHidden->Release(); s_pTexHidden = nullptr; }
        s_lastColVis = 0;
        s_lastColHid = 0;
    }

    static void UpdateChamsTexture(IDirect3DDevice9* pDevice, IDirect3DTexture9** ppTex, uint32_t col, uint32_t& lastCol)
    {
        if (*ppTex && col == lastCol) return;
        if (*ppTex) { (*ppTex)->Release(); *ppTex = nullptr; }

        if (SUCCEEDED(pDevice->CreateTexture(1, 1, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, ppTex, nullptr)))
        {
            D3DLOCKED_RECT rect;
            if (SUCCEEDED((*ppTex)->LockRect(0, &rect, nullptr, 0)))
            {
                *reinterpret_cast<uint32_t*>(rect.pBits) = col;
                (*ppTex)->UnlockRect(0);
                lastCol = col;
            }
        }
    }

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

    HRESULT APIENTRY Manager::Hooked_DrawIndexedPrimitive(
        IDirect3DDevice9* pDevice,
        D3DPRIMITIVETYPE Type,
        INT BaseVertexIndex,
        UINT MinVertexIndex,
        UINT NumVertices,
        UINT startIndex,
        UINT primCount)
    {
        if (!pDevice)
            return s_oDrawIndexedPrimitive(pDevice, Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);

        // Crucial stability guard: only run chams inside active gameplay!
        // Prevents loading screen, level transition, and main menu crashes.
        if (!g_Config.chams.bEnabled || !XRay::Engine::IsInGame())
            return s_oDrawIndexedPrimitive(pDevice, Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);

        __try
        {
            // Skeletal models (Stalkers & Mutants) use compressed skinning vertex formats in X-Ray:
            // Decl 1: SHORT4 (8) + UBYTE4 (4) + UBYTE4 (4) + UBYTE4 (4) + SHORT2 (4) = 24 bytes
            // Decl 2: SHORT4 (8) + UBYTE4 (4) + UBYTE4 (4) + UBYTE4 (4) + SHORT4 (8) = 28 bytes
            // In contrast, 2D HUD text and numbers use FVF (XYZRHW / POSITIONT with FLOAT4)
            UINT stride = 0;
            IDirect3DVertexBuffer9* pVB = nullptr;
            UINT vbOffset = 0;
            if (SUCCEEDED(pDevice->GetStreamSource(0, &pVB, &vbOffset, &stride)) && pVB)
            {
                pVB->Release();
            }

            bool isCharacter = false;
            if (stride == 24 || stride == 28)
            {
                IDirect3DVertexDeclaration9* pDecl = nullptr;
                if (SUCCEEDED(pDevice->GetVertexDeclaration(&pDecl)) && pDecl)
                {
                    D3DVERTEXELEMENT9 elems[64];
                    UINT numElements = 0;
                    if (SUCCEEDED(pDecl->GetDeclaration(elems, &numElements)) && numElements >= 4)
                    {
                        // Compressed skinning in X-Ray: element 0 has SHORT4 position.
                        if (elems[0].Type == D3DDECLTYPE_SHORT4 && elems[0].Usage == D3DDECLUSAGE_POSITION)
                        {
                            isCharacter = true;
                        }
                    }
                    pDecl->Release();
                }
            }

            if (isCharacter)
            {
                ImVec4 cv = g_Config.chams.colVisible;
                ImVec4 ch = g_Config.chams.colHidden;

                // Style 3: Pulsating Glow
                if (g_Config.chams.iChamsType == 3)
                {
                    float pulse = 0.55f + 0.45f * sinf(static_cast<float>(ImGui::GetTime()) * g_Config.chams.fGlowSpeed);
                    cv.x = std::clamp(cv.x * pulse, 0.0f, 1.0f);
                    cv.y = std::clamp(cv.y * pulse, 0.0f, 1.0f);
                    cv.z = std::clamp(cv.z * pulse, 0.0f, 1.0f);
                    ch.x = std::clamp(ch.x * pulse, 0.0f, 1.0f);
                    ch.y = std::clamp(ch.y * pulse, 0.0f, 1.0f);
                    ch.z = std::clamp(ch.z * pulse, 0.0f, 1.0f);
                }
                // Style 5: Rainbow / Chromatic
                else if (g_Config.chams.iChamsType == 5)
                {
                    float hue = fmodf(static_cast<float>(ImGui::GetTime()) * 0.25f, 1.0f);
                    float r, g, b;
                    ImGui::ColorConvertHSVtoRGB(hue, 0.9f, 1.0f, r, g, b);
                    cv = ImVec4(r, g, b, 1.0f);
                    float hue2 = fmodf(hue + 0.5f, 1.0f);
                    ImGui::ColorConvertHSVtoRGB(hue2, 0.9f, 1.0f, r, g, b);
                    ch = ImVec4(r, g, b, 1.0f);
                }

                uint32_t colVis = D3DCOLOR_COLORVALUE(cv.x, cv.y, cv.z, cv.w);
                uint32_t colHid = D3DCOLOR_COLORVALUE(ch.x, ch.y, ch.z, ch.w);

                UpdateChamsTexture(pDevice, &s_pTexVisible, colVis, s_lastColVis);
                UpdateChamsTexture(pDevice, &s_pTexHidden, colHid, s_lastColHid);

                DWORD dwZEnable = D3DZB_TRUE, dwZFunc = D3DCMP_LESSEQUAL, dwZWrite = TRUE, dwFillMode = D3DFILL_SOLID;
                DWORD dwAlphaBlend = FALSE, dwSrcBlend = D3DBLEND_ONE, dwDestBlend = D3DBLEND_ZERO;
                pDevice->GetRenderState(D3DRS_ZENABLE, &dwZEnable);
                pDevice->GetRenderState(D3DRS_ZFUNC, &dwZFunc);
                pDevice->GetRenderState(D3DRS_ZWRITEENABLE, &dwZWrite);
                pDevice->GetRenderState(D3DRS_FILLMODE, &dwFillMode);
                pDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlphaBlend);
                pDevice->GetRenderState(D3DRS_SRCBLEND, &dwSrcBlend);
                pDevice->GetRenderState(D3DRS_DESTBLEND, &dwDestBlend);

                IDirect3DBaseTexture9* pOldTex = nullptr;
                pDevice->GetTexture(0, &pOldTex);

                DWORD fillMode = (g_Config.chams.iChamsType == 1) ? D3DFILL_WIREFRAME : D3DFILL_SOLID;
                pDevice->SetRenderState(D3DRS_FILLMODE, fillMode);

                // Style 4: Glass Hologram (Additive blend)
                if (g_Config.chams.iChamsType == 4)
                {
                    pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
                    pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
                    pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
                }

                // PASS 1: Occluded / Behind Walls (X-Ray through-walls pass)
                if (g_Config.chams.bThroughWalls)
                {
                    pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
                    pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
                    pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
                    if (s_pTexHidden) pDevice->SetTexture(0, s_pTexHidden);

                    s_oDrawIndexedPrimitive(pDevice, Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
                }

                // PASS 2: Visible / Front of Walls
                pDevice->SetRenderState(D3DRS_ZENABLE, dwZEnable);
                pDevice->SetRenderState(D3DRS_ZFUNC, dwZFunc);
                pDevice->SetRenderState(D3DRS_ZWRITEENABLE, dwZWrite);
                if (s_pTexVisible) pDevice->SetTexture(0, s_pTexVisible);

                HRESULT hr = s_oDrawIndexedPrimitive(pDevice, Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);

                // Restore original Direct3D 9 pipeline states
                pDevice->SetRenderState(D3DRS_ZENABLE, dwZEnable);
                pDevice->SetRenderState(D3DRS_ZFUNC, dwZFunc);
                pDevice->SetRenderState(D3DRS_ZWRITEENABLE, dwZWrite);
                pDevice->SetRenderState(D3DRS_FILLMODE, dwFillMode);
                pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlphaBlend);
                pDevice->SetRenderState(D3DRS_SRCBLEND, dwSrcBlend);
                pDevice->SetRenderState(D3DRS_DESTBLEND, dwDestBlend);
                pDevice->SetTexture(0, pOldTex);
                if (pOldTex) pOldTex->Release();

                return hr;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
        }

        return s_oDrawIndexedPrimitive(pDevice, Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
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

                // Strip WS_EX_TOPMOST so game window does NOT lock foreground / block Alt-Tab
                if (g_Config.settings.bPreventTopmost)
                {
                    LONG_PTR exStyle = GetWindowLongPtrA(Core::g_hGameWindow, GWL_EXSTYLE);
                    if (exStyle & WS_EX_TOPMOST)
                    {
                        SetWindowLongPtrA(Core::g_hGameWindow, GWL_EXSTYLE, exStyle & ~WS_EX_TOPMOST);
                        SetWindowPos(Core::g_hGameWindow, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
                        Core::Logger::Success("Removed WS_EX_TOPMOST from S.T.A.L.K.E.R. window (Alt-Tab enabled)");
                    }
                }
            }
            else
            {
                Core::Logger::Error("Failed to find valid game window HWND!");
            }

            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.IniFilename = nullptr; // Don't create external imgui.ini

            // Load Cyrillic font for full Russian language support
            const ImWchar* glyphRanges = io.Fonts->GetGlyphRangesCyrillic();
            const char* fontCandidates[] = {
                "C:\\Windows\\Fonts\\tahoma.ttf",
                "C:\\Windows\\Fonts\\arial.ttf",
                "C:\\Windows\\Fonts\\segoeui.ttf"
            };

            bool fontLoaded = false;
            for (const char* path : fontCandidates)
            {
                if (GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES)
                {
                    io.Fonts->AddFontFromFileTTF(path, 15.0f, nullptr, glyphRanges);
                    fontLoaded = true;
                    Core::Logger::Success("Loaded Cyrillic UI font: %s", path);
                    break;
                }
            }
            if (!fontLoaded)
            {
                io.Fonts->AddFontDefault();
            }

            ImGui_ImplWin32_Init(Core::g_hGameWindow);
            ImGui_ImplDX9_Init(pDevice);

            UI::Style::ApplyStalkerTheme();

            // Hook DirectInput to prevent game camera spinning and shooting when menu is open
            DirectInputHook::Initialize();

            // Try loading default config if available
            g_Config.LoadConfig("Default");

            Core::g_bInitialized = true;
            Core::Logger::Success("ImGui Direct3D9 pipeline and DirectInput hook initialized successfully!");
        }

        // Global hardware hotkey check
        int menuKey = g_Config.settings.keyMenu.vk_code != 0 ? g_Config.settings.keyMenu.vk_code : VK_INSERT;
        if (GetAsyncKeyState(menuKey) & 1)
        {
            g_Config.bShowMenu = !g_Config.bShowMenu;
            Core::Logger::Info("Menu visibility: %s", g_Config.bShowMenu ? "OPEN" : "CLOSED");

            if (g_Config.bShowMenu)
            {
                ClipCursor(nullptr);
            }
        }

        int unloadKey = g_Config.settings.keyUnload.vk_code != 0 ? g_Config.settings.keyUnload.vk_code : VK_END;
        if (GetAsyncKeyState(unloadKey) & 1)
        {
            Core::Logger::Warning("Eject key pressed! Initiating DLL shutdown...");
            Core::g_bUnloadRequested.store(true);
        }

        // Render cheat frame
        if (Core::g_bInitialized)
        {
            __try
            {
                ImGuiIO& io = ImGui::GetIO();

                // Software cursor when menu is open
                io.MouseDrawCursor = g_Config.bShowMenu;

                if (g_Config.bShowMenu)
                {
                    ClipCursor(nullptr);
                }

                ImGui_ImplDX9_NewFrame();
                ImGui_ImplWin32_NewFrame();

                // Feed mouse state to ImGui when menu is open
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

                Features::Aimbot::Update();
                Features::Misc::Update();
                Features::Visuals::Render();
                UI::Menu::Render();

                ImGui::EndFrame();
                ImGui::Render();
                ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
            }
        }

        return s_oEndScene(pDevice);
    }

    HRESULT APIENTRY Manager::Hooked_Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
    {
        ReleaseChamsTextures();

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
        s_pTargetDrawIndexedPrimitive = vTable[D3D9Hook::VMTIndex::DrawIndexedPrimitive];

        Core::Logger::Info("D3D9 EndScene Target: 0x%p", s_pTargetEndScene);
        Core::Logger::Info("D3D9 Reset Target:    0x%p", s_pTargetReset);
        Core::Logger::Info("D3D9 DIP Target:      0x%p", s_pTargetDrawIndexedPrimitive);

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

        if (MH_CreateHook(s_pTargetDrawIndexedPrimitive, &Hooked_DrawIndexedPrimitive, reinterpret_cast<void**>(&s_oDrawIndexedPrimitive)) != MH_OK)
        {
            Core::Logger::Error("Failed to create hook for DrawIndexedPrimitive!");
            return false;
        }

        if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
        {
            Core::Logger::Error("Failed to enable MinHook hooks!");
            return false;
        }

        Core::Logger::Success("DirectX 9 EndScene, Reset & DrawIndexedPrimitive hooks successfully enabled!");
        return true;
    }

    void Manager::Shutdown()
    {
        Core::Logger::Info("Shutting down hooks and unlinking ImGui...");

        ReleaseChamsTextures();

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
