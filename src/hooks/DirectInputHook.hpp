#pragma once

#include <windows.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

namespace Hooks
{
    class DirectInputHook
    {
    public:
        static bool Initialize();
        static void Shutdown();

    private:
        typedef HRESULT(APIENTRY* tGetDeviceState)(IDirectInputDevice8A*, DWORD, LPVOID);
        typedef HRESULT(APIENTRY* tGetDeviceData)(IDirectInputDevice8A*, DWORD, LPDIDEVICEOBJECTDATA, LPDWORD, DWORD);

        static HRESULT APIENTRY Hooked_GetDeviceState(IDirectInputDevice8A* pDevice, DWORD cbData, LPVOID lpvData);
        static HRESULT APIENTRY Hooked_GetDeviceData(IDirectInputDevice8A* pDevice, DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags);

        static inline tGetDeviceState s_oGetDeviceState = nullptr;
        static inline tGetDeviceData s_oGetDeviceData = nullptr;

        static inline void* s_pTargetGetDeviceState = nullptr;
        static inline void* s_pTargetGetDeviceData = nullptr;
        static inline bool s_bHooked = false;
    };
}
