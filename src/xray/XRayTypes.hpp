#pragma once

#include <cmath>
#include <cstdint>

namespace XRay
{
    // Four-character class ID macro used by X-Ray Engine
    #define MK_CLSID(a, b, c, d) \
        ((uint32_t)(uint8_t)(a) | ((uint32_t)(uint8_t)(b) << 8) | \
        ((uint32_t)(uint8_t)(c) << 16) | ((uint32_t)(uint8_t)(d) << 24))

    enum ClassID : uint32_t
    {
        CLSID_OBJECT_ACTOR      = MK_CLSID('O', '_', 'A', 'C'), // Actor (Player)
        CLSID_AI_STALKER        = MK_CLSID('A', 'I', '_', 'S'), // Human NPC / Stalker / Bandit
        CLSID_AI_FLESH          = MK_CLSID('S', 'M', '_', 'F'), // Flesh
        CLSID_AI_BOAR           = MK_CLSID('S', 'M', '_', 'B'), // Boar
        CLSID_AI_DOG            = MK_CLSID('S', 'M', '_', 'D'), // Blind Dog
        CLSID_AI_PSEUDODOG      = MK_CLSID('S', 'M', '_', 'P'), // Pseudodog
        CLSID_AI_BLOODSUCKER    = MK_CLSID('S', 'M', '_', 'B'), // Bloodsucker
        CLSID_AI_SNORK          = MK_CLSID('S', 'M', '_', 'S'), // Snork
        CLSID_AI_CONTROLLER     = MK_CLSID('S', 'M', '_', 'C'), // Controller
        CLSID_AI_POLTERGEIST    = MK_CLSID('S', 'M', '_', 'G'), // Poltergeist
        CLSID_AI_GIANT          = MK_CLSID('S', 'M', '_', 'Z'), // Pseudogiant
        CLSID_AI_ZOMBIE         = MK_CLSID('S', 'M', '_', 'E'), // Zombie
        CLSID_ARTIFACT          = MK_CLSID('A', 'R', 'T', 'E'), // Artifact
        CLSID_ANOMALY           = MK_CLSID('Z', '_', 'A', 'N')  // Anomaly zone
    };

    // 2D Vector
    struct Fvector2
    {
        float x, y;

        Fvector2() : x(0.f), y(0.f) {}
        Fvector2(float _x, float _y) : x(_x), y(_y) {}
    };

    // 3D Vector matching X-Ray Fvector
    struct Fvector
    {
        float x, y, z;

        Fvector() : x(0.f), y(0.f), z(0.f) {}
        Fvector(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

        inline Fvector operator+(const Fvector& v) const { return Fvector(x + v.x, y + v.y, z + v.z); }
        inline Fvector operator-(const Fvector& v) const { return Fvector(x - v.x, y - v.y, z - v.z); }
        inline Fvector operator*(float s) const { return Fvector(x * s, y * s, z * s); }
        inline Fvector operator/(float s) const { float inv = 1.0f / s; return Fvector(x * inv, y * inv, z * inv); }

        inline Fvector& operator+=(const Fvector& v) { x += v.x; y += v.y; z += v.z; return *this; }
        inline Fvector& operator-=(const Fvector& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }

        inline float Dot(const Fvector& v) const { return x * v.x + y * v.y + z * v.z; }

        inline float MagnitudeSq() const { return x * x + y * y + z * z; }
        inline float Magnitude() const { return std::sqrt(MagnitudeSq()); }

        inline float DistanceTo(const Fvector& v) const
        {
            float dx = x - v.x;
            float dy = y - v.y;
            float dz = z - v.z;
            return std::sqrt(dx * dx + dy * dy + dz * dz);
        }

        inline Fvector Normalized() const
        {
            float len = Magnitude();
            if (len > 0.0001f)
            {
                float inv = 1.0f / len;
                return Fvector(x * inv, y * inv, z * inv);
            }
            return Fvector(0.f, 0.f, 0.f);
        }
    };

    // 4D Vector
    struct Fvector4
    {
        float x, y, z, w;

        Fvector4() : x(0.f), y(0.f), z(0.f), w(0.f) {}
        Fvector4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
    };

    // 4x4 Matrix matching X-Ray Fmatrix (Row-major orientation)
    struct Fmatrix
    {
        union
        {
            struct
            {
                float _11, _12, _13, _14;
                float _21, _22, _23, _24;
                float _31, _32, _33, _34;
                float _41, _42, _43, _44;
            };
            float m[4][4];
        };

        Fmatrix()
        {
            Identity();
        }

        inline void Identity()
        {
            _11 = 1.0f; _12 = 0.0f; _13 = 0.0f; _14 = 0.0f;
            _21 = 0.0f; _22 = 1.0f; _23 = 0.0f; _24 = 0.0f;
            _31 = 0.0f; _32 = 0.0f; _33 = 1.0f; _34 = 0.0f;
            _41 = 0.0f; _42 = 0.0f; _43 = 0.0f; _44 = 1.0f;
        }

        // Multiply matrix by matrix
        inline Fmatrix operator*(const Fmatrix& r) const
        {
            Fmatrix res;
            for (int i = 0; i < 4; ++i)
            {
                for (int j = 0; j < 4; ++j)
                {
                    res.m[i][j] = m[i][0] * r.m[0][j] +
                                  m[i][1] * r.m[1][j] +
                                  m[i][2] * r.m[2][j] +
                                  m[i][3] * r.m[3][j];
                }
            }
            return res;
        }

        // Transform 3D point (X-Ray standard vector transformation)
        inline void Transform(Fvector& dest, const Fvector& v) const
        {
            dest.x = v.x * _11 + v.y * _21 + v.z * _31 + _41;
            dest.y = v.x * _12 + v.y * _22 + v.z * _32 + _42;
            dest.z = v.x * _13 + v.y * _23 + v.z * _33 + _43;
        }

        // Transform point with projection W-divide
        inline bool TransformProject(Fvector& dest, const Fvector& v) const
        {
            float w = v.x * _14 + v.y * _24 + v.z * _34 + _44;
            if (w <= 0.001f)
                return false;

            float invW = 1.0f / w;
            dest.x = (v.x * _11 + v.y * _21 + v.z * _31 + _41) * invW;
            dest.y = (v.x * _12 + v.y * _22 + v.z * _32 + _42) * invW;
            dest.z = (v.x * _13 + v.y * _23 + v.z * _33 + _43) * invW;
            return true;
        }
    };
}
