#include "Menu.hpp"
#include "../core/Config.hpp"
#include "../core/Globals.hpp"
#include "../core/Logger.hpp"
#include "../xray/Offsets.hpp"
#include <imgui.h>

namespace UI
{
    void Menu::RenderVisualsTab()
    {
        ImGui::BeginChild("Visuals_Main", ImVec2(0, 0), true);

        ImGui::TextColored(ImVec4(0.95f, 0.68f, 0.15f, 1.0f), "ESP & Overlay Configuration");
        ImGui::Separator();

        ImGui::Checkbox("Enable Visuals (ESP)", &g_Config.visuals.bEnabled);
        ImGui::Spacing();

        if (ImGui::CollapsingHeader("Box & Lines", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Checkbox("2D Box", &g_Config.visuals.bBox2D);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(140);
            const char* boxTypes[] = { "Corners", "Full Box" };
            ImGui::Combo("##BoxType", &g_Config.visuals.iBoxType, boxTypes, IM_ARRAYSIZE(boxTypes));

            ImGui::Checkbox("Health Bar", &g_Config.visuals.bHealthBar);
            ImGui::Checkbox("Distance", &g_Config.visuals.bDistance);
            ImGui::Checkbox("Name & Tag", &g_Config.visuals.bName);

            ImGui::Checkbox("Snaplines", &g_Config.visuals.bSnaplines);
            if (g_Config.visuals.bSnaplines)
            {
                ImGui::SameLine();
                ImGui::SetNextItemWidth(140);
                const char* snapPositions[] = { "Bottom", "Crosshair" };
                ImGui::Combo("##SnaplinePos", &g_Config.visuals.iSnaplinePos, snapPositions, IM_ARRAYSIZE(snapPositions));
            }
        }

        if (ImGui::CollapsingHeader("Entity Filters", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Checkbox("Show Stalkers / Enemies", &g_Config.visuals.bShowEnemies);
            ImGui::Checkbox("Show Mutants / Monsters", &g_Config.visuals.bShowMutants);
            ImGui::Checkbox("Show Artifacts", &g_Config.visuals.bShowArtifacts);

            ImGui::SliderFloat("Max Distance (m)", &g_Config.visuals.fMaxDistance, 10.0f, 1000.0f, "%.0f m");
        }

        if (ImGui::CollapsingHeader("Crosshair & FOV"))
        {
            ImGui::Checkbox("Custom Crosshair", &g_Config.visuals.bCrosshair);
            ImGui::Checkbox("Draw Aimbot FOV Circle", &g_Config.visuals.bDrawAimbotFov);
        }

        if (ImGui::CollapsingHeader("ESP Colors"))
        {
            ImGui::ColorEdit4("Enemies", reinterpret_cast<float*>(&g_Config.visuals.colEnemy));
            ImGui::ColorEdit4("Mutants", reinterpret_cast<float*>(&g_Config.visuals.colMutant));
            ImGui::ColorEdit4("Artifacts", reinterpret_cast<float*>(&g_Config.visuals.colArtifact));
        }

        ImGui::EndChild();
    }

    void Menu::RenderAimbotTab()
    {
        ImGui::BeginChild("Aimbot_Main", ImVec2(0, 0), true);

        ImGui::TextColored(ImVec4(0.95f, 0.68f, 0.15f, 1.0f), "Aimbot Settings (Hold Right Mouse Button)");
        ImGui::Separator();

        ImGui::Checkbox("Enable Aimbot", &g_Config.aimbot.bEnabled);
        ImGui::Spacing();

        ImGui::SliderFloat("FOV Radius (Pixels)", &g_Config.aimbot.fFov, 10.0f, 500.0f, "%.0f px");
        ImGui::SliderFloat("Smoothness", &g_Config.aimbot.fSmoothness, 1.0f, 30.0f, "%.1f");

        ImGui::Spacing();
        const char* bones[] = { "Head", "Chest" };
        ImGui::Combo("Target Hitbox", &g_Config.aimbot.iTargetBone, bones, IM_ARRAYSIZE(bones));

        ImGui::Spacing();
        ImGui::Text("Target Filters:");
        ImGui::Checkbox("Target Stalkers", &g_Config.aimbot.bTargetEnemies);
        ImGui::Checkbox("Target Mutants", &g_Config.aimbot.bTargetMutants);

        ImGui::EndChild();
    }

    void Menu::RenderMiscTab()
    {
        ImGui::BeginChild("Misc_Main", ImVec2(0, 0), true);

        ImGui::TextColored(ImVec4(0.95f, 0.68f, 0.15f, 1.0f), "Singleplayer Exploits & Game Tweaks");
        ImGui::Separator();

        ImGui::Checkbox("Godmode (Infinite Health)", &g_Config.misc.bGodmode);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Continuously restores Marked One (Actor) health to 100%");

        ImGui::Checkbox("Infinite Ammo", &g_Config.misc.bInfiniteAmmo);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Patches weapon magazine decrement instruction in xrGame.dll");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Note: Features run safely in offline singleplayer.");

        ImGui::EndChild();
    }

    void Menu::RenderWorldTab()
    {
        ImGui::BeginChild("World_Main", ImVec2(0, 0), true);

        ImGui::TextColored(ImVec4(0.95f, 0.68f, 0.15f, 1.0f), "X-Ray Engine Diagnostics");
        ImGui::Separator();

        XRay::CRenderDevice* pDev = XRay::Engine::GetDevice();
        ImGui::Text("Render Device: %s (0x%p)", pDev ? "Connected" : "Not Found", pDev);
        if (pDev)
        {
            XRay::Fvector camPos = XRay::Engine::GetCameraPosition();
            ImGui::Text("Camera Pos: (%.1f, %.1f, %.1f)", camPos.x, camPos.y, camPos.z);
        }

        XRay::CLevel* pLevel = XRay::Engine::GetLevel();
        ImGui::Text("Level Singleton: %s (0x%p)", pLevel ? "Active (In Game)" : "Inactive (Menu/Loading)", pLevel);

        XRay::CObject* pActor = XRay::Engine::GetCurrentEntity();
        ImGui::Text("Player Entity (Actor): %s (0x%p)", pActor ? "Detected" : "None", pActor);

        XRay::xr_vector<XRay::CObject*>* pActiveObjs = XRay::Engine::GetActiveObjects();
        if (pActiveObjs)
        {
            __try
            {
                size_t activeCount = pActiveObjs->size();
                ImGui::Text("Active Level Objects: %zu", activeCount);
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                ImGui::Text("Active Level Objects: Read Error");
            }
        }

        XRay::xr_vector<XRay::CObject*>* pSleepObjs = XRay::Engine::GetSleepingObjects();
        if (pSleepObjs)
        {
            __try
            {
                size_t sleepCount = pSleepObjs->size();
                ImGui::Text("Sleeping Level Objects: %zu", sleepCount);
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
            }
        }

        uint32_t totalCount = XRay::Engine::GetTotalObjectCount();
        ImGui::Text("Total Level Objects: %u", totalCount);

        ImGui::EndChild();
    }

    void Menu::RenderSettingsTab()
    {
        ImGui::BeginChild("Settings_Main", ImVec2(0, 0), true);

        ImGui::TextColored(ImVec4(0.95f, 0.68f, 0.15f, 1.0f), "Configuration & DLL Lifecycle");
        ImGui::Separator();

        ImGui::Text("Menu Toggle Key: INSERT");
        ImGui::Text("Eject Key: END");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Eject / Unhook DLL", ImVec2(200, 32)))
        {
            Core::Logger::Warning("Eject requested via menu button!");
            Core::g_bUnloadRequested.store(true);
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Safely removes DirectX 9 and WndProc hooks and unloads the DLL from XR_3DA.exe.");

        ImGui::EndChild();
    }

    void Menu::Render()
    {
        if (!g_Config.bShowMenu) return;

        ImGui::SetNextWindowSize(ImVec2(620, 440), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("S.T.A.L.K.E.R. - Internal SOC v1.0006 [X-Ray 1.0]", &g_Config.bShowMenu, ImGuiWindowFlags_NoCollapse))
        {
            if (ImGui::BeginTabBar("MainTabBar"))
            {
                if (ImGui::BeginTabItem("Visuals / ESP"))
                {
                    RenderVisualsTab();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Aimbot"))
                {
                    RenderAimbotTab();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Misc"))
                {
                    RenderMiscTab();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("World & SDK"))
                {
                    RenderWorldTab();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Settings"))
                {
                    RenderSettingsTab();
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
        }
        ImGui::End();
    }
}
