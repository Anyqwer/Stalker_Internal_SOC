#pragma once

#include "XRayTypes.hpp"
#include <windows.h>
#include <string>
#include <vector>

namespace XRay
{
    #pragma pack(push, 1)

    // X-Ray STL vector representation: xr_vector<T> (STLPort 5.0 layout on 32-bit x86)
    template <typename T>
    struct xr_vector
    {
        uint32_t _allocator_pad; // 0x00: Allocator proxy / pad (4 bytes)
        T* _first;               // 0x04: _M_start
        T* _last;                // 0x08: _M_finish
        T* _end;                 // 0x0C: _M_end_of_storage

        size_t size() const
        {
            __try
            {
                if (!_first || !_last || _last < _first) return 0;
                if (IsBadReadPtr(_first, sizeof(T))) return 0;
                size_t count = static_cast<size_t>(_last - _first);
                return (count > 20000) ? 0 : count;
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                return 0;
            }
        }

        T at(size_t index) const
        {
            __try
            {
                if (index >= size()) return nullptr;
                if (IsBadReadPtr(_first + index, sizeof(T))) return nullptr;
                return _first[index];
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                return nullptr;
            }
        }

        bool empty() const { return size() == 0; }
    };

    // Forward declarations
    struct CObject;
    struct CEntityAlive;
    struct CActor;

    enum class EntityRelation
    {
        Unknown,
        Friend,    // Loners, Allies, Traders
        Neutral,   // Duty, Freedom, Ecologists
        Enemy,     // Bandits, Monolith, Mercenaries, Military, Zombied
        Mutant     // Monsters
    };

    // Helper function to safely read X-Ray shared_str
    static inline const char* SafeReadSharedStr(uintptr_t pSharedStr)
    {
        __try
        {
            if (!pSharedStr || IsBadReadPtr(reinterpret_cast<void*>(pSharedStr), sizeof(uintptr_t)))
                return "";

            uintptr_t pStrVal = *reinterpret_cast<uintptr_t*>(pSharedStr);
#if defined(XRAY_COP)
            if (!pStrVal || IsBadReadPtr(reinterpret_cast<void*>(pStrVal + 0x10), 1))
                return "";

            return reinterpret_cast<const char*>(pStrVal + 0x10);
#else
            if (!pStrVal || IsBadReadPtr(reinterpret_cast<void*>(pStrVal + 0x0C), 1))
                return "";

            return reinterpret_cast<const char*>(pStrVal + 0x0C);
#endif
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            return "";
        }
    }

    // CRenderDevice: encapsulates viewport, projection, and camera matrices
#if defined(XRAY_COP)
    struct CRenderDevice
    {
        void* vfptr;                    // 0x00: vtable
        uint32_t dwWidth;               // 0x04: Screen width
        uint32_t dwHeight;              // 0x08: Screen height
        uint32_t dwPrecacheFrame;       // 0x0C
        uint32_t b_is_Ready;            // 0x10
        uint32_t b_is_Active;           // 0x14
        float fWidth_2;                 // 0x18
        float fHeight_2;                // 0x1C
        uint8_t pad_0020[0x10];         // 0x20 -> 0x30
        Fvector vCameraPosition;        // 0x30
        Fvector vCameraDirection;       // 0x3C
        Fvector vCameraTop;             // 0x48
        Fvector vCameraRight;           // 0x54
        Fmatrix mView;                  // 0x60
        Fmatrix mProject;               // 0xA0
        Fmatrix mFullTransform;         // 0xE0
        uint8_t pad_0120[0xCC];         // 0x120 -> 0x1EC
        float fFOV;                     // 0x1EC
        float fASPECT;                  // 0x1F0
    };

    struct CBoneInstance
    {
        Fmatrix mTransform;             // 0x00: Local bone transformation matrix (64 bytes)
        uint8_t pad_bone[0x60];         // 0x40 -> 0xA0 (Total sizeof = 160 bytes = 0xA0)
    };

    struct CBoneData
    {
        void* vfptr;                    // 0x00: vtable
        uint16_t bone_id;               // 0x04: self id
        uint16_t parent_id;             // 0x06: parent bone ID (0xFFFF = root / no parent)
        uint32_t pad08;                 // 0x08: parent pointer / pad
        uintptr_t name;                 // 0x0C: shared_str
    };

    struct CKinematics
    {
        void* vfptr;                    // 0x00: Virtual method table
        uint8_t pad04[0x24];            // 0x04 -> 0x28
        CBoneInstance* bone_instances;  // 0x28: Array of bone instances
        void* pBoneDatas;               // 0x2C: Pointer to vector (_first, _last, _end)
        uint32_t pad30;                 // 0x30
        uint16_t bone_count;            // 0x34: Direct verified count!

