#pragma once

#include "XRaySDK.hpp"
#include "../core/Globals.hpp"
#include "../core/Memory.hpp"
#include "../core/Logger.hpp"

namespace XRay
{
#if defined(XRAY_COP)
    static constexpr uintptr_t OFF_LVL_ACTOR        = 0x20;
    static constexpr uintptr_t OFF_LVL_OBJLIST      = 0x4C;
    static constexpr uintptr_t OFF_LVL_OBJECTSPACE  = 0x40094;
    static constexpr uintptr_t OFF_OBJLIST_ACTIVE   = 0x40008;
    static constexpr uintptr_t OFF_OBJLIST_SLEEP    = 0x40014;

    static constexpr uintptr_t OFF_ACTOR_COND       = 0x26C;
    static constexpr uintptr_t OFF_ACTOR_INV        = 0x278;

    static constexpr uintptr_t OFF_COND_HP          = 0x04;
    static constexpr uintptr_t OFF_COND_HPMAX       = 0x08;
    static constexpr uintptr_t OFF_COND_POWER       = 0x54;
    static constexpr uintptr_t OFF_COND_RAD         = 0x58;
    static constexpr uintptr_t OFF_COND_PSY         = 0x5C;
    static constexpr uintptr_t OFF_COND_BLEED       = 0x60;
    static constexpr uintptr_t OFF_COND_MAXWALK     = 0x1B4;
    static constexpr uintptr_t OFF_COND_MAXWT       = 0x1B8;

    static constexpr uintptr_t OFF_ACTOR_WALKACCEL  = 0x59C;
    static constexpr uintptr_t OFF_ACTOR_JUMPSPEED  = 0x5A0;
    static constexpr uintptr_t OFF_ACTOR_RUNFACTOR  = 0x5A4;
    static constexpr uintptr_t OFF_ACTOR_SPRINT     = 0x5B8;

    static constexpr uintptr_t OFF_ACTOR_CURCAM     = 0x524;
    static constexpr uintptr_t OFF_ACTOR_CAMERAS    = 0x514;

    static constexpr uintptr_t OFF_INV_ACTIVEITEM   = 0x4C;
    static constexpr uintptr_t OFF_WEAPON_MAGSIZE   = 0x694;
    static constexpr uintptr_t OFF_WEAPON_ELAPSED   = 0x690;
#else // SoC (X-Ray 1.0006)
    static constexpr uintptr_t OFF_LVL_ACTOR        = 0x20;
    static constexpr uintptr_t OFF_LVL_OBJLIST      = 0x54;
    static constexpr uintptr_t OFF_LVL_OBJECTSPACE  = 0xCC;
    static constexpr uintptr_t OFF_OBJLIST_ACTIVE   = 0x1C;
    static constexpr uintptr_t OFF_OBJLIST_SLEEP    = 0x2C;

    static constexpr uintptr_t OFF_ACTOR_COND       = 0x220;
    static constexpr uintptr_t OFF_ACTOR_INV        = 0x298;

    static constexpr uintptr_t OFF_COND_HP          = 0x04;
    static constexpr uintptr_t OFF_COND_HPMAX       = 0x08;
    static constexpr uintptr_t OFF_COND_POWER       = 0x58;
    static constexpr uintptr_t OFF_COND_RAD         = 0x5C;
    static constexpr uintptr_t OFF_COND_PSY         = 0x60;
    static constexpr uintptr_t OFF_COND_BLEED       = 0x80;
    static constexpr uintptr_t OFF_COND_MAXWALK     = 0x138;
    static constexpr uintptr_t OFF_COND_MAXWT       = 0x13C;

    static constexpr uintptr_t OFF_ACTOR_WALKACCEL  = 0x5A0;
    static constexpr uintptr_t OFF_ACTOR_JUMPSPEED  = 0x5A4;
    static constexpr uintptr_t OFF_ACTOR_RUNFACTOR  = 0x5A8;
    static constexpr uintptr_t OFF_ACTOR_SPRINT     = 0x5BC;

