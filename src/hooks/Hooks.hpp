#pragma once

#include <windows.h>
#include <d3d9.h>

namespace Hooks
{
    class Manager
    {
    public:
        static bool Initialize();
        static void Shutdown();

    private:
        // Function typedefs
        typedef HRESULT(APIENTRY* tEndScene)(IDirect3DDevice9*);
        typedef HRESULT(APIENTRY* tReset)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);

        // Hook callbacks
        static HRESULT APIENTRY Hooked_EndScene(IDirect3DDevice9* pDevice);
        static HRESULT APIENTRY Hooked_Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);

        // Original function pointers
        static inline tEndScene s_oEndScene = nullptr;
        static inline tReset s_oReset = nullptr;

        static inline void* s_pTargetEndScene = nullptr;
        static inline void* s_pTargetReset = nullptr;
    };
}