        inline size_t GetBoneCount() const
        {
            __try
            {
                if (IsBadReadPtr(this, 0x40)) return 0;
                if (bone_count > 0 && bone_count <= 128) return static_cast<size_t>(bone_count);
                if (!pBoneDatas || IsBadReadPtr(pBoneDatas, 8)) return 0;
                uintptr_t* pVec = reinterpret_cast<uintptr_t*>(pBoneDatas);
                if (!pVec[0] || !pVec[1] || pVec[1] < pVec[0]) return 0;
                size_t cnt = (pVec[1] - pVec[0]) / 4;
                return (cnt <= 128) ? cnt : 0;
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                return 0;
            }
        }

        inline CBoneData* GetBoneData(size_t index) const
        {
            __try
            {
                if (index >= GetBoneCount()) return nullptr;
                if (!pBoneDatas || IsBadReadPtr(pBoneDatas, 8)) return nullptr;
                CBoneData** pStart = *reinterpret_cast<CBoneData***>(pBoneDatas);
                if (!pStart || IsBadReadPtr(pStart + index, sizeof(CBoneData*))) return nullptr;
                return pStart[index];
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                return nullptr;
            }
        }
    };
#else
    struct CRenderDevice
    {
        uint8_t pad_0000[0x100];
        uint32_t dwWidth;               // 0x100
        uint32_t dwHeight;              // 0x104
        float fWidth_2;                 // 0x108
        float fHeight_2;                // 0x10C
        float fFOV;                     // 0x110
        float fASPECT;                  // 0x114
        uint8_t pad_0118[0xF4];         // 0x118 -> 0x20C
        Fvector vCameraPosition;        // 0x20C
        Fvector vCameraDirection;       // 0x218
        Fvector vCameraTop;             // 0x224
        Fvector vCameraRight;           // 0x230
        Fmatrix mView;                  // 0x23C
        Fmatrix mProject;               // 0x27C
        Fmatrix mFullTransform;         // 0x2BC
    };

    struct CBoneInstance
    {
        Fmatrix mTransform;             // 0x00: Local bone transformation matrix (64 bytes)
        uint8_t pad_bone[0x60];         // 0x40 -> 0xA0 (Total sizeof = 160 bytes)
    };

    struct CBoneData
    {
        uint16_t bone_id;               // 0x00
        uint16_t pad02;                 // 0x02
        uint16_t pad04;                 // 0x04
        uint16_t parent_id;             // 0x06: Parent bone ID (0xFFFF = root / no parent)
    };

    struct CKinematics
    {
        void* vfptr;                    // 0x00: Virtual method table
        uint8_t pad04[0x80];            // 0x04 -> 0x84
        CBoneInstance* bone_instances;  // 0x84: Array of bone instances
        xr_vector<CBoneData*> bone_datas; // 0x88: Vector of bone descriptors

        inline size_t GetBoneCount() const
        {
            return bone_datas.size();
        }

        inline CBoneData* GetBoneData(size_t index) const
        {
            return bone_datas.at(index);
        }
    };
#endif

    // Basic game object in X-Ray (1.0006 retail verified layout)
    struct CObject
    {
        void* vfptr;                    // 0x00: Virtual method table
        uint32_t pad_0004;              // 0x04:
        uint64_t cls_id;                // 0x08: CLS_ID (64-bit)
        uint8_t pad_0010[0x40];         // 0x10 -> 0x50
        Fmatrix render_transform;       // 0x50: Object transformation matrix (XFORM)
        void* pVisual;                  // 0x90: IRender_Visual pointer
        uint8_t pad_0094[0x10];         // 0x94 -> 0xA4
        uint16_t ID;                    // 0xA4: Object net ID
        uint16_t ID_pad2;               // 0xA6:
        uintptr_t cName;                // 0xA8: shared_str (name)
        uintptr_t cNameSect;            // 0xAC: shared_str (section)
        uintptr_t cNameVisual;          // 0xB0: shared_str (visual)

        inline Fvector GetPosition() const
        {
            return Fvector(render_transform._41, render_transform._42, render_transform._43);
        }

        inline const char* GetName() const
        {
            return SafeReadSharedStr(reinterpret_cast<uintptr_t>(&cName));
        }

        inline const char* GetSection() const
        {
            return SafeReadSharedStr(reinterpret_cast<uintptr_t>(&cNameSect));
        }

        inline const char* GetVisual() const
        {
            return SafeReadSharedStr(reinterpret_cast<uintptr_t>(&cNameVisual));
        }
    };

    // Entity alive (Stalkers, Mutants, Player)
    struct CEntityAlive : public CObject
    {
        uint8_t pad_entity[0x100];
        float fHealth;                  // Current entity health
    };

    // Forward declare Level
    struct CLevel;

    #pragma pack(pop)
}
