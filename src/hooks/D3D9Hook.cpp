#include "D3D9Hook.hpp"
#include "../core/Logger.hpp"

namespace Hooks
{
    bool D3D9Hook::GetVTable(void** pVTableOut, size_t tableSize)
    {
        if (!pVTableOut || tableSize == 0) return false;

        // Register dummy window class
        WNDCLASSEXA wc = {};
        wc.cbSize = sizeof(WNDCLASSEXA);
        wc.style = CS_CLASSDC;
        wc.lpfnWndProc = DefWindowProcA;
        wc.hInstance = GetModuleHandleA(nullptr);
        wc.lpszClassName = "XRay_Dummy_D3D_Class";

        RegisterClassExA(&wc);

        HWND hWnd = CreateWindowA(
            wc.lpszClassName,
            "XRay_Dummy_Window",
            WS_OVERLAPPEDWINDOW,
            0, 0, 100, 100,
            nullptr, nullptr, wc.hInstance, nullptr
        );

        if (!hWnd)
        {
            UnregisterClassA(wc.lpszClassName, wc.hInstance);
            Core::Logger::Error("Failed to create dummy D3D9 window!");
            return false;
        }

        IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
        if (!pD3D)
        {
            DestroyWindow(hWnd);
            UnregisterClassA(wc.lpszClassName, wc.hInstance);
            Core::Logger::Error("Failed to create IDirect3D9 instance!");
            return false;
        }

        D3DPRESENT_PARAMETERS d3dpp = {};
        d3dpp.Windowed = TRUE;
        d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
        d3dpp.hDeviceWindow = hWnd;

        IDirect3DDevice9* pDummyDevice = nullptr;
        HRESULT hr = pD3D->CreateDevice(
            D3DADAPTER_DEFAULT,
            D3DDEVTYPE_HAL,
            hWnd,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING,
            &d3dpp,
            &pDummyDevice
        );

        if (FAILED(hr) || !pDummyDevice)
        {
            pD3D->Release();
            DestroyWindow(hWnd);
            UnregisterClassA(wc.lpszClassName, wc.hInstance);
            Core::Logger::Error("Failed to create dummy Direct3D9 device! HRESULT: 0x%08X", hr);
            return false;
        }

        // Copy VTable addresses
        void** pVMT = *reinterpret_cast<void***>(pDummyDevice);
        memcpy(pVTableOut, pVMT, tableSize * sizeof(void*));

        // Cleanup dummy resources
        pDummyDevice->Release();
        pD3D->Release();
        DestroyWindow(hWnd);
        UnregisterClassA(wc.lpszClassName, wc.hInstance);

        return true;
    }
}
