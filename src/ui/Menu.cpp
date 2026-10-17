#include "Menu.hpp"
#include "../core/Config.hpp"
#include "../core/Globals.hpp"
#include "../core/Logger.hpp"
#include "../xray/Offsets.hpp"
#include <imgui.h>
#include <algorithm>
#include <vector>
#include <string>

namespace UI
{
    static void BeginCard(const char* id, const char* title, const ImVec2& size = ImVec2(0, 0))
    {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.14f, 0.15f, 0.16f, 0.75f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.28f, 0.30f, 0.32f, 0.70f));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));

        ImGui::BeginChild(id, size, true);
        ImGui::TextColored(ImVec4(0.95f, 0.68f, 0.15f, 1.0f), "%s", title);
        ImGui::Separator();
        ImGui::Spacing();
    }

    static void EndCard()
    {
        ImGui::EndChild();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);
    }

    void Menu::RenderVisualsTab()
    {
        float contentW = ImGui::GetContentRegionAvail().x;
        float colW = (contentW - 12.0f) * 0.5f;

        // Left Card: ESP Overlays
        BeginCard("Card_Visuals_ESP", "ESP & Overlays", ImVec2(colW, 0));
        {
            ImGui::Checkbox("Enable Visuals (ESP)", &g_Config.visuals.bEnabled);
            ImGui::Spacing();

            ImGui::Checkbox("2D Bounding Box", &g_Config.visuals.bBox2D);
            if (g_Config.visuals.bBox2D)
            {
                ImGui::SameLine();
                ImGui::SetNextItemWidth(110 * g_Config.settings.fUiScale);
                const char* boxTypes[] = { "Corners", "Full Box" };
                ImGui::Combo("##BoxType", &g_Config.visuals.iBoxType, boxTypes, IM_ARRAYSIZE(boxTypes));
            }

            ImGui::Checkbox("Bone Skeletons", &g_Config.visuals.bSkeleton);
            if (g_Config.visuals.bSkeleton)
            {
                ImGui::Indent(15.0f);
                ImGui::Checkbox("Visibility Raycast Check", &g_Config.visuals.bSkeletonVisibleCheck);
                if (g_Config.visuals.bSkeletonVisibleCheck)
                {
                    ImGui::ColorEdit4("Visible##SkelVis", reinterpret_cast<float*>(&g_Config.visuals.colSkeletonVisible), ImGuiColorEditFlags_NoInputs);
                    ImGui::SameLine();
                    ImGui::Text("Visible");
                    ImGui::SameLine(120.0f);
                    ImGui::ColorEdit4("Occluded##SkelOcc", reinterpret_cast<float*>(&g_Config.visuals.colSkeletonOccluded), ImGuiColorEditFlags_NoInputs);
                    ImGui::SameLine();
                    ImGui::Text("Occluded");
                }
                else
                {
                    ImGui::ColorEdit4("Color##SkelCol", reinterpret_cast<float*>(&g_Config.visuals.colSkeleton), ImGuiColorEditFlags_NoInputs);
                    ImGui::SameLine();
                    ImGui::Text("Skeleton Color");
                }
                ImGui::Unindent(15.0f);
            }

            ImGui::Checkbox("Health Bar", &g_Config.visuals.bHealthBar);
            ImGui::Checkbox("Distance Tag", &g_Config.visuals.bDistance);
            ImGui::Checkbox("Name & Faction", &g_Config.visuals.bName);

            ImGui::Checkbox("Snaplines", &g_Config.visuals.bSnaplines);
            if (g_Config.visuals.bSnaplines)
            {
                ImGui::SameLine();
                ImGui::SetNextItemWidth(110 * g_Config.settings.fUiScale);
                const char* snapPositions[] = { "Bottom", "Crosshair" };
                ImGui::Combo("##SnaplinePos", &g_Config.visuals.iSnaplinePos, snapPositions, IM_ARRAYSIZE(snapPositions));
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("Crosshair & Markers:");
            ImGui::Checkbox("Custom Crosshair", &g_Config.visuals.bCrosshair);
            if (g_Config.visuals.bCrosshair)
            {
                ImGui::SameLine();
                ImGui::ColorEdit4("##ColCross", reinterpret_cast<float*>(&g_Config.visuals.colCrosshair), ImGuiColorEditFlags_NoInputs);
            }

            ImGui::Checkbox("Draw Aimbot FOV Circle", &g_Config.visuals.bDrawAimbotFov);

            ImGui::Spacing();
            ImGui::SliderFloat("Max Distance (m)", &g_Config.visuals.fMaxDistance, 10.0f, 1000.0f, "%.0f m");
        }
        EndCard();

        ImGui::SameLine();

        // Right Card: Entities & Anomalies
        BeginCard("Card_Visuals_Filters", "Filters & Anomalies", ImVec2(colW, 0));
        {
            ImGui::Text("Anomaly Zones:");
            ImGui::Checkbox("Anomalies ESP (3D Radii)", &g_Config.visuals.bAnomalies);
            if (g_Config.visuals.bAnomalies)
            {
                ImGui::SameLine();
                ImGui::ColorEdit4("##ColAnom", reinterpret_cast<float*>(&g_Config.visuals.colAnomaly), ImGuiColorEditFlags_NoInputs);
                ImGui::SliderFloat("Anomaly Dist (m)", &g_Config.visuals.fAnomalyDistance, 10.0f, 500.0f, "%.0f m");
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("Entity Filters:");
            ImGui::Checkbox("Show Stalkers / Friends", &g_Config.visuals.bShowFriends);
            ImGui::Checkbox("Show Neutrals (Duty/Freedom)", &g_Config.visuals.bShowNeutrals);
            ImGui::Checkbox("Show Enemies (Bandit/Monolith)", &g_Config.visuals.bShowEnemies);
            ImGui::Checkbox("Show Mutants / Monsters", &g_Config.visuals.bShowMutants);
            ImGui::Checkbox("Show Artifacts", &g_Config.visuals.bShowArtifacts);
            ImGui::Checkbox("Show Dropped Weapons", &g_Config.visuals.bShowWeapons);

            ImGui::Spacing();
            if (ImGui::CollapsingHeader("Color Palette"))
            {
                ImGui::ColorEdit4("Enemies##Col", reinterpret_cast<float*>(&g_Config.visuals.colEnemy));
                ImGui::ColorEdit4("Neutrals##Col", reinterpret_cast<float*>(&g_Config.visuals.colNeutral));
                ImGui::ColorEdit4("Friends##Col", reinterpret_cast<float*>(&g_Config.visuals.colFriend));
                ImGui::ColorEdit4("Mutants##Col", reinterpret_cast<float*>(&g_Config.visuals.colMutant));
                ImGui::ColorEdit4("Artifacts##Col", reinterpret_cast<float*>(&g_Config.visuals.colArtifact));
                ImGui::ColorEdit4("Weapons##Col", reinterpret_cast<float*>(&g_Config.visuals.colWeapon));
            }
        }
        EndCard();
    }

    void Menu::RenderChamsTab()
    {
        float contentW = ImGui::GetContentRegionAvail().x;
        float colW = (contentW - 12.0f) * 0.5f;

        BeginCard("Card_Chams_Main", "Direct3D 9 Chams Engine", ImVec2(colW, 0));
        {
            ImGui::Checkbox("Enable Chams", &g_Config.chams.bEnabled);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Renders textured / flat chams materials directly via D3D9 DrawIndexedPrimitive hook.");

            ImGui::Spacing();
            ImGui::Checkbox("Through Walls (X-Ray Wallhack)", &g_Config.chams.bThroughWalls);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Renders two passes: occluded behind walls, and visible in front of walls.");

            ImGui::Spacing();
            const char* chamsStyles[] = { "Flat (Solid Unlit)", "Wireframe", "Shaded / Solid" };
            ImGui::Combo("Material Style", &g_Config.chams.iChamsType, chamsStyles, IM_ARRAYSIZE(chamsStyles));

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("Two-Tone Colors:");
            ImGui::ColorEdit4("Visible (In Front)", reinterpret_cast<float*>(&g_Config.chams.colVisible));
            ImGui::ColorEdit4("Occluded (Behind Wall)", reinterpret_cast<float*>(&g_Config.chams.colHidden));
        }
        EndCard();

        ImGui::SameLine();

        BeginCard("Card_Chams_Targets", "Target Filters & Details", ImVec2(colW, 0));
        {
            ImGui::Text("Model Filters:");
            ImGui::Checkbox("Apply to Stalkers / Humans", &g_Config.chams.bChamsStalkers);
            ImGui::Checkbox("Apply to Mutants / Monsters", &g_Config.chams.bChamsMutants);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::TextColored(ImVec4(0.85f, 0.65f, 0.15f, 1.0f), "DirectX 9 Pipeline Information:");
            ImGui::BulletText("Target Hook: VMT[82] DrawIndexedPrimitive");
            ImGui::BulletText("Mesh Filter: Skeletal Stride (32-48 bytes)");
            ImGui::BulletText("Z-Buffer: Automatic dual-depth pass");
            ImGui::BulletText("Performance: Near 0 CPU overhead");
        }
        EndCard();
    }

    void Menu::RenderAimbotTab()
    {
        float contentW = ImGui::GetContentRegionAvail().x;
        float colW = (contentW - 12.0f) * 0.5f;

        BeginCard("Card_Aimbot_Main", "Aimbot Configuration", ImVec2(colW, 0));
        {
            ImGui::Checkbox("Enable Aimbot", &g_Config.aimbot.bEnabled);
            ImGui::Spacing();

            RenderKeyBindWidget("Activation Key", g_Config.aimbot.bindKey, 120.0f);
            ImGui::Spacing();

            ImGui::SliderFloat("FOV Radius (px)", &g_Config.aimbot.fFov, 10.0f, 600.0f, "%.0f px");
            ImGui::SliderFloat("Smoothing Factor", &g_Config.aimbot.fSmoothness, 1.0f, 30.0f, "%.1f");

            ImGui::Spacing();
            const char* bones[] = { "Head", "Neck", "Chest", "Pelvis" };
            ImGui::Combo("Target Hitbox", &g_Config.aimbot.iTargetBone, bones, IM_ARRAYSIZE(bones));

            const char* priorities[] = { "Closest to Crosshair", "Closest 3D Distance" };
            ImGui::Combo("Target Priority", &g_Config.aimbot.iTargetPriority, priorities, IM_ARRAYSIZE(priorities));

            ImGui::Spacing();
            ImGui::Checkbox("Raycast Visible Check", &g_Config.aimbot.bVisCheck);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Only lock onto targets with direct line of sight (CObjectSpace raycast).");
        }
        EndCard();

        ImGui::SameLine();

        BeginCard("Card_Aimbot_Filters", "Target Filters & Triggerbot", ImVec2(colW, 0));
        {
            ImGui::Text("Target Selection:");
            ImGui::Checkbox("Target Stalkers / Humans", &g_Config.aimbot.bTargetEnemies);
            ImGui::Checkbox("Target Mutants / Monsters", &g_Config.aimbot.bTargetMutants);
            ImGui::Checkbox("Target Friends / Allies", &g_Config.aimbot.bTargetFriends);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::TextColored(ImVec4(0.95f, 0.68f, 0.15f, 1.0f), "Triggerbot (Auto-Fire):");
            ImGui::Checkbox("Enable Triggerbot", &g_Config.aimbot.bTriggerbot);
            if (g_Config.aimbot.bTriggerbot)
            {
                ImGui::Checkbox("Always Active (No Key)", &g_Config.aimbot.bTriggerbotAlways);
                if (!g_Config.aimbot.bTriggerbotAlways)
                {
                    RenderKeyBindWidget("Triggerbot Hotkey", g_Config.aimbot.keyTriggerbot, 120.0f);
                }
                ImGui::SliderFloat("Reaction Delay (ms)", &g_Config.aimbot.fTriggerbotDelay, 0.0f, 250.0f, "%.0f ms");
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Engine Integration:");
            ImGui::BulletText("Direct mouse event input emulation");
            ImGui::BulletText("3D Bone Kinematics & Crouch tracking");
            ImGui::BulletText("Native X-Ray line-of-sight raycasting");
        }
        EndCard();
    }

    void Menu::RenderTrainersTab()
    {
        float contentW = ImGui::GetContentRegionAvail().x;
        float colW = (contentW - 12.0f) * 0.5f;

        BeginCard("Card_Trainers_Player", "Player Exploits", ImVec2(colW, 0));
        {
            ImGui::Checkbox("Godmode (Infinite Health & Power)", &g_Config.misc.bGodmode);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Continuously restores Marked One health, stamina, and clears radiation/bleeding.");

            RenderKeyBindWidget("Godmode Toggle Key", g_Config.settings.keyGodmode, 120.0f);
            ImGui::Spacing();

            ImGui::Checkbox("Infinite Ammo", &g_Config.misc.bInfiniteAmmo);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Patches magazine decrement instruction in xrGame.dll.");

            ImGui::Checkbox("No Weapon Recoil", &g_Config.misc.bNoRecoil);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Suppresses camera recoil kick upon firing.");

            ImGui::Checkbox("Bunnyhop (Auto-Jump)", &g_Config.misc.bBunnyhop);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Hold Spacebar to continuously and effortlessly bunny-hop across the Zone.");
        }
        EndCard();

        ImGui::SameLine();

        BeginCard("Card_Trainers_Physics", "Movement & Inventory Physics", ImVec2(colW, 0));
        {
            ImGui::Checkbox("Infinite Carry Weight", &g_Config.misc.bInfiniteWeight);
            if (g_Config.misc.bInfiniteWeight)
            {
                ImGui::SliderFloat("Weight Limit (kg)", &g_Config.misc.fWeightLimit, 100.0f, 100000.0f, "%.0f kg");
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Checkbox("Super Jump", &g_Config.misc.bSuperJump);
            if (g_Config.misc.bSuperJump)
            {
                ImGui::SliderFloat("Jump Velocity", &g_Config.misc.fJumpSpeed, 6.0f, 40.0f, "%.1f");
            }
            RenderKeyBindWidget("Super Jump Hotkey", g_Config.settings.keySuperJump, 120.0f);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Checkbox("Speedhack", &g_Config.misc.bSpeedhack);
            if (g_Config.misc.bSpeedhack)
            {
                ImGui::SliderFloat("Speed Multiplier", &g_Config.misc.fSpeedMultiplier, 1.0f, 10.0f, "%.1fx");
            }
            RenderKeyBindWidget("Speedhack Hotkey", g_Config.settings.keySpeedhack, 120.0f);
        }
        EndCard();
    }

    void Menu::RenderWorldTab()
    {
        float contentW = ImGui::GetContentRegionAvail().x;
        float colW = (contentW - 12.0f) * 0.5f;

        // Left Card: Environment, Lighting & Weather
        BeginCard("Card_World_Environment", "Environment & Weather", ImVec2(colW, 0));
        {
            ImGui::TextColored(ImVec4(0.95f, 0.68f, 0.15f, 1.0f), "Lighting & Atmosphere:");
            ImGui::Checkbox("Nightmode (Cold Night Colors)", &g_Config.environment.bNightmode);
            if (g_Config.environment.bNightmode)
            {
                ImGui::SameLine();
                ImGui::ColorEdit4("##ColNight", reinterpret_cast<float*>(&g_Config.environment.colNightmode), ImGuiColorEditFlags_NoInputs);
            }

            ImGui::Checkbox("Custom Skybox Color", &g_Config.environment.bCustomSkybox);
            if (g_Config.environment.bCustomSkybox)
            {
                ImGui::SameLine();
                ImGui::ColorEdit4("##ColSky", reinterpret_cast<float*>(&g_Config.environment.colSkybox), ImGuiColorEditFlags_NoInputs);
            }

            ImGui::Checkbox("Custom Ambient Color", &g_Config.environment.bCustomAmbient);
            if (g_Config.environment.bCustomAmbient)
            {
                ImGui::SameLine();
                ImGui::ColorEdit4("##ColAmb", reinterpret_cast<float*>(&g_Config.environment.colAmbient), ImGuiColorEditFlags_NoInputs);
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::TextColored(ImVec4(0.95f, 0.68f, 0.15f, 1.0f), "Time of Day (24h Clock):");
            ImGui::Checkbox("Override Time of Day", &g_Config.environment.bTimeOverride);
            if (g_Config.environment.bTimeOverride)
            {
                ImGui::SliderFloat("Time (Hours)", &g_Config.environment.fTimeOfDay, 0.0f, 24.0f, "%.1f h");

                float btnW = (ImGui::GetContentRegionAvail().x - 6.0f) * 0.5f;
                if (ImGui::Button("Morning (06:00)", ImVec2(btnW, 24)))
                    g_Config.environment.fTimeOfDay = 6.0f;
                ImGui::SameLine();
                if (ImGui::Button("Noon (12:00)", ImVec2(btnW, 24)))
                    g_Config.environment.fTimeOfDay = 12.0f;

                if (ImGui::Button("Evening (19:00)", ImVec2(btnW, 24)))
                    g_Config.environment.fTimeOfDay = 19.0f;
                ImGui::SameLine();
                if (ImGui::Button("Night (01:00)", ImVec2(btnW, 24)))
                    g_Config.environment.fTimeOfDay = 1.0f;
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::TextColored(ImVec4(0.95f, 0.68f, 0.15f, 1.0f), "Weather Controller:");
            const char* weathers[] = { "Default (Game)", "Default Weather", "Level Map Weather", "Rainy Weather", "Radar Atmosphere", "Yantar Misty" };
            ImGui::SetNextItemWidth(-1.0f);
            ImGui::Combo("##WeatherPreset", &g_Config.environment.iSelectedWeather, weathers, IM_ARRAYSIZE(weathers));
        }
        EndCard();

        ImGui::SameLine();

        // Right Card: Engine Diagnostics
        BeginCard("Card_World_Diagnostics", "X-Ray Engine 1.0006 Diagnostics", ImVec2(colW, 0));
        {
            XRay::CRenderDevice* pDev = XRay::Engine::GetDevice();
            ImGui::Text("Render Device: %s (0x%p)", pDev ? "Connected" : "Not Found", pDev);
            if (pDev)
            {
                XRay::Fvector camPos = XRay::Engine::GetCameraPosition();
                ImGui::Text("Camera Pos:    (X: %.1f, Y: %.1f, Z: %.1f)", camPos.x, camPos.y, camPos.z);
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
        }
        EndCard();
    }

    void Menu::RenderSettingsTab()
    {
        float contentW = ImGui::GetContentRegionAvail().x;
        float colW = (contentW - 12.0f) * 0.5f;

        // Left Card: Config Profiles
        BeginCard("Card_Config_Profiles", "Configuration Profiles (.INI)", ImVec2(colW, 0));
        {
            std::vector<std::string> configs = g_Config.GetAvailableConfigs();
            static int selectedCfgIdx = 0;

            std::vector<const char*> configNames;
            for (const auto& c : configs)
                configNames.push_back(c.c_str());

            ImGui::Text("Select Profile:");
            ImGui::SetNextItemWidth(-1.0f);
            if (ImGui::Combo("##ConfigSelect", &selectedCfgIdx, configNames.data(), static_cast<int>(configNames.size())))
            {
                if (selectedCfgIdx >= 0 && selectedCfgIdx < static_cast<int>(configs.size()))
                {
                    strncpy_s(g_Config.settings.szSelectedConfig, configs[selectedCfgIdx].c_str(), sizeof(g_Config.settings.szSelectedConfig) - 1);
                }
            }

            ImGui::Spacing();
            if (ImGui::Button("Load Profile", ImVec2(120 * g_Config.settings.fUiScale, 28)))
            {
                if (selectedCfgIdx >= 0 && selectedCfgIdx < static_cast<int>(configs.size()))
                {
                    g_Config.LoadConfig(configs[selectedCfgIdx].c_str());
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Save Current", ImVec2(120 * g_Config.settings.fUiScale, 28)))
            {
                if (selectedCfgIdx >= 0 && selectedCfgIdx < static_cast<int>(configs.size()))
                {
                    g_Config.SaveConfig(configs[selectedCfgIdx].c_str());
                }
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("Create New Profile:");
            ImGui::SetNextItemWidth(-1.0f);
            ImGui::InputTextWithHint("##NewCfgName", "Profile Name...", g_Config.settings.szNewConfigName, sizeof(g_Config.settings.szNewConfigName));

            ImGui::Spacing();
            if (ImGui::Button("Create & Save", ImVec2(-1.0f, 28)))
            {
                if (g_Config.settings.szNewConfigName[0] != '\0')
                {
                    g_Config.SaveConfig(g_Config.settings.szNewConfigName);
                    g_Config.settings.szNewConfigName[0] = '\0';
                }
            }

            ImGui::Spacing();
            if (ImGui::Button("Reset All Settings to Defaults", ImVec2(-1.0f, 26)))
            {
                g_Config.ResetToDefaults();
            }
        }
        EndCard();

        ImGui::SameLine();

        // Right Card: Keybinds & Lifecycle
        BeginCard("Card_Settings_Keys", "Keybinds & Cheat Lifecycle", ImVec2(colW, 0));
        {
            RenderKeyBindWidget("Menu Toggle Key", g_Config.settings.keyMenu, 120.0f);
            RenderKeyBindWidget("Eject / Unload Key", g_Config.settings.keyUnload, 120.0f);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("UI Scaling:");
            ImGui::SliderFloat("Scale Factor", &g_Config.settings.fUiScale, 0.75f, 1.50f, "%.2fx");

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Eject DLL From XR_3DA.exe", ImVec2(-1.0f, 32)))
            {
                Core::Logger::Warning("Eject requested via menu button!");
                Core::g_bUnloadRequested.store(true);
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Safely unhooks Direct3D 9, DirectInput, and WndProc, and unloads DLL.");
        }
        EndCard();
    }

    void Menu::Render()
    {
        // Smooth delta-time fade in / out animation
        static float s_menu_alpha = 0.0f;
        if (g_Config.bShowMenu)
        {
            s_menu_alpha = std::clamp(s_menu_alpha + ImGui::GetIO().DeltaTime * 8.0f, 0.0f, 1.0f);
        }
        else
        {
            s_menu_alpha = std::clamp(s_menu_alpha - ImGui::GetIO().DeltaTime * 8.0f, 0.0f, 1.0f);
        }

        if (s_menu_alpha <= 0.001f)
            return;

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, s_menu_alpha);

        float s = g_Config.settings.fUiScale;
        ImGui::SetNextWindowSize(ImVec2(740.0f * s, 540.0f * s), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("S.T.A.L.K.E.R. - Internal SOC v1.0006 [X-Ray 1.0]", &g_Config.bShowMenu,
                         ImGuiWindowFlags_NoCollapse))
        {
            // Horizontal Navigation Bar
            const char* tabNames[] = {
                "Visuals / ESP",
                "Chams",
                "Aimbot",
                "Trainers",
                "World & Weather",
                "Configs & Binds"
            };
            const int tabCount = IM_ARRAYSIZE(tabNames);

            float totalWidth = ImGui::GetContentRegionAvail().x;
            float tabWidth = (totalWidth - 6.0f * (tabCount - 1)) / tabCount;
            float tabHeight = 32.0f * s;

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
            for (int i = 0; i < tabCount; ++i)
            {
                bool isActive = (g_Config.iActiveTab == i);
                if (isActive)
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.58f, 0.12f, 1.00f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.95f, 0.68f, 0.18f, 1.00f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.08f, 0.08f, 0.09f, 1.00f));
                }
                else
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.19f, 0.21f, 1.00f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.26f, 0.28f, 0.31f, 1.00f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.85f, 0.85f, 1.00f));
                }

                if (ImGui::Button(tabNames[i], ImVec2(tabWidth, tabHeight)))
                {
                    g_Config.iActiveTab = i;
                }

                ImGui::PopStyleColor(3);

                if (i < tabCount - 1)
                    ImGui::SameLine(0, 6.0f);
            }
            ImGui::PopStyleVar();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Render Active Tab Content
            switch (g_Config.iActiveTab)
            {
            case 0: RenderVisualsTab(); break;
            case 1: RenderChamsTab(); break;
            case 2: RenderAimbotTab(); break;
            case 3: RenderTrainersTab(); break;
            case 4: RenderWorldTab(); break;
            case 5: RenderSettingsTab(); break;
            default: break;
            }
        }
        ImGui::End();

        ImGui::PopStyleVar(); // Pop Alpha
    }
}
