#include "Visuals.hpp"
#include "../core/Config.hpp"
#include "../core/Logger.hpp"
#include "../xray/Offsets.hpp"
#include "../xray/WorldToScreen.hpp"
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <cctype>
#include <cmath>

namespace Features
{
    void Visuals::DrawBox2D(float x, float y, float w, float h, ImU32 color, bool bCornerOnly)
    {
        ImDrawList* pDrawList = ImGui::GetBackgroundDrawList();
        ImU32 outline = IM_COL32(0, 0, 0, 220);

        if (bCornerOnly)
        {
            float lineW = w / 4.0f;
            float lineH = h / 4.0f;

            // Top-left
            pDrawList->AddLine(ImVec2(x - 1, y), ImVec2(x + lineW, y), outline, 3.0f);
            pDrawList->AddLine(ImVec2(x, y - 1), ImVec2(x, y + lineH), outline, 3.0f);
            pDrawList->AddLine(ImVec2(x, y), ImVec2(x + lineW, y), color, 1.5f);
            pDrawList->AddLine(ImVec2(x, y), ImVec2(x, y + lineH), color, 1.5f);

            // Top-right
            pDrawList->AddLine(ImVec2(x + w - lineW, y), ImVec2(x + w + 1, y), outline, 3.0f);
            pDrawList->AddLine(ImVec2(x + w, y - 1), ImVec2(x + w, y + lineH), outline, 3.0f);
            pDrawList->AddLine(ImVec2(x + w - lineW, y), ImVec2(x + w, y), color, 1.5f);
            pDrawList->AddLine(ImVec2(x + w, y), ImVec2(x + w, y + lineH), color, 1.5f);

            // Bottom-left
            pDrawList->AddLine(ImVec2(x - 1, y + h), ImVec2(x + lineW, y + h), outline, 3.0f);
            pDrawList->AddLine(ImVec2(x, y + h - lineH), ImVec2(x, y + h + 1), outline, 3.0f);
            pDrawList->AddLine(ImVec2(x, y + h), ImVec2(x + lineW, y + h), color, 1.5f);
            pDrawList->AddLine(ImVec2(x, y + h - lineH), ImVec2(x, y + h), color, 1.5f);

            // Bottom-right
            pDrawList->AddLine(ImVec2(x + w - lineW, y + h), ImVec2(x + w + 1, y + h), outline, 3.0f);
            pDrawList->AddLine(ImVec2(x + w, y + h - lineH), ImVec2(x + w, y + h + 1), outline, 3.0f);
            pDrawList->AddLine(ImVec2(x + w - lineW, y + h), ImVec2(x + w, y + h), color, 1.5f);
            pDrawList->AddLine(ImVec2(x + w, y + h - lineH), ImVec2(x + w, y + h), color, 1.5f);
        }
        else
        {
            // Full box with black outlines for high contrast
            pDrawList->AddRect(ImVec2(x - 1, y - 1), ImVec2(x + w + 1, y + h + 1), outline, 0.0f, 0, 1.5f);
            pDrawList->AddRect(ImVec2(x + 1, y + 1), ImVec2(x + w - 1, y + h - 1), outline, 0.0f, 0, 1.5f);
            pDrawList->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), color, 0.0f, 0, 1.5f);
        }
    }

    void Visuals::DrawHealthBar(float x, float y, float h, float health01)
    {
        ImDrawList* pDrawList = ImGui::GetBackgroundDrawList();

        float barW = 3.0f;
        float barX = x - barW - 4.0f;
        float barH = h * std::clamp(health01, 0.0f, 1.0f);

        // Black background
        pDrawList->AddRectFilled(ImVec2(barX - 1, y - 1), ImVec2(barX + barW + 1, y + h + 1), IM_COL32(0, 0, 0, 220));

        // Green to Red gradient color based on health
        ImU32 barCol = IM_COL32(
            static_cast<int>((1.0f - health01) * 255.0f),
            static_cast<int>(health01 * 255.0f),
            0,
            255
        );

        pDrawList->AddRectFilled(ImVec2(barX, y + h - barH), ImVec2(barX + barW, y + h), barCol);
    }

    void Visuals::DrawSnapline(const XRay::Fvector2& screenPos, ImU32 color, int snaplinePos)
    {
        ImDrawList* pDrawList = ImGui::GetBackgroundDrawList();
        ImGuiIO& io = ImGui::GetIO();

        ImVec2 start;
        if (snaplinePos == 0) // Bottom screen
        {
            start = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y);
        }
        else // Center / Crosshair
        {
            start = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        }

        pDrawList->AddLine(start, ImVec2(screenPos.x, screenPos.y), color, 1.0f);
    }

    void Visuals::DrawFovCircle(float fovRadius, ImU32 color)
    {
        ImDrawList* pDrawList = ImGui::GetBackgroundDrawList();
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);

        pDrawList->AddCircle(center, fovRadius, color, 64, 1.2f);
    }

    void Visuals::DrawCustomCrosshair(ImU32 color)
    {
        ImDrawList* pDrawList = ImGui::GetBackgroundDrawList();
        ImGuiIO& io = ImGui::GetIO();
        float cx = io.DisplaySize.x * 0.5f;
        float cy = io.DisplaySize.y * 0.5f;

        float size = g_Config.visuals.fCrosshairSize;
        float gap = g_Config.visuals.fCrosshairGap;
        float thick = g_Config.visuals.fCrosshairThickness;
        bool bOutline = g_Config.visuals.bCrosshairOutline;
        ImU32 outlineCol = IM_COL32(0, 0, 0, 220);

        auto DrawLineThick = [&](ImVec2 p1, ImVec2 p2) {
            if (bOutline) {
                pDrawList->AddLine(p1, p2, outlineCol, thick + 2.0f);
            }
            pDrawList->AddLine(p1, p2, color, thick);
        };

        // Center Dot
        if (g_Config.visuals.bCrosshairDot)
        {
            if (bOutline) pDrawList->AddCircleFilled(ImVec2(cx, cy), thick * 0.8f + 1.2f, outlineCol);
            pDrawList->AddCircleFilled(ImVec2(cx, cy), thick * 0.8f, color);
        }

        switch (g_Config.visuals.iCrosshairStyle)
        {
        case 0: // Classic Cross (+)
            DrawLineThick(ImVec2(cx - gap - size, cy), ImVec2(cx - gap, cy));
            DrawLineThick(ImVec2(cx + gap, cy), ImVec2(cx + gap + size, cy));
            DrawLineThick(ImVec2(cx, cy - gap - size), ImVec2(cx, cy - gap));
            DrawLineThick(ImVec2(cx, cy + gap), ImVec2(cx, cy + gap + size));
            break;

        case 1: // Dot Only (.)
            if (!g_Config.visuals.bCrosshairDot)
            {
                if (bOutline) pDrawList->AddCircleFilled(ImVec2(cx, cy), size + 1.2f, outlineCol);
                pDrawList->AddCircleFilled(ImVec2(cx, cy), size, color);
            }
            break;

        case 2: // Circle (o)
            if (bOutline) pDrawList->AddCircle(ImVec2(cx, cy), gap + size, outlineCol, 32, thick + 1.8f);
            pDrawList->AddCircle(ImVec2(cx, cy), gap + size, color, 32, thick);
            break;

        case 3: // Target (Circle + Cross)
            if (bOutline) pDrawList->AddCircle(ImVec2(cx, cy), gap + size, outlineCol, 32, thick + 1.8f);
            pDrawList->AddCircle(ImVec2(cx, cy), gap + size, color, 32, thick);
            DrawLineThick(ImVec2(cx - gap - size - 4, cy), ImVec2(cx - gap, cy));
            DrawLineThick(ImVec2(cx + gap, cy), ImVec2(cx + gap + size + 4, cy));
            DrawLineThick(ImVec2(cx, cy - gap - size - 4), ImVec2(cx, cy - gap));
            DrawLineThick(ImVec2(cx, cy + gap), ImVec2(cx, cy + gap + size + 4));
            break;

        case 4: // T-Shape
            DrawLineThick(ImVec2(cx - gap - size, cy), ImVec2(cx - gap, cy));
            DrawLineThick(ImVec2(cx + gap, cy), ImVec2(cx + gap + size, cy));
            DrawLineThick(ImVec2(cx, cy + gap), ImVec2(cx, cy + gap + size));
            break;

        case 5: // CS-Style (with gap)
            DrawLineThick(ImVec2(cx - gap - size, cy), ImVec2(cx - gap, cy));
            DrawLineThick(ImVec2(cx + gap, cy), ImVec2(cx + gap + size, cy));
            DrawLineThick(ImVec2(cx, cy - gap - size), ImVec2(cx, cy - gap));
            DrawLineThick(ImVec2(cx, cy + gap), ImVec2(cx, cy + gap + size));
            break;
        }
    }

    void Visuals::DrawOffscreenArrow(const XRay::Fvector& targetPos, ImU32 color)
    {
        ImGuiIO& io = ImGui::GetIO();
        float cx = io.DisplaySize.x * 0.5f;
        float cy = io.DisplaySize.y * 0.5f;

        XRay::Fvector camPos = XRay::Engine::GetCameraPosition();
        XRay::Fvector camDir = XRay::Engine::GetCameraDirection();
        XRay::Fvector camRight = XRay::Engine::GetCameraRight();

        XRay::Fvector diff = targetPos - camPos;
        float dirX = diff.x * camRight.x + diff.y * camRight.y + diff.z * camRight.z;
        float dirZ = diff.x * camDir.x + diff.y * camDir.y + diff.z * camDir.z;

        float angle = std::atan2(dirX, dirZ);

        float radius = g_Config.rage.fArrowRadius;
        float px = cx + radius * std::sin(angle);
        float py = cy - radius * std::cos(angle);

        // Arrow head pointing outwards
        float forwardX = std::sin(angle);
        float forwardY = -std::cos(angle);
        float sideX = -forwardY;
        float sideY = forwardX;

        float arrowSize = 13.0f;
        float arrowWidth = 7.0f;

        ImVec2 pTip(px + forwardX * arrowSize, py + forwardY * arrowSize);
        ImVec2 pLeft(px - forwardX * (arrowSize * 0.5f) - sideX * arrowWidth, py - forwardY * (arrowSize * 0.5f) - sideY * arrowWidth);
        ImVec2 pRight(px - forwardX * (arrowSize * 0.5f) + sideX * arrowWidth, py - forwardY * (arrowSize * 0.5f) + sideY * arrowWidth);

        ImDrawList* pDrawList = ImGui::GetBackgroundDrawList();
        pDrawList->AddTriangleFilled(pTip, pLeft, pRight, color);
        pDrawList->AddTriangle(pTip, pLeft, pRight, IM_COL32(0, 0, 0, 200), 1.5f);
    }

    bool Visuals::DrawSkeletonAndGetBounds(XRay::CObject* pObj, ImDrawList* pDrawList, ImU32 color,
                                           float& outMinX, float& outMinY, float& outMaxX, float& outMaxY)
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

            // VMT index 6 is dcast_PKinematics (index 5 is Spawn)
            typedef void* (__thiscall* t_dcast_PKinematics)(void* thisPtr);
            auto dcast = reinterpret_cast<t_dcast_PKinematics>(vtbl[6]);
            void* pKinematics = dcast(reinterpret_cast<void*>(visual));
            if (!pKinematics || IsBadReadPtr(pKinematics, 0x90))
                return false;

            uintptr_t kAddr = reinterpret_cast<uintptr_t>(pKinematics);
            uintptr_t boneInstances = *reinterpret_cast<uintptr_t*>(kAddr + 0x84);
            if (!boneInstances || IsBadReadPtr(reinterpret_cast<void*>(boneInstances), sizeof(XRay::CBoneInstance)))
                return false;

            // In X-Ray 1.0006 CKinematics:
            // Offset 0x88 is a pointer to xr_vector<CBoneData*> (vecBones*)
            // Inside the vector, +0x04 is _first and +0x08 is _last
            uintptr_t pBoneDataVec = *reinterpret_cast<uintptr_t*>(kAddr + 0x88);
            if (!pBoneDataVec || IsBadReadPtr(reinterpret_cast<void*>(pBoneDataVec), 0x10))
                return false;

            XRay::CBoneData** boneDataFirst = *reinterpret_cast<XRay::CBoneData***>(pBoneDataVec + 0x04);
            XRay::CBoneData** boneDataLast = *reinterpret_cast<XRay::CBoneData***>(pBoneDataVec + 0x08);

            if (!boneDataFirst || !boneDataLast || boneDataLast <= boneDataFirst)
                return false;

            size_t boneCount = boneDataLast - boneDataFirst;
            if (boneCount == 0 || boneCount > 128)
                return false;

            struct ScreenBone
            {
                XRay::Fvector2 screenPos;
                bool valid = false;
            };
            ScreenBone screenBones[128];

            float minX = 1e6f, maxX = -1e6f;
            float minY = 1e6f, maxY = -1e6f;
            int validCount = 0;

            for (size_t i = 0; i < boneCount; ++i)
            {
                // Each CBoneInstance is 0xA0 (160) bytes
                // Transform matrix is at offset 0x00, position is row 4: _41, _42, _43 (offset 0x30, 0x34, 0x38)
                float lx = *reinterpret_cast<float*>(boneInstances + i * 0xA0 + 0x30);
                float ly = *reinterpret_cast<float*>(boneInstances + i * 0xA0 + 0x34);
                float lz = *reinterpret_cast<float*>(boneInstances + i * 0xA0 + 0x38);

                XRay::Fvector localBone(lx, ly, lz);
                XRay::Fvector worldBone;
                pObj->render_transform.Transform(worldBone, localBone);

                XRay::Fvector2 screenPt;
                if (XRay::Math::WorldToScreen(worldBone, screenPt))
                {
                    screenBones[i].screenPos = screenPt;
                    screenBones[i].valid = true;
                    validCount++;

                    if (screenPt.x < minX) minX = screenPt.x;
                    if (screenPt.x > maxX) maxX = screenPt.x;
                    if (screenPt.y < minY) minY = screenPt.y;
                    if (screenPt.y > maxY) maxY = screenPt.y;
                }
            }

            if (validCount < 2)
                return false;

            // Draw bone connection lines if enabled
            if (g_Config.visuals.bSkeleton)
            {
                ImU32 boneColor = color;
                if (g_Config.visuals.bSkeletonVisibleCheck)
                {
                    XRay::Fvector camPos = XRay::Engine::GetCameraPosition();
                    XRay::CObject* pActor = XRay::Engine::GetCurrentEntity();
                    bool isVis = false;
                    int checkBones[3] = { 0, 2, 3 }; // Head, Chest, Pelvis
                    for (int b : checkBones)
                    {
                        XRay::Fvector bonePos;
                        if (XRay::Engine::GetBoneWorldPosition(pObj, b, bonePos))
                        {
                            if (XRay::Engine::IsVisible(camPos, bonePos, pActor))
                            {
                                isVis = true;
                                break;
                            }
                        }
                    }

                    boneColor = isVis ? ImGui::ColorConvertFloat4ToU32(g_Config.visuals.colSkeletonVisible)
                                      : ImGui::ColorConvertFloat4ToU32(g_Config.visuals.colSkeletonOccluded);
                }

                for (size_t i = 0; i < boneCount; ++i)
                {
                    if (!screenBones[i].valid) continue;

                    XRay::CBoneData* pData = boneDataFirst[i];
                    if (!pData || IsBadReadPtr(pData, sizeof(XRay::CBoneData))) continue;

                    uint16_t parentId = pData->parent_id;
                    if (parentId < boneCount && parentId != 0xFFFF && screenBones[parentId].valid)
                    {
                        pDrawList->AddLine(
                            ImVec2(screenBones[i].screenPos.x, screenBones[i].screenPos.y),
                            ImVec2(screenBones[parentId].screenPos.x, screenBones[parentId].screenPos.y),
                            boneColor,
                            1.5f
                        );
                    }
                }
            }

            outMinX = minX;
            outMinY = minY;
            outMaxX = maxX;
            outMaxY = maxY;
            return true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            return false;
        }
    }

    void Visuals::RenderAnomaly(XRay::CObject* pObj, ImDrawList* pDrawList, const XRay::Fvector& origin,
                                float distance, const char* sectName)
    {
        if (!g_Config.visuals.bAnomalies) return;
        if (distance > g_Config.visuals.fAnomalyDistance) return;

        // Friendly anomaly name
        const char* anomName = "Anomaly";
        if (strstr(sectName, "mosquito_bald") || strstr(sectName, "bald")) anomName = "Springboard";
        else if (strstr(sectName, "mincer")) anomName = "Whirligig";
        else if (strstr(sectName, "gravi")) anomName = "Vortex";
        else if (strstr(sectName, "galantine") || strstr(sectName, "witches")) anomName = "Electro";
        else if (strstr(sectName, "zharka")) anomName = "Burner";
        else if (strstr(sectName, "buzz")) anomName = "Fruit Punch";
        else if (strstr(sectName, "radioactive")) anomName = "Radiation Zone";
        else if (strstr(sectName, "teleport")) anomName = "Teleport";

        ImU32 anomColor = ImGui::ColorConvertFloat4ToU32(g_Config.visuals.colAnomaly);

        // Draw 3D ground danger puddle (inspired by chitalka inferno puddle)
        const int NUM_SEGMENTS = 24;
        float radius = 2.8f;
        ImVec2 screenPoints[NUM_SEGMENTS];
        int validPts = 0;

        for (int i = 0; i < NUM_SEGMENTS; ++i)
        {
            float angle = (i * 2.0f * 3.14159265f) / NUM_SEGMENTS;
            XRay::Fvector pt = origin + XRay::Fvector(radius * std::cos(angle), 0.15f, radius * std::sin(angle));
            XRay::Fvector2 spt;
            if (XRay::Math::WorldToScreen(pt, spt))
            {
                screenPoints[validPts++] = ImVec2(spt.x, spt.y);
            }
        }

        if (validPts >= 3)
        {
            ImVec4 fillVec = g_Config.visuals.colAnomaly;
            fillVec.w *= 0.28f; // Translucent puddle fill like chitalka
            ImU32 fillColor = ImGui::ColorConvertFloat4ToU32(fillVec);

            pDrawList->AddConvexPolyFilled(screenPoints, validPts, fillColor);
            pDrawList->AddPolyline(screenPoints, validPts, anomColor, ImDrawFlags_Closed, 2.0f);
        }

        // Draw center icon & distance label
        XRay::Fvector2 centerScreen;
        if (XRay::Math::WorldToScreen(origin + XRay::Fvector(0, 0.5f, 0), centerScreen))
        {
            char labelBuf[128];
            snprintf(labelBuf, sizeof(labelBuf), "[!] %s (%.0fm)", anomName, distance);
            ImVec2 textSize = ImGui::CalcTextSize(labelBuf);

            ImVec2 badgePos(centerScreen.x - textSize.x * 0.5f - 4.0f, centerScreen.y - textSize.y * 0.5f - 2.0f);
            pDrawList->AddRectFilled(
                badgePos,
                ImVec2(badgePos.x + textSize.x + 8.0f, badgePos.y + textSize.y + 4.0f),
                IM_COL32(15, 15, 20, 200),
                4.0f
            );
            pDrawList->AddRect(
                badgePos,
                ImVec2(badgePos.x + textSize.x + 8.0f, badgePos.y + textSize.y + 4.0f),
                anomColor,
                4.0f,
                0,
                1.0f
            );

            pDrawList->AddText(
                ImVec2(badgePos.x + 4.0f, badgePos.y + 2.0f),
                IM_COL32(255, 255, 255, 255),
                labelBuf
            );
        }
    }

    enum class EntityCategory
    {
        Unknown,
        Player,
        Stalker,
        Bandit,
        Military,
        Monolith,
        Mercenary,
        Freedom,
        Duty,
        Zombie,
        Mutant,
        Weapon,
        Artifact,
        Anomaly
    };

    static EntityCategory ClassifyObject(uintptr_t objAddr, XRay::CObject* pObj, XRay::CObject* pActor,
                                         const char*& outName, const char*& outSection, const char*& outVisual)
    {
        if (!pObj) return EntityCategory::Unknown;
        if (pObj == pActor) return EntityCategory::Player;

        outSection = XRay::SafeReadSharedStr(objAddr + 0xAC);
        outName = XRay::SafeReadSharedStr(objAddr + 0xA8);
        outVisual = XRay::SafeReadSharedStr(objAddr + 0xB0);

        char sectLower[128] = { 0 };
        char visualLower[128] = { 0 };
        char nameLower[128] = { 0 };

        for (size_t i = 0; i < sizeof(sectLower) - 1 && outSection[i]; ++i)
            sectLower[i] = static_cast<char>(tolower(static_cast<unsigned char>(outSection[i])));
        for (size_t i = 0; i < sizeof(visualLower) - 1 && outVisual[i]; ++i)
            visualLower[i] = static_cast<char>(tolower(static_cast<unsigned char>(outVisual[i])));
        for (size_t i = 0; i < sizeof(nameLower) - 1 && outName[i]; ++i)
            nameLower[i] = static_cast<char>(tolower(static_cast<unsigned char>(outName[i])));

        // Skip player if not already caught
        if (strstr(sectLower, "actor") || strstr(nameLower, "single_player"))
            return EntityCategory::Player;

        // Anomalies check
        if (strncmp(sectLower, "zone_", 5) == 0 ||
            strstr(sectLower, "anomaly") ||
            strstr(sectLower, "mosquito_bald") ||
            strstr(sectLower, "mincer") ||
            strstr(sectLower, "gravi_zone") ||
            strstr(sectLower, "galantine") ||
            strstr(sectLower, "zharka") ||
            strstr(sectLower, "buzz") ||
            strstr(sectLower, "radioactive"))
        {
            return EntityCategory::Anomaly;
        }

        // 1. Mutants / Monsters
        if (strstr(visualLower, "monster") || strstr(visualLower, "mutant") ||
            strncmp(sectLower, "m_", 2) == 0 ||
            strstr(sectLower, "boar") || strstr(sectLower, "flesh") || strstr(sectLower, "dog") ||
            strstr(sectLower, "bloodsucker") || strstr(sectLower, "krovosos") || strstr(sectLower, "snork") ||
            strstr(sectLower, "controller") || strstr(sectLower, "gigant") || strstr(sectLower, "tushkano") ||
            strstr(sectLower, "burer") || strstr(sectLower, "chimera") || strstr(sectLower, "poltergeist"))
        {
            return EntityCategory::Mutant;
        }

        // 2. Human Characters
        bool isHuman = false;
        if (strstr(visualLower, "actor") || strstr(visualLower, "stalker") || strstr(visualLower, "hero"))
        {
            isHuman = true;
        }
        else if (strstr(sectLower, "stalker") || strstr(sectLower, "bandit") || strstr(sectLower, "soldier") ||
                 strstr(sectLower, "military") || strstr(sectLower, "monolith") || strstr(sectLower, "merc") ||
                 strstr(sectLower, "dolg") || strstr(sectLower, "duty") || strstr(sectLower, "freedom") ||
                 strstr(sectLower, "zombied") || strstr(sectLower, "novice") || strstr(sectLower, "veteran") ||
                 strstr(sectLower, "master") || strstr(sectLower, "guard") || strstr(sectLower, "commander") ||
                 strncmp(sectLower, "esc_", 4) == 0 || strncmp(sectLower, "gar_", 4) == 0 ||
                 strncmp(sectLower, "agr_", 4) == 0 || strncmp(sectLower, "val_", 4) == 0 ||
                 strncmp(sectLower, "bar_", 4) == 0 || strncmp(sectLower, "mil_", 4) == 0 ||
                 strncmp(sectLower, "rad_", 4) == 0 || strncmp(sectLower, "pri_", 4) == 0)
        {
            isHuman = true;
        }

        if (isHuman)
        {
            if (strstr(sectLower, "bandit") || strstr(visualLower, "bandit") || strstr(nameLower, "bandit"))
                return EntityCategory::Bandit;
            if (strstr(sectLower, "monolith") || strstr(visualLower, "monolith") || strstr(nameLower, "monolith"))
                return EntityCategory::Monolith;
            if (strstr(sectLower, "merc") || strstr(sectLower, "killer") || strstr(visualLower, "merc"))
                return EntityCategory::Mercenary;
            if (strstr(sectLower, "military") || strstr(sectLower, "soldier") || strstr(visualLower, "soldier") || strstr(sectLower, "voen"))
                return EntityCategory::Military;
            if (strstr(sectLower, "zombied") || strstr(visualLower, "zombied"))
                return EntityCategory::Zombie;
            if (strstr(sectLower, "freedom") || strstr(sectLower, "svoboda") || strstr(visualLower, "freedom"))
                return EntityCategory::Freedom;
            if (strstr(sectLower, "dolg") || strstr(sectLower, "duty") || strstr(visualLower, "dolg"))
                return EntityCategory::Duty;

            return EntityCategory::Stalker;
        }

        // 3. Weapons
        if (strncmp(sectLower, "wpn_", 4) == 0 || strstr(visualLower, "weapons"))
        {
            return EntityCategory::Weapon;
        }

        // 4. Artifacts
        if (strncmp(sectLower, "af_", 3) == 0 || strstr(sectLower, "artefact") || strstr(sectLower, "artifact") || strstr(visualLower, "artefact"))
        {
            return EntityCategory::Artifact;
        }

        return EntityCategory::Unknown;
    }

    void Visuals::ProcessObject(XRay::CObject* pObj, ImDrawList* pDrawList, const XRay::Fvector& localPos)
    {
        if (!pObj || IsBadReadPtr(pObj, 0x100)) return;

        __try
        {
            uintptr_t objAddr = reinterpret_cast<uintptr_t>(pObj);
            XRay::CObject* pActor = XRay::Engine::GetCurrentEntity();

            if (pObj == pActor) return;

            // Read 3D world position from verified CObject offset [obj + 0x80]
            float posX = *reinterpret_cast<float*>(objAddr + 0x80);
            float posY = *reinterpret_cast<float*>(objAddr + 0x84);
            float posZ = *reinterpret_cast<float*>(objAddr + 0x88);

            // Sanity check coordinates
            if (posX == 0.0f && posY == 0.0f && posZ == 0.0f)
                return;

            XRay::Fvector origin(posX, posY, posZ);
            float distance = origin.DistanceTo(localPos);

            if (distance > g_Config.visuals.fMaxDistance || distance < 0.2f)
                return;

            const char* sectName = "";
            const char* objName = "";
            const char* visualName = "";
            EntityCategory category = ClassifyObject(objAddr, pObj, pActor, objName, sectName, visualName);

            if (category == EntityCategory::Unknown || category == EntityCategory::Player)
                return;

            // Handle Anomaly
            if (category == EntityCategory::Anomaly)
            {
                RenderAnomaly(pObj, pDrawList, origin, distance, sectName);
                return;
            }

            float entityHeight = 1.8f;
            ImU32 boxColor = IM_COL32(255, 255, 255, 255);
            const char* typeTag = "Entity";
            bool isLiving = false;
            float healthRatio = 1.0f;

            switch (category)
            {
            case EntityCategory::Stalker:
                if (!g_Config.visuals.bShowEnemies && !g_Config.visuals.bShowNeutrals && !g_Config.visuals.bShowFriends)
                    return;
                isLiving = true;
                entityHeight = 1.8f;
                boxColor = ImGui::ColorConvertFloat4ToU32(g_Config.visuals.colFriend);
                typeTag = "Stalker";
                break;

            case EntityCategory::Bandit:
                if (!g_Config.visuals.bShowEnemies) return;
                isLiving = true;
                entityHeight = 1.8f;
                boxColor = ImGui::ColorConvertFloat4ToU32(g_Config.visuals.colEnemy);
                typeTag = "Bandit";
                break;

            case EntityCategory::Military:
                if (!g_Config.visuals.bShowEnemies) return;
                isLiving = true;
                entityHeight = 1.8f;
                boxColor = ImGui::ColorConvertFloat4ToU32(g_Config.visuals.colEnemy);
                typeTag = "Military";
                break;

            case EntityCategory::Monolith:
                if (!g_Config.visuals.bShowEnemies) return;
                isLiving = true;
                entityHeight = 1.8f;
                boxColor = ImGui::ColorConvertFloat4ToU32(g_Config.visuals.colEnemy);
                typeTag = "Monolith";
                break;

            case EntityCategory::Mercenary:
                if (!g_Config.visuals.bShowEnemies) return;
                isLiving = true;
                entityHeight = 1.8f;
                boxColor = ImGui::ColorConvertFloat4ToU32(g_Config.visuals.colEnemy);
                typeTag = "Merc";
                break;

            case EntityCategory::Zombie:
                if (!g_Config.visuals.bShowEnemies) return;
                isLiving = true;
                entityHeight = 1.8f;
                boxColor = ImGui::ColorConvertFloat4ToU32(g_Config.visuals.colEnemy);
                typeTag = "Zombie";
                break;

            case EntityCategory::Freedom:
                if (!g_Config.visuals.bShowNeutrals && !g_Config.visuals.bShowFriends) return;
                isLiving = true;
                entityHeight = 1.8f;
                boxColor = ImGui::ColorConvertFloat4ToU32(g_Config.visuals.colNeutral);
                typeTag = "Freedom";
                break;

            case EntityCategory::Duty:
                if (!g_Config.visuals.bShowNeutrals && !g_Config.visuals.bShowFriends) return;
                isLiving = true;
                entityHeight = 1.8f;
                boxColor = ImGui::ColorConvertFloat4ToU32(g_Config.visuals.colNeutral);
                typeTag = "Duty";
                break;

            case EntityCategory::Mutant:
                if (!g_Config.visuals.bShowMutants) return;
                isLiving = true;
                entityHeight = 1.2f;
                boxColor = ImGui::ColorConvertFloat4ToU32(g_Config.visuals.colMutant);
                typeTag = "Mutant";
                break;

            case EntityCategory::Weapon:
                if (!g_Config.visuals.bShowWeapons) return;
                entityHeight = 0.35f;
                boxColor = ImGui::ColorConvertFloat4ToU32(g_Config.visuals.colWeapon);
                typeTag = "Weapon";
                break;

            case EntityCategory::Artifact:
                if (!g_Config.visuals.bShowArtifacts) return;
                entityHeight = 0.35f;
                boxColor = ImGui::ColorConvertFloat4ToU32(g_Config.visuals.colArtifact);
                typeTag = "Artifact";
                break;

            default:
                return;
            }

            // Screen coordinates calculation
            float boxX = 0, boxY = 0, boxW = 0, boxH = 0;
            XRay::Fvector2 screenFeet;
            bool bHasBox = false;

            // Try dynamic bone skeleton extraction first
            if (isLiving)
            {
                float minX = 0, minY = 0, maxX = 0, maxY = 0;
                ImU32 skelColor = ImGui::ColorConvertFloat4ToU32(g_Config.visuals.colSkeleton);
                if (g_Config.visuals.bSkeletonByRelation)
                {
                    if (category == EntityCategory::Stalker)
                        skelColor = ImGui::ColorConvertFloat4ToU32(g_Config.visuals.colSkeletonFriend);
                    else if (category == EntityCategory::Freedom || category == EntityCategory::Duty)
                        skelColor = ImGui::ColorConvertFloat4ToU32(g_Config.visuals.colSkeletonNeutral);
                    else if (category == EntityCategory::Mutant)
                        skelColor = ImGui::ColorConvertFloat4ToU32(g_Config.visuals.colSkeletonMutant);
                    else
                        skelColor = ImGui::ColorConvertFloat4ToU32(g_Config.visuals.colSkeletonEnemy);
                }
                if (DrawSkeletonAndGetBounds(pObj, pDrawList, skelColor, minX, minY, maxX, maxY))
                {
                    float padX = (maxX - minX) * 0.12f + 4.0f;
                    float padY = 6.0f;
                    boxX = minX - padX;
                    boxY = minY - padY;
                    boxW = (maxX - minX) + padX * 2.0f;
                    boxH = (maxY - minY) + padY * 2.0f;
                    screenFeet = XRay::Fvector2(minX + (maxX - minX) * 0.5f, maxY);
                    bHasBox = (boxH >= 4.0f);
                }
            }

            // Fallback to bounding cylinder if skeleton was unavailable
            if (!bHasBox)
            {
                XRay::Fvector head = origin + XRay::Fvector(0.0f, entityHeight, 0.0f);
                XRay::Fvector2 screenHead;
                if (!XRay::Math::WorldToScreen(origin, screenFeet) || !XRay::Math::WorldToScreen(head, screenHead))
                {
                    if (g_Config.rage.bOffscreenArrows && (category == EntityCategory::Bandit || category == EntityCategory::Military || category == EntityCategory::Monolith || category == EntityCategory::Mercenary || category == EntityCategory::Zombie || category == EntityCategory::Mutant))
                    {
                        DrawOffscreenArrow(origin, boxColor);
                    }
                    return;
                }

                boxH = std::abs(screenFeet.y - screenHead.y);
                if (boxH < 4.0f) return;

                boxW = boxH * 0.55f;
                boxX = screenHead.x - boxW * 0.5f;
                boxY = screenHead.y;
                bHasBox = true;
            }

            ImGuiIO& io = ImGui::GetIO();
            bool isOffscreen = (screenFeet.x < -10.0f || screenFeet.x > io.DisplaySize.x + 10.0f || screenFeet.y < -10.0f || screenFeet.y > io.DisplaySize.y + 10.0f);
            if (isOffscreen && g_Config.rage.bOffscreenArrows && (category == EntityCategory::Bandit || category == EntityCategory::Military || category == EntityCategory::Monolith || category == EntityCategory::Mercenary || category == EntityCategory::Zombie || category == EntityCategory::Mutant))
            {
                DrawOffscreenArrow(origin, boxColor);
            }

            // 1. Draw 2D Box
            if (g_Config.visuals.bBox2D && bHasBox)
            {
                DrawBox2D(boxX, boxY, boxW, boxH, boxColor, g_Config.visuals.iBoxType == 0);
            }

            // 2. Draw Health Bar
            if (g_Config.visuals.bHealthBar && isLiving && bHasBox)
            {
                DrawHealthBar(boxX, boxY, boxH, healthRatio);
            }

            // 3. Draw Snapline
            if (g_Config.visuals.bSnaplines)
            {
                DrawSnapline(screenFeet, boxColor, g_Config.visuals.iSnaplinePos);
            }

            // 4. Draw Name / Tag
            if (g_Config.visuals.bName && bHasBox)
            {
                char labelBuf[128];
                if (objName && objName[0] != '\0')
                    snprintf(labelBuf, sizeof(labelBuf), "[%s] %s", typeTag, objName);
                else
                    snprintf(labelBuf, sizeof(labelBuf), "[%s]", typeTag);

                ImVec2 textSize = ImGui::CalcTextSize(labelBuf);

                pDrawList->AddText(
                    ImVec2(boxX + (boxW - textSize.x) * 0.5f, boxY - textSize.y - 2.0f),
                    IM_COL32(255, 255, 255, 240),
                    labelBuf
                );
            }

            // 5. Draw Distance
            if (g_Config.visuals.bDistance && bHasBox)
            {
                char distBuf[64];
                snprintf(distBuf, sizeof(distBuf), "%.0fm", distance);
                ImVec2 distSize = ImGui::CalcTextSize(distBuf);

                pDrawList->AddText(
                    ImVec2(boxX + (boxW - distSize.x) * 0.5f, boxY + boxH + 2.0f),
                    IM_COL32(220, 220, 220, 230),
                    distBuf
                );
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
        }
    }

    void Visuals::Render()
    {
        if (!g_Config.visuals.bEnabled) return;
        if (!XRay::Engine::IsInGame()) return;

        ImDrawList* pDrawList = ImGui::GetBackgroundDrawList();

        // Custom crosshair
        if (g_Config.visuals.bCrosshair)
        {
            DrawCustomCrosshair(ImGui::ColorConvertFloat4ToU32(g_Config.visuals.colCrosshair));
        }

        // Aimbot FOV circle
        if (g_Config.aimbot.bEnabled && g_Config.visuals.bDrawAimbotFov)
        {
            DrawFovCircle(g_Config.aimbot.fFov, IM_COL32(255, 255, 255, 120));
        }

        XRay::Fvector localCamPos = XRay::Engine::GetCameraPosition();
        XRay::CObject* pActor = XRay::Engine::GetCurrentEntity();

        // 1. Iterate active level objects
        XRay::xr_vector<XRay::CObject*>* pActiveObjs = XRay::Engine::GetActiveObjects();
        if (pActiveObjs)
        {
            __try
            {
                size_t activeCount = pActiveObjs->size();
                if (activeCount > 0 && activeCount < 20000)
                {
                    for (size_t i = 0; i < activeCount; ++i)
                    {
                        XRay::CObject* pObj = pActiveObjs->at(i);
                        if (pObj && pObj != pActor)
                        {
                            ProcessObject(pObj, pDrawList, localCamPos);
                        }
                    }
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
            }
        }

        // 2. Fallback to total objects if active list was empty
        uint32_t totalCount = XRay::Engine::GetTotalObjectCount();
        if ((!pActiveObjs || pActiveObjs->size() == 0) && totalCount > 0 && totalCount < 20000)
        {
            __try
            {
                for (uint32_t i = 0; i < totalCount; ++i)
                {
                    XRay::CObject* pObj = XRay::Engine::GetObjectByIndex(i);
                    if (pObj && pObj != pActor)
                    {
                        ProcessObject(pObj, pDrawList, localCamPos);
                    }
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
            }
        }
    }
}
