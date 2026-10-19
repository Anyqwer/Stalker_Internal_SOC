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

        static bool IsInGame()
        {
            __try
            {
                CRenderDevice* pDev = GetDevice();
                if (!pDev || IsBadReadPtr(pDev, sizeof(void*))) return false;

                CLevel* pLevel = GetLevel();
                if (!pLevel || IsBadReadPtr(pLevel, sizeof(void*))) return false;

                uintptr_t lvlAddr = reinterpret_cast<uintptr_t>(pLevel);
                if (IsBadReadPtr(reinterpret_cast<void*>(lvlAddr + 0x54), 0x40)) return false;

                CObject* pActor = nullptr;
                if (!Core::Memory::SafeRead(lvlAddr + 0x20, pActor)) return false;
                if (!pActor || IsBadReadPtr(pActor, 0x600)) return false;

                uintptr_t actorAddr = reinterpret_cast<uintptr_t>(pActor);
                float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
                if (!Core::Memory::SafeRead(actorAddr + 0x80, posX) ||
                    !Core::Memory::SafeRead(actorAddr + 0x84, posY) ||
                    !Core::Memory::SafeRead(actorAddr + 0x88, posZ))
                    return false;

                if (posX == 0.0f && posY == 0.0f && posZ == 0.0f)
                    return false;

                uintptr_t pConditions = 0;
                if (!Core::Memory::SafeRead(actorAddr + 0x220, pConditions)) return false;
                if (!pConditions || IsBadReadPtr(reinterpret_cast<void*>(pConditions), 0x150)) return false;

                float hp = 0.0f;
                if (!Core::Memory::SafeRead(pConditions + 0x04, hp)) return false;
                if (hp <= 0.0f || hp > 1000.0f || std::isnan(hp)) return false;

                return true;
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                return false;
            }
        }

        static EntityRelation ClassifyRelation(uintptr_t objAddr, CObject* pObj, CObject* pActor)
        {
            if (!pObj || pObj == pActor) return EntityRelation::Unknown;

            __try
            {
                const char* sect = SafeReadSharedStr(objAddr + 0xAC);
                const char* name = SafeReadSharedStr(objAddr + 0xA8);
                const char* visual = SafeReadSharedStr(objAddr + 0xB0);

                char sectLower[128] = { 0 };
                char visualLower[128] = { 0 };
                char nameLower[128] = { 0 };

                for (size_t i = 0; i < sizeof(sectLower) - 1 && sect[i]; ++i)
                    sectLower[i] = static_cast<char>(tolower(static_cast<unsigned char>(sect[i])));
                for (size_t i = 0; i < sizeof(visualLower) - 1 && visual[i]; ++i)
                    visualLower[i] = static_cast<char>(tolower(static_cast<unsigned char>(visual[i])));
                for (size_t i = 0; i < sizeof(nameLower) - 1 && name[i]; ++i)
                    nameLower[i] = static_cast<char>(tolower(static_cast<unsigned char>(name[i])));

                // Skip player
                if (strstr(sectLower, "actor") || strstr(nameLower, "single_player"))
                    return EntityRelation::Unknown;

                // Mutants
                if (strstr(visualLower, "monster") || strstr(visualLower, "mutant") ||
                    strncmp(sectLower, "m_", 2) == 0 ||
                    strstr(sectLower, "boar") || strstr(sectLower, "flesh") || strstr(sectLower, "dog") ||
                    strstr(sectLower, "bloodsucker") || strstr(sectLower, "snork") ||
                    strstr(sectLower, "controller") || strstr(sectLower, "gigant") ||
                    strstr(sectLower, "tushkano") || strstr(sectLower, "burer") ||
                    strstr(sectLower, "chimera") || strstr(sectLower, "poltergeist"))
                {
                    return EntityRelation::Mutant;
                }

                // Check human characters
                bool isHuman = (strstr(visualLower, "actor") || strstr(visualLower, "stalker") || strstr(visualLower, "hero") ||
                                strstr(sectLower, "stalker") || strstr(sectLower, "bandit") || strstr(sectLower, "soldier") ||
                                strstr(sectLower, "military") || strstr(sectLower, "monolith") || strstr(sectLower, "merc") ||
                                strstr(sectLower, "dolg") || strstr(sectLower, "duty") || strstr(sectLower, "freedom") ||
                                strstr(sectLower, "zombied") || strstr(sectLower, "novice") || strstr(sectLower, "veteran") ||
                                strncmp(sectLower, "esc_", 4) == 0 || strncmp(sectLower, "gar_", 4) == 0 ||
                                strncmp(sectLower, "agr_", 4) == 0 || strncmp(sectLower, "val_", 4) == 0 ||
                                strncmp(sectLower, "bar_", 4) == 0 || strncmp(sectLower, "mil_", 4) == 0);

                if (isHuman)
                {
                    // Enemies (Hostile by default in SoC)
                    if (strstr(sectLower, "bandit") || strstr(visualLower, "bandit") || strstr(nameLower, "bandit") ||
                        strstr(sectLower, "monolith") || strstr(visualLower, "monolith") || strstr(nameLower, "monolith") ||
                        strstr(sectLower, "merc") || strstr(sectLower, "killer") || strstr(visualLower, "merc") ||
                        strstr(sectLower, "military") || strstr(sectLower, "soldier") || strstr(visualLower, "soldier") || strstr(sectLower, "voen") ||
                        strstr(sectLower, "zombied") || strstr(visualLower, "zombied"))
                    {
                        return EntityRelation::Enemy;
                    }

                    // Neutrals (Duty & Freedom, Ecologists)
                    if (strstr(sectLower, "freedom") || strstr(sectLower, "svoboda") || strstr(visualLower, "freedom") ||
                        strstr(sectLower, "dolg") || strstr(sectLower, "duty") || strstr(visualLower, "dolg") ||
                        strstr(sectLower, "ecolog") || strstr(visualLower, "ecolog"))
                    {
                        return EntityRelation::Neutral;
                    }

                    // Friends / Allies (Loners, neutral stalkers, traders, bar regulars)
                    return EntityRelation::Friend;
                }

                return EntityRelation::Unknown;
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                return EntityRelation::Unknown;
            }
        }

        static CObject* GetCurrentEntity()
        {
            CLevel* pLevel = GetLevel();
            if (!pLevel || IsBadReadPtr(pLevel, sizeof(void*))) return nullptr;

            uintptr_t lvlAddr = reinterpret_cast<uintptr_t>(pLevel);
            CObject* pActor = nullptr;
            if (Core::Memory::SafeRead(lvlAddr + 0x20, pActor))
            {
                if (pActor && !IsBadReadPtr(pActor, 0x100))
                    return pActor;
            }
            return nullptr;
        }

        static xr_vector<CObject*>* GetActiveObjects()
        {
            CLevel* pLevel = GetLevel();
            if (!pLevel || IsBadReadPtr(pLevel, sizeof(void*))) return nullptr;

            uintptr_t lvlAddr = reinterpret_cast<uintptr_t>(pLevel);
            uintptr_t objList = lvlAddr + 0x54;
            if (IsBadReadPtr(reinterpret_cast<void*>(objList), 0x40)) return nullptr;

            xr_vector<CObject*>* pVec = reinterpret_cast<xr_vector<CObject*>*>(objList + 0x1C);
            if (IsBadReadPtr(pVec, sizeof(xr_vector<CObject*>))) return nullptr;

            return pVec;
        }

        static xr_vector<CObject*>* GetSleepingObjects()
        {
            CLevel* pLevel = GetLevel();
            if (!pLevel || IsBadReadPtr(pLevel, sizeof(void*))) return nullptr;

            uintptr_t lvlAddr = reinterpret_cast<uintptr_t>(pLevel);
            uintptr_t objList = lvlAddr + 0x54;
            if (IsBadReadPtr(reinterpret_cast<void*>(objList), 0x40)) return nullptr;

            xr_vector<CObject*>* pVec = reinterpret_cast<xr_vector<CObject*>*>(objList + 0x2C);
            if (IsBadReadPtr(pVec, sizeof(xr_vector<CObject*>))) return nullptr;

            return pVec;
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

        static Fvector GetCameraDirection()
        {
            CRenderDevice* pDev = GetDevice();
            if (!pDev) return Fvector(0, 0, 1);

            uintptr_t devAddr = reinterpret_cast<uintptr_t>(pDev);
            Fvector camDir;
            if (Core::Memory::SafeRead(devAddr + 0x218, camDir))
                return camDir;

            return Fvector(0, 0, 1);
        }

        static Fvector GetCameraRight()
        {
            CRenderDevice* pDev = GetDevice();
            if (!pDev) return Fvector(1, 0, 0);

            uintptr_t devAddr = reinterpret_cast<uintptr_t>(pDev);
            Fvector camRight;
            if (Core::Memory::SafeRead(devAddr + 0x230, camRight))
                return camRight;

            return Fvector(1, 0, 0);
        }

        static bool TeleportActor(const Fvector& newPos)
        {
            CObject* pActor = GetCurrentEntity();
            if (!pActor) return false;

            __try
            {
                uintptr_t actorAddr = reinterpret_cast<uintptr_t>(pActor);

                // 1. Set render_transform position matrix (_41, _42, _43)
                *reinterpret_cast<float*>(actorAddr + 0x80) = newPos.x;
                *reinterpret_cast<float*>(actorAddr + 0x84) = newPos.y;
                *reinterpret_cast<float*>(actorAddr + 0x88) = newPos.z;

                // 2. Set physics movement controller position if available
                uintptr_t physSupport = *reinterpret_cast<uintptr_t*>(actorAddr + 0x24C);
                if (physSupport && !IsBadReadPtr(reinterpret_cast<void*>(physSupport), 0x20))
                {
                    uintptr_t mov = *reinterpret_cast<uintptr_t*>(physSupport + 0x08);
                    if (mov && !IsBadReadPtr(reinterpret_cast<void*>(mov), 0x30))
                    {
                        *reinterpret_cast<Fvector*>(mov + 0x10) = newPos;
                        *reinterpret_cast<Fvector*>(mov + 0x1C) = Fvector(0, 0, 0); // clear velocity
                    }
                }
                return true;
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                return false;
            }
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

    namespace Script
    {
        typedef int (*luaL_loadbuffer_t)(void* L, const char* buff, size_t sz, const char* name);
        typedef int (*lua_pcall_t)(void* L, int nargs, int nresults, int errfunc);
        typedef const char* (*lua_tolstring_t)(void* L, int idx, size_t* len);
        typedef void (*lua_settop_t)(void* L, int idx);

        inline bool Execute(const char* luaCode)
        {
            if (!luaCode || !*luaCode) return false;

            HMODULE hXrGame = GetModuleHandleA("xrGame.dll");
            HMODULE hXrLua = GetModuleHandleA("xrLua.dll");
            if (!hXrGame || !hXrLua) return false;

            __try
            {
                uintptr_t baseGame = reinterpret_cast<uintptr_t>(hXrGame);
                uintptr_t ai_space = *reinterpret_cast<uintptr_t*>(baseGame + 0x5602B0);
                if (!ai_space || IsBadReadPtr(reinterpret_cast<void*>(ai_space), 0x30)) return false;

                uintptr_t script_engine = *reinterpret_cast<uintptr_t*>(ai_space + 0x20);
                if (!script_engine || IsBadReadPtr(reinterpret_cast<void*>(script_engine), 0x10)) return false;

                void* L = *reinterpret_cast<void**>(script_engine + 0x04);
                if (!L) return false;

                static auto fnLoadBuffer = reinterpret_cast<luaL_loadbuffer_t>(GetProcAddress(hXrLua, "luaL_loadbuffer"));
                static auto fnPCall = reinterpret_cast<lua_pcall_t>(GetProcAddress(hXrLua, "lua_pcall"));
                static auto fnToString = reinterpret_cast<lua_tolstring_t>(GetProcAddress(hXrLua, "lua_tolstring"));
                static auto fnSetTop = reinterpret_cast<lua_settop_t>(GetProcAddress(hXrLua, "lua_settop"));

                if (!fnLoadBuffer || !fnPCall) return false;

                int loadRes = fnLoadBuffer(L, luaCode, strlen(luaCode), "@CheatZoneLord");
                if (loadRes != 0)
                {
                    if (fnToString)
                    {
                        const char* err = fnToString(L, -1, nullptr);
                        Core::Logger::Error("ZoneLord Lua Load Error: %s", err ? err : "unknown");
                    }
                    if (fnSetTop) fnSetTop(L, -2);
                    return false;
                }

                int pcallRes = fnPCall(L, 0, 0, 0);
                if (pcallRes != 0)
                {
                    if (fnToString)
                    {
                        const char* err = fnToString(L, -1, nullptr);
                        Core::Logger::Error("ZoneLord Lua Runtime Error: %s", err ? err : "unknown");
                    }
                    if (fnSetTop) fnSetTop(L, -2);
                    return false;
                }

                return true;
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                return false;
            }
        }

        inline void SpawnItem(const char* section, int count = 1, bool toInventory = true)
        {
            if (!section || !*section) return;
            if (count < 1) count = 1;

            char script[512];
            if (toInventory)
            {
                snprintf(script, sizeof(script),
                    "local a=db.actor if a and alife() then for i=1,%d do alife():create(\"%s\",a:position(),a:level_vertex_id(),a:game_vertex_id(),a:id()) end end",
                    count, section);
            }
            else
            {
                snprintf(script, sizeof(script),
                    "local a=db.actor if a and alife() then local d=a:direction() local p=a:position() p.x=p.x+d.x*2.0; p.y=p.y+d.y*2.0+0.3; p.z=p.z+d.z*2.0 for i=1,%d do alife():create(\"%s\",p,a:level_vertex_id(),a:game_vertex_id()) end end",
                    count, section);
            }
            Execute(script);
            Core::Logger::Success("Spawned %d x [%s] (To inventory: %s)", count, section, toInventory ? "yes" : "no");
        }

        inline void SpawnEntity(const char* section, int count = 1, float distance = 4.0f)
        {
            if (!section || !*section) return;
            if (count < 1) count = 1;
            if (distance < 1.0f) distance = 1.0f;

            char script[512];
            snprintf(script, sizeof(script),
                "local a=db.actor if a and alife() then local d=a:direction() local p=a:position() p.x=p.x+d.x*%.1f; p.y=p.y+d.y*%.1f+0.4; p.z=p.z+d.z*%.1f for i=1,%d do alife():create(\"%s\",p,a:level_vertex_id(),a:game_vertex_id()) end end",
                distance, distance, distance, count, section);
            Execute(script);
            Core::Logger::Success("Spawned %d x [%s] (Distance: %.1fm)", count, section, distance);
        }

        inline void GiveMoney(int amount)
        {
            char script[256];
            snprintf(script, sizeof(script),
                "local a=db.actor if a then a:give_money(%d) end", amount);
            Execute(script);
            Core::Logger::Success("Added %d RU to Actor money", amount);
        }

        inline void Teleport(float x, float y, float z)
        {
            char script[256];
            snprintf(script, sizeof(script),
                "local a=db.actor if a then a:set_actor_position(vector():set(%.2f,%.2f,%.2f)) end",
                x, y, z);
            Execute(script);
            Core::Logger::Success("Teleported Actor to (%.2f, %.2f, %.2f)", x, y, z);
        }

        inline void SetWeather(const char* weatherName)
        {
            if (!weatherName || !*weatherName) return;
            char script[256];
            snprintf(script, sizeof(script), "level.set_weather(\"%s\", true)", weatherName);
            Execute(script);
            Core::Logger::Success("Changed Level Weather to [%s]", weatherName);
        }

        inline void SetTimeFactor(float factor)
        {
            if (factor < 0.1f) factor = 0.1f;
            char script[256];
            snprintf(script, sizeof(script), "level.set_time_factor(%.1f)", factor);
            Execute(script);
            Core::Logger::Success("Changed Time Factor to %.1fx", factor);
        }
    }
}
