#pragma once

#include "XRaySDK.hpp"
#include "../core/Globals.hpp"
#include "../core/Memory.hpp"
#include "../core/Logger.hpp"

namespace XRay
{
    class Engine
    {
    public:
        static bool Initialize()
        {
            HMODULE h3DA = GetModuleHandleA("XR_3DA.exe");
            if (!h3DA)
            {
                h3DA = GetModuleHandleA(nullptr);
            }

            if (h3DA)
            {
                s_pDevice = reinterpret_cast<CRenderDevice*>(GetProcAddress(h3DA, "?Device@@3VCRenderDevice@@A"));
                s_ppGameLevel = reinterpret_cast<CLevel**>(GetProcAddress(h3DA, "?g_pGameLevel@@3PAVIGame_Level@@A"));

                s_fnObjList_o_count = reinterpret_cast<tCObjectList_o_count>(GetProcAddress(h3DA, "?o_count@CObjectList@@QAEIXZ"));
                s_fnObjList_o_get_by_iterator = reinterpret_cast<tCObjectList_o_get_by_iterator>(GetProcAddress(h3DA, "?o_get_by_iterator@CObjectList@@QAEPAVCObject@@I@Z"));

                if (s_pDevice)
                {
                    Core::Logger::Success("Located CRenderDevice instance at 0x%p", s_pDevice);
                }
                if (s_ppGameLevel)
                {
                    Core::Logger::Success("Located g_pGameLevel exported address at 0x%p", s_ppGameLevel);
                }
                if (s_fnObjList_o_count && s_fnObjList_o_get_by_iterator)
                {
                    Core::Logger::Success("Located CObjectList::o_count and o_get_by_iterator");
                }
            }

            return true;
        }

        static CRenderDevice* GetDevice()
        {
            if (!s_pDevice)
            {
                HMODULE h3DA = GetModuleHandleA("XR_3DA.exe");
                if (h3DA)
                    s_pDevice = reinterpret_cast<CRenderDevice*>(GetProcAddress(h3DA, "?Device@@3VCRenderDevice@@A"));
            }
            return s_pDevice;
        }

        static CLevel* GetLevel()
        {
            if (!s_ppGameLevel)
            {
                HMODULE h3DA = GetModuleHandleA("XR_3DA.exe");
                if (h3DA)
                    s_ppGameLevel = reinterpret_cast<CLevel**>(GetProcAddress(h3DA, "?g_pGameLevel@@3PAVIGame_Level@@A"));
            }

            if (s_ppGameLevel)
            {
                CLevel* pLevel = nullptr;
                if (Core::Memory::SafeRead(reinterpret_cast<uintptr_t>(s_ppGameLevel), pLevel))
                {
                    return pLevel;
                }
            }
            return nullptr;
        }

        static CObject* GetCurrentEntity()
        {
            CLevel* pLevel = GetLevel();
            if (!pLevel) return nullptr;

            uintptr_t lvlAddr = reinterpret_cast<uintptr_t>(pLevel);
            CObject* pActor = nullptr;
            Core::Memory::SafeRead(lvlAddr + 0x20, pActor);
            return pActor;
        }

        static xr_vector<CObject*>* GetActiveObjects()
        {
            CLevel* pLevel = GetLevel();
            if (!pLevel) return nullptr;

            // In S.T.A.L.K.E.R. SoC (1.0006):
            // CObjectList is at offset 0x54 in g_pGameLevel
            // Inside CObjectList, objects_active vector is at offset 0x1C
            uintptr_t lvlAddr = reinterpret_cast<uintptr_t>(pLevel);
            uintptr_t objList = lvlAddr + 0x54;
            return reinterpret_cast<xr_vector<CObject*>*>(objList + 0x1C);
        }

        static xr_vector<CObject*>* GetSleepingObjects()
        {
            CLevel* pLevel = GetLevel();
            if (!pLevel) return nullptr;

            uintptr_t lvlAddr = reinterpret_cast<uintptr_t>(pLevel);
            uintptr_t objList = lvlAddr + 0x54;
            return reinterpret_cast<xr_vector<CObject*>*>(objList + 0x2C);
        }

