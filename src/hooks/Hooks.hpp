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
        typedef HRESULT(APIENTRY* tDrawIndexedPrimitive)(
            IDirect3DDevice9*, D3DPRIMITIVETYPE, INT, UINT, UINT, UINT, UINT
        );

        // Hook callbacks
        static HRESULT APIENTRY Hooked_EndScene(IDirect3DDevice9* pDevice);
        static HRESULT APIENTRY Hooked_Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
        static HRESULT APIENTRY Hooked_DrawIndexedPrimitive(
            IDirect3DDevice9* pDevice, D3DPRIMITIVETYPE Type, INT BaseVertexIndex,
            UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount
        );

        // Original function pointers
        static inline tEndScene s_oEndScene = nullptr;
        static inline tReset s_oReset = nullptr;
        static inline tDrawIndexedPrimitive s_oDrawIndexedPrimitive = nullptr;

        static inline void* s_pTargetEndScene = nullptr;
        static inline void* s_pTargetReset = nullptr;
        static inline void* s_pTargetDrawIndexedPrimitive = nullptr;
    };
}
