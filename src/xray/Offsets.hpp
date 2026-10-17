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

        // Raycast Visibility Check using native X-Ray CObjectSpace::RayTest
        static bool IsVisible(const Fvector& from, const Fvector& to, CObject* pIgnore = nullptr)
        {
            CLevel* pLevel = GetLevel();
            if (!pLevel) return true;

            uintptr_t lvlAddr = reinterpret_cast<uintptr_t>(pLevel);
            void* pObjectSpace = reinterpret_cast<void*>(lvlAddr + 0xCC);
            if (!pObjectSpace || IsBadReadPtr(pObjectSpace, sizeof(void*)))
                return true;

            if (!s_fnRayTest)
            {
                HMODULE h3DA = GetModuleHandleA("XR_3DA.exe");
                if (h3DA)
                {
                    s_fnRayTest = reinterpret_cast<tRayTest>(GetProcAddress(h3DA,
                        "?RayTest@CObjectSpace@@QAEHABU?$_vector3@M@@0MW4rq_target@collide@@PAUray_cache@4@PAVCObject@@@Z"));
                    if (!s_fnRayTest)
                    {
                        s_fnRayTest = reinterpret_cast<tRayTest>(reinterpret_cast<uintptr_t>(h3DA) + 0x4E300);
                    }
                }
            }

            if (!s_fnRayTest) return true;

            Fvector dir = to - from;
            float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
            if (dist <= 0.4f) return true;

            float invDist = 1.0f / dist;
            Fvector dirNorm(dir.x * invDist, dir.y * invDist, dir.z * invDist);

            // Stop 0.35m before the target bone so the ray doesn't collide with the enemy's own weapon/model
            float testDist = (dist > 0.5f) ? (dist - 0.35f) : (dist * 0.5f);

            __try
            {
                // rqtStatic = 2: Only solid world geometry (walls, terrain, buildings) blocks LOS.
                // Dynamic bushes, foliage, leaves, weapons, and ragdolls will NOT falsely block visibility!
                BOOL bBlocked = s_fnRayTest(pObjectSpace, from, dirNorm, testDist, 2, nullptr, pIgnore);
                return (bBlocked == FALSE);
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                return true;
            }
        }

        // Extract accurate world-space position of bone (Head, Neck, Chest, Pelvis) with crouch support
        static bool GetBoneWorldPosition(CObject* pObj, int boneTargetType, Fvector& outWorldPos)
        {
            if (!pObj) return false;

            __try
            {
                uintptr_t objAddr = reinterpret_cast<uintptr_t>(pObj);
                uintptr_t visual = *reinterpret_cast<uintptr_t*>(objAddr + 0x90);
                if (!visual || IsBadReadPtr(reinterpret_cast<void*>(visual), sizeof(void*)))
                    return false;

                void** vtbl = *reinterpret_cast<void***>(visual);
                if (!vtbl || IsBadReadPtr(vtbl, sizeof(void*) * 8))
                    return false;

                typedef void* (__thiscall* t_dcast_PKinematics)(void* thisPtr);
                auto dcast = reinterpret_cast<t_dcast_PKinematics>(vtbl[6]);
                void* pKinematics = dcast(reinterpret_cast<void*>(visual));
                if (!pKinematics || IsBadReadPtr(pKinematics, 0xA0))
                    return false;

                uintptr_t kAddr = reinterpret_cast<uintptr_t>(pKinematics);
                uintptr_t boneInstances = *reinterpret_cast<uintptr_t*>(kAddr + 0x84);
                if (!boneInstances || IsBadReadPtr(reinterpret_cast<void*>(boneInstances), sizeof(CBoneInstance)))
                    return false;

                if (!s_fnLL_BoneID)
                {
                    HMODULE h3DA = GetModuleHandleA("XR_3DA.exe");
                    if (h3DA)
                    {
                        s_fnLL_BoneID = reinterpret_cast<tLL_BoneID>(GetProcAddress(h3DA,
                            "?LL_BoneID@CKinematics@@QAEGPBD@Z"));
                        if (!s_fnLL_BoneID)
                        {
                            s_fnLL_BoneID = reinterpret_cast<tLL_BoneID>(reinterpret_cast<uintptr_t>(h3DA) + 0x6C6E0);
                        }
                    }
                }

                uint16_t boneId = 0xFFFF;
                if (s_fnLL_BoneID)
                {
                    const char* boneName = "bip01_head";
                    if (boneTargetType == 1) boneName = "bip01_neck";
                    else if (boneTargetType == 2) boneName = "bip01_spine2";
                    else if (boneTargetType == 3) boneName = "bip01_pelvis";

                    boneId = s_fnLL_BoneID(pKinematics, boneName);

                    // Fallbacks for mutants / non-standard rigs
                    if (boneId == 0xFFFF)
                    {
                        if (boneTargetType <= 1) boneId = s_fnLL_BoneID(pKinematics, "head");
                        if (boneId == 0xFFFF) boneId = s_fnLL_BoneID(pKinematics, "bip01_spine1");
                        if (boneId == 0xFFFF) boneId = s_fnLL_BoneID(pKinematics, "bip01_spine");
                    }
                }

                if (boneId != 0xFFFF && boneId < 128)
                {
                    float lx = *reinterpret_cast<float*>(boneInstances + boneId * 0xA0 + 0x30);
                    float ly = *reinterpret_cast<float*>(boneInstances + boneId * 0xA0 + 0x34);
                    float lz = *reinterpret_cast<float*>(boneInstances + boneId * 0xA0 + 0x38);

                    Fvector localBone(lx, ly, lz);
                    pObj->render_transform.Transform(outWorldPos, localBone);
                    return true;
                }

                // If bone ID lookup failed, find the highest bone in local Y (head)
                uintptr_t pBoneDataVec = *reinterpret_cast<uintptr_t*>(kAddr + 0x88);
                if (pBoneDataVec && !IsBadReadPtr(reinterpret_cast<void*>(pBoneDataVec), 0x10))
                {
                    CBoneData** boneDataFirst = *reinterpret_cast<CBoneData***>(pBoneDataVec + 0x04);
                    CBoneData** boneDataLast = *reinterpret_cast<CBoneData***>(pBoneDataVec + 0x08);
                    if (boneDataFirst && boneDataLast && boneDataLast > boneDataFirst)
                    {
                        size_t count = boneDataLast - boneDataFirst;
                        if (count > 0 && count <= 128)
                        {
                            size_t bestIdx = 0;
                            float maxLy = -1e6f;
                            for (size_t i = 0; i < count; ++i)
                            {
                                float ly = *reinterpret_cast<float*>(boneInstances + i * 0xA0 + 0x34);
                                if (ly > maxLy)
                                {
                                    maxLy = ly;
                                    bestIdx = i;
                                }
                            }

                            float lx = *reinterpret_cast<float*>(boneInstances + bestIdx * 0xA0 + 0x30);
                            float ly = *reinterpret_cast<float*>(boneInstances + bestIdx * 0xA0 + 0x34);
                            float lz = *reinterpret_cast<float*>(boneInstances + bestIdx * 0xA0 + 0x38);

                            Fvector localBone(lx, ly, lz);
                            pObj->render_transform.Transform(outWorldPos, localBone);
                            return true;
                        }
                    }
                }

                return false;
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                return false;
            }
        }

        static uintptr_t GetEnvironment()
        {
            HMODULE h3DA = GetModuleHandleA("XR_3DA.exe");
            if (!h3DA) return 0;

            static void** ppPersistent = nullptr;
            if (!ppPersistent)
            {
                ppPersistent = reinterpret_cast<void**>(GetProcAddress(h3DA, "?g_pGamePersistent@@3PAVIGame_Persistent@@A"));
                if (!ppPersistent)
                    ppPersistent = reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(h3DA) + 0x10BB7C);
            }

            if (!ppPersistent || !*ppPersistent || IsBadReadPtr(*ppPersistent, sizeof(void*)))
                return 0;

            uintptr_t pPersistent = reinterpret_cast<uintptr_t>(*ppPersistent);
            if (IsBadReadPtr(reinterpret_cast<void*>(pPersistent), 0x500))
                return 0;

            uintptr_t pEnv = *reinterpret_cast<uintptr_t*>(pPersistent + 0x46C);
            if (!pEnv || IsBadReadPtr(reinterpret_cast<void*>(pEnv), 0x300))
                return 0;

            return pEnv;
        }

        static void SetGameTime(float gameTimeSeconds, float timeFactor = 1.0f)
        {
            uintptr_t pEnv = GetEnvironment();
            if (!pEnv) return;

            typedef void (__thiscall* tSetGameTime)(void* thisPtr, float time, float factor);
            static tSetGameTime s_fnSetGameTime = nullptr;

            if (!s_fnSetGameTime)
            {
                HMODULE h3DA = GetModuleHandleA("XR_3DA.exe");
                if (h3DA)
                {
                    s_fnSetGameTime = reinterpret_cast<tSetGameTime>(GetProcAddress(h3DA,
                        "?SetGameTime@CEnvironment@@QAEXMM@Z"));
                    if (!s_fnSetGameTime)
                        s_fnSetGameTime = reinterpret_cast<tSetGameTime>(reinterpret_cast<uintptr_t>(h3DA) + 0x38730);
                }
            }

            if (s_fnSetGameTime)
            {
                __try
                {
                    s_fnSetGameTime(reinterpret_cast<void*>(pEnv), gameTimeSeconds, timeFactor);
                }
                __except (EXCEPTION_EXECUTE_HANDLER)
                {
                }
            }
        }

    private:
        typedef uint32_t(__thiscall* tCObjectList_o_count)(void* thisPtr);
        typedef CObject*(__thiscall* tCObjectList_o_get_by_iterator)(void* thisPtr, uint32_t index);
        typedef BOOL(__thiscall* tRayTest)(void* thisPtr, const Fvector& start, const Fvector& dir, float range,
                                           int rqTarget, void* pRayCache, CObject* pIgnoreObj);
        typedef uint16_t(__thiscall* tLL_BoneID)(void* thisPtr, const char* boneName);

        static inline CRenderDevice* s_pDevice = nullptr;
        static inline CLevel** s_ppGameLevel = nullptr;
        static inline tCObjectList_o_count s_fnObjList_o_count = nullptr;
        static inline tCObjectList_o_get_by_iterator s_fnObjList_o_get_by_iterator = nullptr;
        static inline tRayTest s_fnRayTest = nullptr;
        static inline tLL_BoneID s_fnLL_BoneID = nullptr;
    };
}