    static constexpr uintptr_t OFF_ACTOR_CURCAM     = 0x530;
    static constexpr uintptr_t OFF_ACTOR_CAMERAS    = 0x524;

    static constexpr uintptr_t OFF_INV_ACTIVEITEM   = 0x4C;
    static constexpr uintptr_t OFF_WEAPON_MAGSIZE   = 0x580;
    static constexpr uintptr_t OFF_WEAPON_ELAPSED   = 0x57C;
#endif

    class Engine
    {
    public:
        static bool Initialize()
        {
            HMODULE hEngine = GetModuleHandleA("xrEngine.exe");
            if (!hEngine) hEngine = GetModuleHandleA("XR_3DA.exe");
            if (!hEngine) hEngine = GetModuleHandleA(nullptr);

            if (hEngine)
            {
                s_pDevice = reinterpret_cast<CRenderDevice*>(GetProcAddress(hEngine, "?Device@@3VCRenderDevice@@A"));
                s_ppGameLevel = reinterpret_cast<CLevel**>(GetProcAddress(hEngine, "?g_pGameLevel@@3PAVIGame_Level@@A"));

                s_fnObjList_o_count = reinterpret_cast<tCObjectList_o_count>(GetProcAddress(hEngine, "?o_count@CObjectList@@QAEIXZ"));
                s_fnObjList_o_get_by_iterator = reinterpret_cast<tCObjectList_o_get_by_iterator>(GetProcAddress(hEngine, "?o_get_by_iterator@CObjectList@@QAEPAVCObject@@I@Z"));

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
                HMODULE hEngine = GetModuleHandleA("xrEngine.exe");
                if (!hEngine) hEngine = GetModuleHandleA("XR_3DA.exe");
                if (hEngine)
                    s_pDevice = reinterpret_cast<CRenderDevice*>(GetProcAddress(hEngine, "?Device@@3VCRenderDevice@@A"));
            }
            return s_pDevice;
        }

