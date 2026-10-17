#pragma once

#include <windows.h>
#include <d3d9.h>

namespace Hooks
{
    class D3D9Hook
    {
    public:
        // Table indices in IDirect3DDevice9 Virtual Method Table (VMT)
        enum VMTIndex : int
        {
            Reset = 16,
            Present = 17,
            EndScene = 42
        };

        // Create a dummy device to extract the real VMT pointers
        static bool GetVTable(void** pVTableOut, size_t tableSize);
    };
}
