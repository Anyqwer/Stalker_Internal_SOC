#include "Visuals.hpp"
#include "../core/Config.hpp"
#include "../core/Logger.hpp"
#include "../xray/Offsets.hpp"
#include "../xray/WorldToScreen.hpp"
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <cctype>

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
            // Full box with black outline
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
        float size = 5.0f;

        pDrawList->AddLine(ImVec2(cx - size, cy), ImVec2(cx + size + 1, cy), color, 1.5f);
        pDrawList->AddLine(ImVec2(cx, cy - size), ImVec2(cx, cy + size + 1), color, 1.5f);
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
        Artifact
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
        if (!pObj) return;

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

            // Calculate 3D points
            XRay::Fvector head = origin + XRay::Fvector(0.0f, entityHeight, 0.0f);

            XRay::Fvector2 screenFeet, screenHead;
            if (!XRay::Math::WorldToScreen(origin, screenFeet) || !XRay::Math::WorldToScreen(head, screenHead))
                return;

            float boxH = std::abs(screenFeet.y - screenHead.y);
            if (boxH < 4.0f) return;

            float boxW = boxH * 0.55f;
            float boxX = screenHead.x - boxW * 0.5f;
            float boxY = screenHead.y;

            // 1. Draw 2D Box
            if (g_Config.visuals.bBox2D)
            {
                DrawBox2D(boxX, boxY, boxW, boxH, boxColor, g_Config.visuals.iBoxType == 0);
            }

            // 2. Draw Health Bar
            if (g_Config.visuals.bHealthBar && isLiving)
            {
                DrawHealthBar(boxX, boxY, boxH, healthRatio);
            }

            // 3. Draw Snapline
            if (g_Config.visuals.bSnaplines)
            {
                DrawSnapline(screenFeet, boxColor, g_Config.visuals.iSnaplinePos);
            }

            // 4. Draw Name / Tag
            if (g_Config.visuals.bName)
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
            if (g_Config.visuals.bDistance)
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

        // 1. First iterate active level objects
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

        // 2. Also check if total objects can be enumerated via engine iterator if active was empty
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