        static CLevel* GetLevel()
        {
            if (!s_ppGameLevel)
            {
                HMODULE hEngine = GetModuleHandleA("xrEngine.exe");
                if (!hEngine) hEngine = GetModuleHandleA("XR_3DA.exe");
                if (hEngine)
                    s_ppGameLevel = reinterpret_cast<CLevel**>(GetProcAddress(hEngine, "?g_pGameLevel@@3PAVIGame_Level@@A"));
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
                if (IsBadReadPtr(reinterpret_cast<void*>(lvlAddr + OFF_LVL_OBJLIST), 0x40)) return false;

                CObject* pActor = nullptr;
                if (!Core::Memory::SafeRead(lvlAddr + OFF_LVL_ACTOR, pActor)) return false;
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
                if (!Core::Memory::SafeRead(actorAddr + OFF_ACTOR_COND, pConditions)) return false;
                if (!pConditions || IsBadReadPtr(reinterpret_cast<void*>(pConditions), 0x100)) return false;

                float hp = 0.0f;
                if (!Core::Memory::SafeRead(pConditions + OFF_COND_HP, hp)) return false;
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
                                strstr(visualLower, "human") || strstr(visualLower, "characters") ||
                                strstr(sectLower, "stalker") || strstr(sectLower, "bandit") || strstr(sectLower, "soldier") ||
                                strstr(sectLower, "military") || strstr(sectLower, "monolith") || strstr(sectLower, "merc") ||
                                strstr(sectLower, "killer") || strstr(sectLower, "dolg") || strstr(sectLower, "duty") ||
                                strstr(sectLower, "freedom") || strstr(sectLower, "zombied") || strstr(sectLower, "novice") ||
                                strstr(sectLower, "veteran") ||
                                strncmp(sectLower, "sim_default_", 12) == 0 ||
                                strncmp(sectLower, "esc_", 4) == 0 || strncmp(sectLower, "gar_", 4) == 0 ||
                                strncmp(sectLower, "agr_", 4) == 0 || strncmp(sectLower, "val_", 4) == 0 ||
                                strncmp(sectLower, "bar_", 4) == 0 || strncmp(sectLower, "mil_", 4) == 0 ||
                                strncmp(sectLower, "zat_", 4) == 0 || strncmp(sectLower, "jup_", 4) == 0 ||
                                strncmp(sectLower, "pri_", 4) == 0 || strncmp(sectLower, "pas_", 4) == 0);

                if (isHuman)
                {
                    // Enemies (Hostiles)
                    if (strstr(sectLower, "bandit") || strstr(visualLower, "bandit") || strstr(nameLower, "bandit") ||
                        strstr(sectLower, "monolith") || strstr(sectLower, "monolit") || strstr(visualLower, "monolith") || strstr(visualLower, "monolit") || strstr(nameLower, "monolith") || strstr(nameLower, "monolit") ||
                        strstr(sectLower, "merc") || strstr(sectLower, "killer") || strstr(visualLower, "merc") || strstr(visualLower, "killer") || strstr(nameLower, "killer") ||
                        strstr(sectLower, "military") || strstr(sectLower, "soldier") || strstr(visualLower, "soldier") || strstr(sectLower, "voen") ||
                        strstr(sectLower, "zombied") || strstr(visualLower, "zombied"))
                    {
                        return EntityRelation::Enemy;
                    }

                    // Neutrals (Duty & Freedom, Ecologists)
                    if (strstr(sectLower, "freedom") || strstr(sectLower, "svoboda") || strstr(visualLower, "freedom") ||
                        strstr(sectLower, "dolg") || strstr(sectLower, "duty") || strstr(visualLower, "dolg") || strstr(visualLower, "duty") ||
                        strstr(sectLower, "ecolog") || strstr(visualLower, "ecolog"))
                    {
                        return EntityRelation::Neutral;
                    }

                    // Friends / Allies (Loners, neutral stalkers, traders)
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
            if (Core::Memory::SafeRead(lvlAddr + OFF_LVL_ACTOR, pActor))
            {
                if (pActor && !IsBadReadPtr(pActor, 0x100))
                    return pActor;
            }
            return nullptr;
        }

        static void EnsureObjListMethods()
        {
            if (!s_fnObjList_o_count || !s_fnObjList_o_get_by_iterator)
            {
                HMODULE hEngine = GetModuleHandleA("xrEngine.exe");
                if (!hEngine) hEngine = GetModuleHandleA("XR_3DA.exe");
                if (!hEngine) hEngine = GetModuleHandleA(nullptr);
                if (hEngine)
                {
                    if (!s_fnObjList_o_count)
                        s_fnObjList_o_count = reinterpret_cast<tCObjectList_o_count>(GetProcAddress(hEngine, "?o_count@CObjectList@@QAEIXZ"));
                    if (!s_fnObjList_o_get_by_iterator)
                        s_fnObjList_o_get_by_iterator = reinterpret_cast<tCObjectList_o_get_by_iterator>(GetProcAddress(hEngine, "?o_get_by_iterator@CObjectList@@QAEPAVCObject@@I@Z"));
                }
            }
        }

        static xr_vector<CObject*>* GetActiveObjects()
        {
            CLevel* pLevel = GetLevel();
            if (!pLevel || IsBadReadPtr(pLevel, sizeof(void*))) return nullptr;

            uintptr_t lvlAddr = reinterpret_cast<uintptr_t>(pLevel);
            uintptr_t objList = lvlAddr + OFF_LVL_OBJLIST;
            uintptr_t vecAddr = objList + OFF_OBJLIST_ACTIVE;
            if (IsBadReadPtr(reinterpret_cast<void*>(vecAddr), sizeof(xr_vector<CObject*>))) return nullptr;

            xr_vector<CObject*>* pVec = reinterpret_cast<xr_vector<CObject*>*>(vecAddr);
            return pVec;
        }

        static xr_vector<CObject*>* GetSleepingObjects()
        {
            CLevel* pLevel = GetLevel();
            if (!pLevel || IsBadReadPtr(pLevel, sizeof(void*))) return nullptr;

            uintptr_t lvlAddr = reinterpret_cast<uintptr_t>(pLevel);
            uintptr_t objList = lvlAddr + OFF_LVL_OBJLIST;
            uintptr_t vecAddr = objList + OFF_OBJLIST_SLEEP;
            if (IsBadReadPtr(reinterpret_cast<void*>(vecAddr), sizeof(xr_vector<CObject*>))) return nullptr;

            xr_vector<CObject*>* pVec = reinterpret_cast<xr_vector<CObject*>*>(vecAddr);
            return pVec;
        }

        static uint32_t GetTotalObjectCount()
        {
            CLevel* pLevel = GetLevel();
            if (!pLevel) return 0;

            uintptr_t lvlAddr = reinterpret_cast<uintptr_t>(pLevel);
            void* objList = reinterpret_cast<void*>(lvlAddr + OFF_LVL_OBJLIST);

            EnsureObjListMethods();

            if (s_fnObjList_o_count)
            {
                __try
                {
                    uint32_t count = s_fnObjList_o_count(objList);
                    if (count < 20000) return count;
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
            void* objList = reinterpret_cast<void*>(lvlAddr + OFF_LVL_OBJLIST);

            EnsureObjListMethods();

            if (s_fnObjList_o_get_by_iterator)
            {
                __try
                {
                    CObject* pObj = s_fnObjList_o_get_by_iterator(objList, index);
                    if (pObj && !IsBadReadPtr(pObj, 0x100)) return pObj;
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
            if (!pDev || IsBadReadPtr(pDev, sizeof(CRenderDevice))) return Fmatrix();
            return pDev->mView;
        }

        static Fmatrix GetProjectMatrix()
        {
            CRenderDevice* pDev = GetDevice();
            if (!pDev || IsBadReadPtr(pDev, sizeof(CRenderDevice))) return Fmatrix();
            return pDev->mProject;
        }

        static Fmatrix GetFullTransform()
        {
            CRenderDevice* pDev = GetDevice();
            if (!pDev || IsBadReadPtr(pDev, sizeof(CRenderDevice))) return Fmatrix();
            return pDev->mFullTransform;
        }

        static Fvector GetCameraPosition()
        {
            CRenderDevice* pDev = GetDevice();
            if (!pDev || IsBadReadPtr(pDev, sizeof(CRenderDevice))) return Fvector(0, 0, 0);
            return pDev->vCameraPosition;
        }

        static Fvector GetCameraDirection()
        {
            CRenderDevice* pDev = GetDevice();
            if (!pDev || IsBadReadPtr(pDev, sizeof(CRenderDevice))) return Fvector(0, 0, 1);
            return pDev->vCameraDirection;
        }

        static Fvector GetCameraRight()
        {
            CRenderDevice* pDev = GetDevice();
            if (!pDev || IsBadReadPtr(pDev, sizeof(CRenderDevice))) return Fvector(1, 0, 0);
            return pDev->vCameraRight;
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
            void* pObjectSpace = reinterpret_cast<void*>(lvlAddr + OFF_LVL_OBJECTSPACE);
            if (!pObjectSpace || IsBadReadPtr(pObjectSpace, sizeof(void*)))
                return true;

            if (!s_fnRayTest)
            {
                HMODULE hCDB = GetModuleHandleA("xrCDB.dll");
                if (hCDB)
                {
                    s_fnRayTest = reinterpret_cast<tRayTest>(GetProcAddress(hCDB,
                        "?RayTest@CObjectSpace@@QAEHABU?$_vector3@M@@0MW4rq_target@collide@@PAUray_cache@4@PAVCObject@@@Z"));
                }
                if (!s_fnRayTest)
                {
                    HMODULE hEngine = GetModuleHandleA("xrEngine.exe");
                    if (!hEngine) hEngine = GetModuleHandleA("XR_3DA.exe");
                    if (hEngine)
                    {
                        s_fnRayTest = reinterpret_cast<tRayTest>(GetProcAddress(hEngine,
                            "?RayTest@CObjectSpace@@QAEHABU?$_vector3@M@@0MW4rq_target@collide@@PAUray_cache@4@PAVCObject@@@Z"));
#if !defined(XRAY_COP)
                        if (!s_fnRayTest)
                            s_fnRayTest = reinterpret_cast<tRayTest>(reinterpret_cast<uintptr_t>(hEngine) + 0x4E300);
#endif
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

#if defined(XRAY_COP)
                typedef void* (__stdcall* t_dcast_PKinematics)(void* thisPtr);
                auto dcast = reinterpret_cast<t_dcast_PKinematics>(vtbl[3]);
#else
                typedef void* (__thiscall* t_dcast_PKinematics)(void* thisPtr);
                auto dcast = reinterpret_cast<t_dcast_PKinematics>(vtbl[6]);
#endif
                void* pKinematics = dcast(reinterpret_cast<void*>(visual));
                if (!pKinematics || IsBadReadPtr(pKinematics, 0x40))
                    return false;

                auto k = reinterpret_cast<CKinematics*>(pKinematics);
                CBoneInstance* boneInstances = k->bone_instances;
                if (!boneInstances || IsBadReadPtr(boneInstances, sizeof(CBoneInstance)))
                    return false;

                const char* targetName = "bip01_head";
                if (boneTargetType == 1) targetName = "bip01_neck";
                else if (boneTargetType == 2) targetName = "bip01_spine2";
                else if (boneTargetType == 3) targetName = "bip01_pelvis";

                uint16_t boneId = 0xFFFF;
                size_t count = k->GetBoneCount();

                // 1. Direct name match in bone_datas
                if (count > 0 && count <= 128)
                {
                    for (size_t i = 0; i < count; ++i)
                    {
                        CBoneData* pData = k->GetBoneData(i);
                        if (!pData || IsBadReadPtr(pData, sizeof(CBoneData))) continue;
#if defined(XRAY_COP)
                        const char* bName = SafeReadSharedStr(reinterpret_cast<uintptr_t>(&pData->name));
#else
                        const char* bName = SafeReadSharedStr(reinterpret_cast<uintptr_t>(pData) + 0x08);
#endif
                        if (bName && _stricmp(bName, targetName) == 0)
                        {
                            boneId = static_cast<uint16_t>(i);
                            break;
                        }
                    }
                }

                // 2. Engine LL_BoneID lookup
                if (boneId == 0xFFFF)
                {
                    if (!s_fnLL_BoneID)
                    {
                        HMODULE hEngine = GetModuleHandleA("XR_3DA.exe");
                        if (!hEngine) hEngine = GetModuleHandleA("xrEngine.exe");
                        if (hEngine)
                        {
                            s_fnLL_BoneID = reinterpret_cast<tLL_BoneID>(GetProcAddress(hEngine,
                                "?LL_BoneID@CKinematics@@QAEGPBD@Z"));
#if !defined(XRAY_COP)
                            if (!s_fnLL_BoneID)
                                s_fnLL_BoneID = reinterpret_cast<tLL_BoneID>(reinterpret_cast<uintptr_t>(hEngine) + 0x6C6E0);
#endif
                        }
                    }

                    if (s_fnLL_BoneID)
                    {
                        boneId = s_fnLL_BoneID(pKinematics, targetName);
                        if (boneId == 0xFFFF)
                        {
                            if (boneTargetType <= 1) boneId = s_fnLL_BoneID(pKinematics, "head");
                            if (boneId == 0xFFFF) boneId = s_fnLL_BoneID(pKinematics, "bip01_spine1");
                            if (boneId == 0xFFFF) boneId = s_fnLL_BoneID(pKinematics, "bip01_spine");
                        }
                    }
                }

                if (boneId != 0xFFFF && boneId < 128)
                {
                    float lx = boneInstances[boneId].mTransform._41;
                    float ly = boneInstances[boneId].mTransform._42;
                    float lz = boneInstances[boneId].mTransform._43;

                    Fvector localBone(lx, ly, lz);
                    pObj->render_transform.Transform(outWorldPos, localBone);
                    return true;
                }

                // 3. Fallback: find highest bone in local Y (head)
                if (count > 0 && count <= 128)
                {
                    size_t bestIdx = 0;
                    float maxLy = -1e6f;
                    for (size_t i = 0; i < count; ++i)
                    {
                        float ly = boneInstances[i].mTransform._42;
                        if (ly > maxLy)
                        {
                            maxLy = ly;
                            bestIdx = i;
                        }
                    }

                    float lx = boneInstances[bestIdx].mTransform._41;
                    float ly = boneInstances[bestIdx].mTransform._42;
                    float lz = boneInstances[bestIdx].mTransform._43;

                    Fvector localBone(lx, ly, lz);
                    pObj->render_transform.Transform(outWorldPos, localBone);
                    return true;
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
            HMODULE hEngine = GetModuleHandleA("xrEngine.exe");
            if (!hEngine) hEngine = GetModuleHandleA("XR_3DA.exe");
            if (!hEngine) return 0;

            static void** ppPersistent = nullptr;
            if (!ppPersistent)
            {
                ppPersistent = reinterpret_cast<void**>(GetProcAddress(hEngine, "?g_pGamePersistent@@3PAVIGame_Persistent@@A"));
#if !defined(XRAY_COP)
                if (!ppPersistent)
                    ppPersistent = reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(hEngine) + 0x10BB7C);
#endif
            }

            if (!ppPersistent || !*ppPersistent || IsBadReadPtr(*ppPersistent, sizeof(void*)))
                return 0;

            uintptr_t pPersistent = reinterpret_cast<uintptr_t>(*ppPersistent);
            if (IsBadReadPtr(reinterpret_cast<void*>(pPersistent), 0x500))
                return 0;

            uintptr_t pEnv = *reinterpret_cast<uintptr_t*>(pPersistent + 0x46C);
            if (!pEnv || IsBadReadPtr(reinterpret_cast<void*>(pEnv), 0x100))
            {
                pEnv = *reinterpret_cast<uintptr_t*>(pPersistent + 0x474);
            }
            if (!pEnv || IsBadReadPtr(reinterpret_cast<void*>(pEnv), 0x100))
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
                HMODULE hEngine = GetModuleHandleA("xrEngine.exe");
                if (!hEngine) hEngine = GetModuleHandleA("XR_3DA.exe");
                if (hEngine)
                {
                    s_fnSetGameTime = reinterpret_cast<tSetGameTime>(GetProcAddress(hEngine,
                        "?SetGameTime@CEnvironment@@QAEXMM@Z"));
#if !defined(XRAY_COP)
                    if (!s_fnSetGameTime)
                        s_fnSetGameTime = reinterpret_cast<tSetGameTime>(reinterpret_cast<uintptr_t>(hEngine) + 0x38730);
#endif
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
            HMODULE hXrLua = GetModuleHandleA("lua.JIT.1.1.4.dll");
            if (!hXrLua) hXrLua = GetModuleHandleA("xrLua.dll");
            if (!hXrGame || !hXrLua) return false;

            __try
            {
                uintptr_t baseGame = reinterpret_cast<uintptr_t>(hXrGame);
#if defined(XRAY_COP)
                uintptr_t ai_space = *reinterpret_cast<uintptr_t*>(baseGame + 0x64DA98);
#else
                uintptr_t ai_space = *reinterpret_cast<uintptr_t*>(baseGame + 0x5602B0);
#endif
                if (!ai_space || IsBadReadPtr(reinterpret_cast<void*>(ai_space), 0x30)) return false;

#if defined(XRAY_COP)
                uintptr_t script_engine = *reinterpret_cast<uintptr_t*>(ai_space + 0x1C);
#else
                uintptr_t script_engine = *reinterpret_cast<uintptr_t*>(ai_space + 0x20);
#endif
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