        static uint32_t GetTotalObjectCount()
        {
            CLevel* pLevel = GetLevel();
            if (!pLevel) return 0;

            uintptr_t lvlAddr = reinterpret_cast<uintptr_t>(pLevel);
            void* objList = reinterpret_cast<void*>(lvlAddr + 0x54);

            if (s_fnObjList_o_count)
            {
                __try
                {
                    return s_fnObjList_o_count(objList);
                }
                __except (EXCEPTION_EXECUTE_HANDLER)
                {
                }
            }

            xr_vector<CObject*>* active = GetActiveObjects();
            xr_vector<CObject*>* sleep = GetSleepingObjects();
            uint32_t count = 0;
            if (active) count += static_cast<uint32_t>(active->size());
            if (sleep) count += static_cast<uint32_t>(sleep->size());
            return count;
        }

        static CObject* GetObjectByIndex(uint32_t index)
        {
            CLevel* pLevel = GetLevel();
            if (!pLevel) return nullptr;

            uintptr_t lvlAddr = reinterpret_cast<uintptr_t>(pLevel);
            void* objList = reinterpret_cast<void*>(lvlAddr + 0x54);

            if (s_fnObjList_o_get_by_iterator)
            {
                __try
                {
                    return s_fnObjList_o_get_by_iterator(objList, index);
                }
                __except (EXCEPTION_EXECUTE_HANDLER)
                {
                }
            }

            xr_vector<CObject*>* active = GetActiveObjects();
            if (active)
            {
                size_t activeCount = active->size();
                if (index < activeCount)
                    return active->at(index);
                index -= static_cast<uint32_t>(activeCount);
            }

            xr_vector<CObject*>* sleep = GetSleepingObjects();
            if (sleep && index < sleep->size())
            {
                return sleep->at(index);
            }

            return nullptr;
        }

        static Fmatrix GetViewMatrix()
        {
            CRenderDevice* pDev = GetDevice();
            if (!pDev) return Fmatrix();

            uintptr_t devAddr = reinterpret_cast<uintptr_t>(pDev);
            Fmatrix view;
            if (Core::Memory::SafeRead(devAddr + 0x23C, view))
                return view;

            return Fmatrix();
        }

        static Fmatrix GetProjectMatrix()
        {
            CRenderDevice* pDev = GetDevice();
            if (!pDev) return Fmatrix();

            uintptr_t devAddr = reinterpret_cast<uintptr_t>(pDev);
            Fmatrix proj;
            if (Core::Memory::SafeRead(devAddr + 0x27C, proj))
                return proj;

            return Fmatrix();
        }

        static Fmatrix GetFullTransform()
        {
            CRenderDevice* pDev = GetDevice();
            if (!pDev) return Fmatrix();

            uintptr_t devAddr = reinterpret_cast<uintptr_t>(pDev);
            Fmatrix full;
            // In X-Ray 1.0006: CRenderDevice has precalculated mFullTransform at offset 0x2BC
            if (Core::Memory::SafeRead(devAddr + 0x2BC, full))
                return full;

            Fmatrix v = GetViewMatrix();
            Fmatrix p = GetProjectMatrix();
            return v * p;
        }

        static Fvector GetCameraPosition()
        {
            CRenderDevice* pDev = GetDevice();
            if (!pDev) return Fvector(0, 0, 0);

            uintptr_t devAddr = reinterpret_cast<uintptr_t>(pDev);
            Fvector camPos;
            if (Core::Memory::SafeRead(devAddr + 0x20C, camPos))
                return camPos;

            return Fvector(0, 0, 0);
        }

    private:
        typedef uint32_t(__thiscall* tCObjectList_o_count)(void* thisPtr);
        typedef CObject*(__thiscall* tCObjectList_o_get_by_iterator)(void* thisPtr, uint32_t index);

        static inline CRenderDevice* s_pDevice = nullptr;
        static inline CLevel** s_ppGameLevel = nullptr;
        static inline tCObjectList_o_count s_fnObjList_o_count = nullptr;
        static inline tCObjectList_o_get_by_iterator s_fnObjList_o_get_by_iterator = nullptr;
    };
}
