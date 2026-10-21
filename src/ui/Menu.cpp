#include "Menu.hpp"
#include "../core/Config.hpp"
#include "../core/Globals.hpp"
#include "../core/Logger.hpp"
#include "../features/Misc.hpp"
#include "../xray/Offsets.hpp"
#include <imgui.h>
#include <algorithm>
#include <vector>
#include <string>

namespace UI
{
    static void DrawMedusaArtifactIcon(ImDrawList* drawList, const ImVec2& center, float radius)
    {
        float t = static_cast<float>(ImGui::GetTime());

        // Outer gravitational pulsating aura (cyan-blue glow)
        float pulse = sinf(t * 2.5f) * 0.15f;
        drawList->AddCircleFilled(center, radius * (1.35f + pulse), IM_COL32(35, 140, 220, 35), 24);
        drawList->AddCircleFilled(center, radius * 0.95f, IM_COL32(50, 190, 255, 75), 24);

        // Luminous inner core
        drawList->AddCircleFilled(center, radius * 0.48f, IM_COL32(180, 240, 255, 210), 20);
        drawList->AddCircleFilled(center, radius * 0.25f, IM_COL32(255, 255, 255, 255), 16);

        // Medusa swirling curved tendrils / arms (amber gold & electric cyan)
        for (int i = 0; i < 6; ++i)
        {
            float angle = i * (6.2831853f / 6.0f) + t * 0.85f;
            float r1 = radius * 0.35f;
            float r2 = radius * 0.75f;
            float r3 = radius * 1.10f;

            ImVec2 p1(center.x + cosf(angle) * r1, center.y + sinf(angle) * r1);
            ImVec2 p2(center.x + cosf(angle + 0.5f) * r2, center.y + sinf(angle + 0.5f) * r2);
            ImVec2 p3(center.x + cosf(angle + 1.0f) * r3, center.y + sinf(angle + 1.0f) * r3);

            ImU32 colTendril = (i % 2 == 0) ? IM_COL32(255, 215, 80, 225) : IM_COL32(80, 225, 255, 235);
            drawList->AddBezierCubic(p1, p2, p3, p3, colTendril, 2.0f);
            drawList->AddCircleFilled(p3, 2.2f, colTendril);
        }
    }

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
                    ImGui::Checkbox("Color by Faction / Relationship", &g_Config.visuals.bSkeletonByRelation);
                    if (g_Config.visuals.bSkeletonByRelation)
                    {
                        ImGui::ColorEdit4("Enemies##SkelEnm", reinterpret_cast<float*>(&g_Config.visuals.colSkeletonEnemy), ImGuiColorEditFlags_NoInputs);
                        ImGui::SameLine(); ImGui::Text("Enemies");
                        ImGui::SameLine(110.0f);
                        ImGui::ColorEdit4("Neutrals##SkelNeu", reinterpret_cast<float*>(&g_Config.visuals.colSkeletonNeutral), ImGuiColorEditFlags_NoInputs);
                        ImGui::SameLine(); ImGui::Text("Neutrals");

                        ImGui::ColorEdit4("Friends##SkelFrd", reinterpret_cast<float*>(&g_Config.visuals.colSkeletonFriend), ImGuiColorEditFlags_NoInputs);
                        ImGui::SameLine(); ImGui::Text("Friends");
                        ImGui::SameLine(110.0f);
                        ImGui::ColorEdit4("Mutants##SkelMut", reinterpret_cast<float*>(&g_Config.visuals.colSkeletonMutant), ImGuiColorEditFlags_NoInputs);
                        ImGui::SameLine(); ImGui::Text("Mutants");
                    }
                    else
                    {
                        ImGui::ColorEdit4("Color##SkelCol", reinterpret_cast<float*>(&g_Config.visuals.colSkeleton), ImGuiColorEditFlags_NoInputs);
                        ImGui::SameLine();
                        ImGui::Text("Skeleton Color");
                    }
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

                ImGui::Indent(15.0f);
                const char* crossStyles[] = { "Classic Cross (+)", "Dot (.)", "Circle (o)", "Target (Circle+Cross)", "T-Shape", "CS-Style" };
                ImGui::Combo("Crosshair Style", &g_Config.visuals.iCrosshairStyle, crossStyles, IM_ARRAYSIZE(crossStyles));

                ImGui::SliderFloat("Size (px)", &g_Config.visuals.fCrosshairSize, 2.0f, 25.0f, "%.0f px");
                if (g_Config.visuals.iCrosshairStyle != 1)
                {
                    ImGui::SliderFloat("Center Gap (px)", &g_Config.visuals.fCrosshairGap, 0.0f, 20.0f, "%.0f px");
                    ImGui::SliderFloat("Thickness (px)", &g_Config.visuals.fCrosshairThickness, 1.0f, 5.0f, "%.1f px");
                }
                ImGui::Checkbox("Center Dot", &g_Config.visuals.bCrosshairDot);
                ImGui::SameLine(120.0f);
                ImGui::Checkbox("Dark Outline", &g_Config.visuals.bCrosshairOutline);
                ImGui::Unindent(15.0f);
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
            const char* chamsStyles[] = {
                "Flat (Solid Unlit)",
                "Wireframe",
                "Shaded / Solid",
                "Pulsating Glow",
                "Glass / Hologram",
                "Rainbow / Chromatic"
            };
            ImGui::Combo("Material Style", &g_Config.chams.iChamsType, chamsStyles, IM_ARRAYSIZE(chamsStyles));

            if (g_Config.chams.iChamsType == 3)
            {
                ImGui::SliderFloat("Glow Pulse Speed", &g_Config.chams.fGlowSpeed, 0.5f, 8.0f, "%.1fx");
            }

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
            ImGui::BulletText("Mesh Filter: Skeletal Stride (24-28 bytes)");
            ImGui::BulletText("Styles: Flat, Wire, Shaded, Glow, Glass, Rainbow");
            ImGui::BulletText("Z-Buffer: Automatic dual-depth pass");
            ImGui::BulletText("Safety: Level transition guard active");
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
            ImGui::Text("Target Selection (Faction / Relation):");
            ImGui::Checkbox("Target Hostile Enemies (Bandits, Monolith, etc.)", &g_Config.aimbot.bTargetEnemies);
            ImGui::Checkbox("Target Neutrals (Duty, Freedom, Ecologists)", &g_Config.aimbot.bTargetNeutrals);
            ImGui::Checkbox("Target Friends / Loners", &g_Config.aimbot.bTargetFriends);
            ImGui::Checkbox("Target Mutants / Monsters", &g_Config.aimbot.bTargetMutants);

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

    void Menu::RenderRageTab()
    {
        float contentW = ImGui::GetContentRegionAvail().x;
        float colW = (contentW - 12.0f) * 0.5f;

        // Left Card: Rage Aimbot & Gunplay
        BeginCard("Card_Rage_Combat", "Rage Gunplay & Auto-Fire", ImVec2(colW, 0));
        {
            ImGui::TextColored(ImVec4(0.95f, 0.40f, 0.20f, 1.0f), "Rage Target Lock:");
            ImGui::Checkbox("Enable Rage Aimbot", &g_Config.rage.bRageAimbot);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Instant 360 degree lock onto targets without angle smoothing.");

            ImGui::Checkbox("Auto-Shoot (Fire on Lock)", &g_Config.rage.bAutoShoot);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Automatically cycles weapon fire as soon as a target is locked.");

            ImGui::Checkbox("Rapid Fire (Machinegun Mode)", &g_Config.rage.bRapidFire);
            if (g_Config.rage.bRapidFire)
            {
                ImGui::SliderFloat("Shot Delay (ms)", &g_Config.rage.fRapidFireDelay, 10.0f, 60.0f, "%.0f ms");
            }

            ImGui::Checkbox("360 deg All-Direction Lock", &g_Config.rage.b360Fov);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Target enemies anywhere around player in 360 degrees.");

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::TextColored(ImVec4(0.95f, 0.40f, 0.20f, 1.0f), "Rage Targeting Filter:");
            const char* rageBones[] = { "Head", "Neck", "Chest", "Pelvis" };
            ImGui::SetNextItemWidth(-1.0f);
            ImGui::Combo("##RageBone", &g_Config.rage.iRageBone, rageBones, IM_ARRAYSIZE(rageBones));

            ImGui::Checkbox("Target Hostile Stalkers", &g_Config.rage.bTargetEnemies);
            ImGui::Checkbox("Target Neutrals (Duty, Freedom)", &g_Config.rage.bTargetNeutrals);
            ImGui::Checkbox("Target Friends / Loners", &g_Config.rage.bTargetFriends);
            ImGui::Checkbox("Target Mutants / Monsters", &g_Config.rage.bTargetMutants);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Checkbox("Проверка стен (Raycast Wall Check)", &g_Config.rage.bVisCheck);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Проверять видимость через стены движком. Предотвращает бесполезную стрельбу в бетон/стены.");

            ImGui::Text("Приоритет выбора цели:");
            const char* ragePriorities[] = { "Ближайший к прицелу (Crosshair)", "Ближайший по расстоянию (3D Nearest)" };
            ImGui::SetNextItemWidth(-1.0f);
            ImGui::Combo("##RagePriority", &g_Config.rage.iTargetPriority, ragePriorities, IM_ARRAYSIZE(ragePriorities));
        }
        EndCard();

        ImGui::SameLine();

        // Right Card: Movement Exploits & FOV
        BeginCard("Card_Rage_Movement", "Movement Exploits & FOV", ImVec2(colW, 0));
        {
            ImGui::TextColored(ImVec4(0.95f, 0.40f, 0.20f, 1.0f), "Field of View (FOV):");
            ImGui::Checkbox("FOV Changer", &g_Config.fov.bEnabled);
            if (g_Config.fov.bEnabled)
            {
                ImGui::SliderFloat("Camera FOV", &g_Config.fov.fFov, 60.0f, 130.0f, "%.1f deg");
                if (ImGui::Button("Reset to Default (67.5 deg)", ImVec2(-1.0f, 24)))
                {
                    g_Config.fov.fFov = 67.5f;
                }
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::TextColored(ImVec4(0.95f, 0.40f, 0.20f, 1.0f), "Teleport Exploits:");
            ImGui::Checkbox("Blink Hack (Forward Teleport)", &g_Config.rage.bBlink);
            if (g_Config.rage.bBlink)
            {
                RenderKeyBindWidget("Blink Hotkey", g_Config.rage.keyBlink, 120.0f);
                ImGui::SliderFloat("Distance (m)", &g_Config.rage.fBlinkDist, 3.0f, 30.0f, "%.0f m");
            }

            ImGui::Spacing();
            ImGui::Checkbox("Backstab Hack (Behind Target)", &g_Config.rage.bBackstab);
            if (g_Config.rage.bBackstab)
            {
                RenderKeyBindWidget("Backstab Hotkey", g_Config.rage.keyBackstab, 120.0f);
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::TextColored(ImVec4(0.95f, 0.40f, 0.20f, 1.0f), "Awareness:");
            ImGui::Checkbox("Off-Screen Radar Arrows", &g_Config.rage.bOffscreenArrows);
            if (g_Config.rage.bOffscreenArrows)
            {
                ImGui::SliderFloat("Arrow Radius (px)", &g_Config.rage.fArrowRadius, 80.0f, 350.0f, "%.0f px");
            }
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

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::TextColored(ImVec4(0.95f, 0.68f, 0.15f, 1.0f), "Third-Person View (Camera):");
            if (ImGui::Checkbox("Enable Third-Person View", &g_Config.misc.bThirdPerson))
            {
                Features::Misc::SetCameraMode(g_Config.misc.bThirdPerson ? g_Config.misc.iThirdPersonMode : 0);
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Switches camera perspective to native 3rd person with weapon/crosshair alignment.");

            RenderKeyBindWidget("Toggle Perspective Hotkey", g_Config.misc.keyThirdPerson, 120.0f);

            const char* camStyles[] = { "Over-The-Shoulder (Look-At)", "Free Look (Far)" };
            int selectedStyle = (g_Config.misc.iThirdPersonMode == 2) ? 1 : 0;
            if (ImGui::Combo("Perspective Style", &selectedStyle, camStyles, IM_ARRAYSIZE(camStyles)))
            {
                g_Config.misc.iThirdPersonMode = (selectedStyle == 1) ? 2 : 1;
                if (g_Config.misc.bThirdPerson)
                {
                    Features::Misc::SetCameraMode(g_Config.misc.iThirdPersonMode);
                }
            }

            ImGui::TextDisabled("Note: Engine hotkeys F1, F2, F3 also bound.");
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

    struct ZoneLordItem {
        const char* label;
        const char* section;
        const char* group;
    };

#if defined(XRAY_COP)
    static const ZoneLordItem s_Weapons[] = {
        { "ПМ (Пистолет Макарова)", "wpn_pm", "Пистолеты" },
        { "ПБ (Бесшумный)", "wpn_pb", "Пистолеты" },
        { "Форт-12", "wpn_fort", "Пистолеты" },
        { "Beretta 92FS (Марта)", "wpn_beretta", "Пистолеты" },
        { "HPSA (Браунинг)", "wpn_hpsa", "Пистолеты" },
        { "Walther P99 (Волкер)", "wpn_walther", "Пистолеты" },
        { "Colt 1911 (Кора)", "wpn_colt1911", "Пистолеты" },
        { "SIG Sauer P220 (СИП)", "wpn_sig220", "Пистолеты" },
        { "USP Compact (УДП)", "wpn_usp", "Пистолеты" },
        { "Desert Eagle (Черный ястреб)", "wpn_desert_eagle", "Пистолеты" },
        { "MP5 (Гадюка-5)", "wpn_mp5", "Дробовики & ПП" },
        { "БМ-16 (Двустволка)", "wpn_bm16", "Дробовики & ПП" },
        { "ТОЗ-34 (Охотничье ружье)", "wpn_toz34", "Дробовики & ПП" },
        { "Winchester 1300 (Чейзер-13)", "wpn_wincheaster1300", "Дробовики & ПП" },
        { "SPAS-12 (СПСА-14)", "wpn_spas12", "Дробовики & ПП" },
        { "Protecta (Отбойник)", "wpn_protecta", "Дробовики & ПП" },
        { "АКС-74У", "wpn_ak74u", "Автоматы" },
        { "АК-74", "wpn_ak74", "Автоматы" },
        { "АН-94 Абакан", "wpn_abakan", "Автоматы" },
        { "L85 (ИЛ 86)", "wpn_l85", "Автоматы" },
        { "LR-300 (ТРс-301)", "wpn_lr300", "Автоматы" },
        { "SIG SG 550 (СГИ-5к)", "wpn_sig550", "Автоматы" },
        { "G36 (ГП 37)", "wpn_g36", "Автоматы" },
        { "ОЦ-14 Гроза", "wpn_groza", "Автоматы" },
        { "АС ВАЛ", "wpn_val", "Автоматы" },
        { "FN F2000 (ФТ-200М)", "wpn_fn2000", "Автоматы" },
        { "ВСС Винторез", "wpn_vintorez", "Снайперские" },
        { "СВД", "wpn_svd", "Снайперские" },
        { "СВУ", "wpn_svu", "Снайперские" },
        { "Гаусс-пушка", "wpn_gauss", "Снайперские" },
        { "ПКМ (Пулемет Калашникова)", "wpn_pkm", "Тяжелое & Взрывчатка" },
        { "РПГ-7", "wpn_rpg7", "Тяжелое & Взрывчатка" },
        { "РГ-6 (Бульдог-6)", "wpn_rg-6", "Тяжелое & Взрывчатка" },
        { "Граната Ф-1", "grenade_f1", "Тяжелое & Взрывчатка" },
        { "Граната РГД-5", "grenade_rgd5", "Тяжелое & Взрывчатка" },
    };

    static const ZoneLordItem s_UniqueWeapons[] = {
        { "ПМ Наградной (Актера)", "wpn_pm_actor", "Пистолеты" },
        { "Форт-12 (Коряги)", "wpn_fort_snag", "Пистолеты" },
        { "SIG Sauer Альпиец (Шустрый)", "wpn_sig220_nimble", "Пистолеты" },
        { "USP Марш (Шустрый)", "wpn_usp_nimble", "Пистолеты" },
        { "Desert Eagle Степной орел (Шустрый)", "wpn_desert_eagle_nimble", "Пистолеты" },
        { "MP5 Фрезер (Шустрый)", "wpn_mp5_nimble", "ПП" },
        { "Winchester Хищник (Зверобой)", "wpn_wincheaster1300_trapper", "Дробовики" },
        { "SPAS-12 Карабинер (Шустрый)", "wpn_spas12_nimble", "Дробовики" },
        { "Protecta Карабинер (Шустрый)", "wpn_protecta_nimble", "Дробовики" },
        { "АКС-74У (Коряги)", "wpn_ak74u_snag", "Автоматы" },
        { "Гроза Шторм (Шустрый)", "wpn_groza_nimble", "Автоматы" },
        { "Гроза Спецназ", "wpn_groza_specops", "Автоматы" },
        { "SIG SG 550 Стрелка", "wpn_sig550_luckygun", "Автоматы" },
        { "G36 Смерч (Шустрый)", "wpn_g36_nimble", "Автоматы" },
        { "FN F2000 (Шустрый)", "wpn_fn2000_nimble", "Автоматы" },
        { "Винторез Прибой (Шустрый)", "wpn_vintorez_nimble", "Снайперские" },
        { "СВУ Громобой (Шустрый)", "wpn_svu_nimble", "Снайперские" },
        { "СВД Рысь (Шустрый)", "wpn_svd_nimble", "Снайперские" },
        { "ПКМ Зулуса", "wpn_pkm_zulus", "Тяжелое" },
        { "Гаусс-пушка (Неизвестное оружие)", "pri_a17_gauss_rifle", "Специальное" },
    };

    static const ZoneLordItem s_Attachments[] = {
        { "Оптический прицел ПСО-1", "wpn_addon_scope", "Прицелы" },
        { "Прицел SUSAT (НАТО)", "wpn_addon_scope_susat", "Прицелы" },
        { "Прицел SUSAT Custom", "wpn_addon_scope_susat_custom", "Прицелы" },
        { "Прицел SUSAT Ночной", "wpn_addon_scope_susat_night", "Прицелы" },
        { "Прицел SUSAT Сумеречный", "wpn_addon_scope_susat_dusk", "Прицелы" },
        { "Прицел НСПУ (Ночной)", "wpn_addon_scope_night", "Прицелы" },
        { "Прицел с детектором", "wpn_addon_scope_detector", "Прицелы" },
        { "Глушитель", "wpn_addon_silencer", "Глушители" },
        { "Подствольник ГП-25 Костер", "wpn_addon_grenade_launcher", "Подствольники" },
        { "Подствольник M203 (НАТО)", "wpn_addon_grenade_launcher_m203", "Подствольники" },
    };

    static const ZoneLordItem s_Ammo[] = {
        { "9x18 мм FMJ", "ammo_9x18_fmj", "Пистолетные" },
        { "9x18 мм PMM", "ammo_9x18_pmm", "Пистолетные" },
        { "9x19 мм FMJ", "ammo_9x19_fmj", "Пистолетные" },
        { "9x19 мм PBP", "ammo_9x19_pbp", "Пистолетные" },
        { ".45 ACP FMJ", "ammo_11.43x23_fmj", "Пистолетные" },
        { ".45 ACP Hydro-Shock", "ammo_11.43x23_hydro", "Пистолетные" },
        { "12x70 Дробь", "ammo_12x70_buck", "Дробовые" },
        { "12x76 Жекан", "ammo_12x76_zhekan", "Дробовые" },
        { "5.45x39 мм FMJ", "ammo_5.45x39_fmj", "Автоматные" },
        { "5.45x39 мм БП", "ammo_5.45x39_ap", "Автоматные" },
        { "5.56x45 мм SS109", "ammo_5.56x45_ss190", "Автоматные" },
        { "5.56x45 мм AP", "ammo_5.56x45_ap", "Автоматные" },
        { "7.62x54 мм 7H1 (Снайперские)", "ammo_7.62x54_7h1", "Винтовочные" },
        { "7.62x54 мм БП", "ammo_7.62x54_ap", "Винтовочные" },
        { "Лента 100 патронов ПКМ", "ammo_pkm_100", "Пулеметные" },
        { "9x39 мм СП-5", "ammo_9x39_sp5", "Специальные" },
        { "9x39 мм СП-6 (Бронебойные)", "ammo_9x39_ap", "Специальные" },
        { "9x39 мм ПАБ-9", "ammo_9x39_pab9", "Специальные" },
        { "Гаусс-патроны", "ammo_gauss", "Специальные" },
        { "Гаусс-патроны Кустарные (Кардан)", "ammo_gauss_cardan", "Специальные" },
        { "Заряд ОГ-7В (РПГ-7)", "ammo_og-7b", "Гранатометные" },
        { "ВОГ-25", "ammo_vog-25", "Гранатометные" },
        { "M209 (НАТО)", "ammo_m209", "Гранатометные" },
    };

    static const ZoneLordItem s_Outfits[] = {
        { "Кожаная куртка", "novice_outfit", "Костюмы" },
        { "Комбинезон 'Заря'", "stalker_outfit", "Костюмы" },
        { "Комбинезон 'Заря' (Баржи)", "stalker_outfit_barge", "Костюмы" },
        { "Комбинезон 'Ветер Свободы'", "svoboda_light_outfit", "Костюмы" },
        { "Бронекостюм 'Страж Свободы'", "svoboda_heavy_outfit", "Костюмы" },
        { "Броня 'Долга' ПС5-М", "dolg_outfit", "Костюмы" },
        { "Тяжелый бронекостюм 'Долга' ПСЗ-9д", "dolg_heavy_outfit", "Костюмы" },
        { "Комбинезон 'СЕВА'", "scientific_outfit", "Научные" },
        { "Бронекостюм ЧН-3а", "cs_heavy_outfit", "Тяжелая броня" },
        { "Армейский бронекостюм Берилл-5М", "specops_outfit", "Военная броня" },
        { "Армейский бронекостюм СКАТ-9М", "military_outfit", "Военная броня" },
        { "Экзоскелет", "exo_outfit", "Экзоскелеты" },
        { "Противогаз", "helm_respirator", "Шлемы" },
        { "Противогаз (Шутника)", "helm_respirator_joker", "Шлемы" },
        { "Стальной шлем", "helm_hardhat", "Шлемы" },
        { "Стальной шлем (Коряги)", "helm_hardhat_snag", "Шлемы" },
        { "Шлем 'Заслон'", "helm_protective", "Шлемы" },
        { "Шлем 'Сфера-08'", "helm_battle", "Шлемы" },
        { "Тактический шлем", "helm_tactic", "Шлемы" },
    };

    static const ZoneLordItem s_MonsterParts[] = {
        { "Детектор 'Отклик'", "detector_simple", "Детекторы" },
        { "Детектор 'Медведь'", "detector_advanced", "Детекторы" },
        { "Детектор 'Велес'", "detector_elite", "Детекторы" },
        { "Детектор 'Сварог'", "detector_scientific", "Детекторы" },
        { "Инструменты для грубой работы", "toolkit_1", "Инструменты" },
        { "Инструменты для тонкой работы", "toolkit_2", "Инструменты" },
        { "Инструменты для калибровки", "toolkit_3", "Инструменты" },
    };

    static const ZoneLordItem s_QuestItems[] = {
        { "Инструменты для грубой работы", "toolkit_1", "Инструменты" },
        { "Инструменты для тонкой работы", "toolkit_2", "Инструменты" },
        { "Инструменты для калибровки", "toolkit_3", "Инструменты" },
        { "Документы по проекту X-8 (№1)", "pri_a15_documents_1", "Документы X-8" },
        { "Документы по проекту X-8 (№2)", "pri_a15_documents_2", "Документы X-8" },
        { "Документы по проекту X-8 (№3)", "pri_a15_documents_3", "Документы X-8" },
        { "Документы по проекту X-8 (№4)", "pri_a15_documents_4", "Документы X-8" },
        { "Документы по проекту X-8 (№5)", "pri_a15_documents_5", "Документы X-8" },
        { "Записка Соколова", "jup_b205_sokolov_note", "Квестовые" },
        { "Карты глубинного рейда", "zat_b100_military_1", "Квестовые" },
        { "Неизвестное оружие (Гаусс)", "pri_a17_gauss_rifle", "Спецпредметы" },
    };

    static const ZoneLordItem s_Artifacts[] = {
        { "Медуза", "af_medusa", "Гравитационные" },
        { "Каменный цветок", "af_cristall_flower", "Гравитационные" },
        { "Ночная звезда", "af_night_star", "Гравитационные" },
        { "Выверт", "af_vyvert", "Гравитационные" },
        { "Грави", "af_gravi", "Гравитационные" },
        { "Золотая рыбка", "af_gold_fish", "Гравитационные" },
        { "Кровь камня", "af_blood", "Органические" },
        { "Ломоть мяса", "af_mincer_meat", "Органические" },
        { "Душа", "af_soul", "Органические" },
        { "Колобок", "af_fuzz_kolobok", "Органические" },
        { "Пузырь", "af_baloon", "Органические" },
        { "Светляк", "af_glass", "Органические" },
        { "Бенгальский огонь", "af_electra_sparkler", "Электрические" },
        { "Вспышка", "af_electra_flash", "Электрические" },
        { "Лунный свет", "af_electra_moonlight", "Электрические" },
        { "Батарейка", "af_dummy_battery", "Электрические" },
        { "Пустышка", "af_dummy_dummy", "Электрические" },
        { "Снежинка", "af_ice", "Электрические" },
        { "Кристалл", "af_cristall", "Термические" },
        { "Огненный шар", "af_fireball", "Термические" },
        { "Мамины бусы", "af_dummy_glassbeads", "Термические" },
        { "Глаз", "af_eye", "Термические" },
        { "Пламя", "af_fire", "Термические" },
        { "Компас", "af_compass", "Уникальные" },
        { "Сердце Оазиса", "af_oasis_heart", "Уникальные" },
    };

    static const ZoneLordItem s_Supplies[] = {
        { "Аптечка (Индивидуальная)", "medkit", "Медицина" },
        { "Аптечка армейская", "medkit_army", "Медицина" },
        { "Аптечка научная", "medkit_scientic", "Медицина" },
        { "Бинт", "bandage", "Медицина" },
        { "Антирад", "antirad", "Медицина" },
        { "Препарат Геркулес", "drug_booster", "Препараты" },
        { "Препарат Барвинок", "drug_coagulant", "Препараты" },
        { "Препарат Радиопротектор", "drug_radioprotector", "Препараты" },
        { "Препарат Пси-блокада", "drug_psy_blockade", "Препараты" },
        { "Препарат Антидот", "drug_antidot", "Препараты" },
        { "Препарат Анабиотик", "drug_anabiotic", "Препараты" },
        { "Водка 'Казаки'", "vodka", "Провизия" },
        { "Энергетик Non Stop", "energy_drink", "Провизия" },
        { "Батон", "bread", "Провизия" },
        { "Колбаса 'Диетическая'", "kolbasa", "Провизия" },
        { "Консервы 'Тушенка'", "conserva", "Провизия" },
        { "Детектор 'Отклик'", "detector_simple", "Детекторы" },
        { "Детектор 'Медведь'", "detector_advanced", "Детекторы" },
        { "Детектор 'Велес'", "detector_elite", "Детекторы" },
        { "Детектор 'Сварог'", "detector_scientific", "Детекторы" },
        { "Инструменты для грубой работы", "toolkit_1", "Инструменты" },
        { "Инструменты для тонкой работы", "toolkit_2", "Инструменты" },
        { "Инструменты для калибровки", "toolkit_3", "Инструменты" },
        { "Фонарик", "device_torch", "Снаряжение" },
    };

    static const ZoneLordItem s_Mutants[] = {
        { "Кровосос (Обычный)", "bloodsucker_normal", "Опасные мутанты" },
        { "Кровосос (Сильный)", "bloodsucker_strong", "Опасные мутанты" },
        { "Снорк (Обычный)", "snork_normal", "Опасные мутанты" },
        { "Снорк (Сильный)", "snork_strong", "Опасные мутанты" },
        { "Псевдогигант", "gigant_normal", "Опасные мутанты" },
        { "Контролер", "m_controller_normal", "Пси-мутанты" },
        { "Полтергейст", "m_poltergeist_normal_tele", "Пси-мутанты" },
        { "Полтергейст огненный", "m_poltergeist_normal_flame", "Пси-мутанты" },
        { "Бюрер", "burer_normal", "Пси-мутанты" },
        { "Химера", "chimera_normal", "Опасные мутанты" },
        { "Псевдособака", "pseudodog_normal", "Звери" },
        { "Пси-собака", "psy_dog_normal", "Звери" },
        { "Слепой пес", "dog_normal", "Звери" },
        { "Кабан (Сильный)", "boar_strong", "Звери" },
        { "Плоть (Обычная)", "flesh_normal", "Звери" },
        { "Тушкан", "tushkano_normal", "Мелкие мутанты" },
    };

    static const ZoneLordItem s_Stalkers[] = {
        { "Одиночка (Новичок)", "sim_default_stalker_0", "Одиночки" },
        { "Одиночка (Опытный)", "sim_default_stalker_1", "Одиночки" },
        { "Одиночка (Ветеран)", "sim_default_stalker_2", "Одиночки" },
        { "Одиночка (Мастер)", "sim_default_stalker_3", "Одиночки" },
        { "Бандит (Новичок)", "sim_default_bandit_0", "Бандиты" },
        { "Бандит (Опытный)", "sim_default_bandit_1", "Бандиты" },
        { "Бандит (Ветеран)", "sim_default_bandit_2", "Бандиты" },
        { "Бандит (Мастер)", "sim_default_bandit_3", "Бандиты" },
        { "Боец 'Долга' (Рядовой)", "sim_default_duty_0", "Долг" },
        { "Боец 'Долга' (Сержант)", "sim_default_duty_1", "Долг" },
        { "Боец 'Долга' (Ветеран)", "sim_default_duty_2", "Долг" },
        { "Боец 'Долга' (Мастер)", "sim_default_duty_3", "Долг" },
        { "Боец 'Свободы' (Рядовой)", "sim_default_freedom_0", "Свобода" },
        { "Боец 'Свободы' (Ветеран)", "sim_default_freedom_2", "Свобода" },
        { "Боец 'Свободы' (Мастер)", "sim_default_freedom_3", "Свобода" },
        { "Наемник (Ветеран)", "sim_default_killer_2", "Наемники" },
        { "Наемник (Мастер)", "sim_default_killer_3", "Наемники" },
        { "Монолитовец (Ветеран)", "sim_default_monolith_2", "Монолит" },
        { "Монолитовец (Мастер)", "sim_default_monolith_3", "Монолит" },
        { "Военный (Спецназ)", "sim_default_military_2", "Военные" },
        { "Военный (Снайпер)", "sim_default_military_3_sniper", "Военные" },
        { "Военный (Командир)", "sim_default_military_4", "Военные" },
        { "Зомбированный (Сталкер)", "sim_default_zombied_1", "Зомбированные" },
        { "Зомбированный (Ветеран)", "sim_default_zombied_3", "Зомбированные" },
    };

    struct ZoneLordTeleport {
        const char* name;
        const char* level;
        float x, y, z;
    };

    static const ZoneLordTeleport s_Teleports[] = {
        { "Затон: Скадовск (Главная палуба)", "Затон", 115.0f, -7.0f, 180.0f },
        { "Затон: Шевченко", "Затон", 170.0f, -4.0f, 50.0f },
        { "Затон: Станция переработки отходов", "Затон", 320.0f, 15.0f, -270.0f },
        { "Затон: Лесопилка", "Затон", -355.0f, 21.0f, 195.0f },
        { "Затон: Железный лес", "Затон", 95.0f, 12.0f, -330.0f },
        { "Затон: Сгоревший хутор", "Затон", -270.0f, 18.0f, -50.0f },
        { "Затон: ВНЗ Круг (Антенны)", "Затон", -130.0f, 25.0f, 50.0f },
        { "Затон: Портовые краны", "Затон", 70.0f, 1.0f, 280.0f },
        { "Юпитер: Станция Янов (Зал ожидания)", "Окрестности Юпитера", -50.0f, 3.5f, 190.0f },
        { "Юпитер: Бункер ученых", "Окрестности Юпитера", -180.0f, -3.0f, -150.0f },
        { "Юпитер: Завод 'Юпитер' (Главный вход)", "Окрестности Юпитера", 200.0f, 5.0f, -200.0f },
        { "Юпитер: Цементный завод", "Окрестности Юпитера", 180.0f, 12.0f, 300.0f },
        { "Юпитер: КПП / Градирня", "Окрестности Юпитера", 240.0f, 15.0f, 40.0f },
        { "Юпитер: Карьер", "Окрестности Юпитера", -20.0f, -20.0f, -80.0f },
        { "Юпитер: Вентиляционный комплекс (Оазис)", "Окрестности Юпитера", -80.0f, -10.0f, -360.0f },
        { "Путепровод Припять-1: Входной тоннель", "Путепровод Припять-1", 0.0f, -30.0f, 0.0f },
        { "Припять: Прачечная (Штаб военных)", "Припять", 160.0f, -0.5f, 10.0f },
        { "Припять: Речной порт", "Припять", 280.0f, -12.0f, -180.0f },
        { "Припять: Универмаг", "Припять", 40.0f, 1.0f, 160.0f },
        { "Припять: КБО 'Юбилейный' (Вход в X-8)", "Припять", -160.0f, 2.0f, 40.0f },
        { "Припять: Кинотеатр 'Прометей'", "Припять", -240.0f, 0.0f, -150.0f },
        { "Припять: Госпиталь", "Припять", -120.0f, 0.0f, 240.0f },
        { "Припять: Школа", "Припять", 70.0f, 2.0f, -70.0f },
        { "Припять: Детский сад", "Припять", 130.0f, 1.0f, -140.0f },
        { "Лаборатория X-8: Центральный зал", "Лаборатория X-8", -50.0f, -25.0f, 15.0f },
    };
#else
    static const ZoneLordItem s_Weapons[] = {
        { "ПМ (Пистолет Макарова)", "wpn_pm", "Пистолеты" },
        { "ПБ (Бесшумный)", "wpn_pb", "Пистолеты" },
        { "Форт-12", "wpn_fort", "Пистолеты" },
        { "Beretta 92FS (Марта)", "wpn_beretta", "Пистолеты" },
        { "HPSA (Браунинг)", "wpn_hpsa", "Пистолеты" },
        { "Walther P99 (Волкер)", "wpn_walther", "Пистолеты" },
        { "Colt 1911 (Кора)", "wpn_colt1911", "Пистолеты" },
        { "SIG Sauer P220 (СИП)", "wpn_sig220", "Пистолеты" },
        { "USP Compact (УДП)", "wpn_usp", "Пистолеты" },
        { "Desert Eagle (Черный ястреб)", "wpn_desert_eagle", "Пистолеты" },
        { "MP5 (Гадюка-5)", "wpn_mp5", "Дробовики & ПП" },
        { "БМ-16 (Обрез)", "wpn_bm16", "Дробовики & ПП" },
        { "ТОЗ-34 (Охотничье ружье)", "wpn_toz34", "Дробовики & ПП" },
        { "Winchester 1300 (Чейзер-13)", "wpn_wincheaster1300", "Дробовики & ПП" },
        { "SPAS-12 (СПСА-14)", "wpn_spas12", "Дробовики & ПП" },
        { "АКС-74У", "wpn_ak74u", "Автоматы" },
        { "АК-74", "wpn_ak74", "Автоматы" },
        { "АН-94 Абакан", "wpn_abakan", "Автоматы" },
        { "L85 (ИЛ 86)", "wpn_l85", "Автоматы" },
        { "LR-300 (ТРс-301)", "wpn_lr300", "Автоматы" },
        { "SIG SG 550 (СГИ-5к)", "wpn_sig550", "Автоматы" },
        { "G36 (ГП 37)", "wpn_g36", "Автоматы" },
        { "ОЦ-14 Гроза", "wpn_groza", "Автоматы" },
        { "АС ВАЛ", "wpn_val", "Автоматы" },
        { "FN F2000 (ФТ-200М)", "wpn_fn2000", "Автоматы" },
        { "ВСС Винторез", "wpn_vintorez", "Снайперские" },
        { "СВД", "wpn_svd", "Снайперские" },
        { "СВУ", "wpn_svu", "Снайперские" },
        { "Гаусс-пушка", "wpn_gauss", "Снайперские" },
        { "РПГ-7", "wpn_rpg7", "Тяжелое & Взрывчатка" },
        { "РГ-6 (Бульдог-6)", "wpn_rg-6", "Тяжелое & Взрывчатка" },
        { "Граната Ф-1", "grenade_f1", "Тяжелое & Взрывчатка" },
        { "Граната РГД-5", "grenade_rgd5", "Тяжелое & Взрывчатка" },
    };

    static const ZoneLordItem s_UniqueWeapons[] = {
        { "АКС-74У Скорострельный (Бес)", "wpn_ak74u_m1", "Автоматы" },
        { "Винторез Снайперский (Коллекционный)", "wpn_vintorez_coll", "Снайперские" },
        { "ОЦ-14-4 Гроза 5.45 (Воронин)", "wpn_groza_m1", "Автоматы" },
        { "LR-300 Снайперская", "wpn_lr300_m1", "Автоматы" },
        { "SIG SG 550 Штурмовой", "wpn_sig_m1", "Автоматы" },
        { "SIG SG 550 Снайперский", "wpn_sig_m2", "Автоматы" },
        { "G36 Модифицированная", "wpn_g36_m1", "Автоматы" },
        { "L85 Балансированная", "wpn_l85_m1", "Автоматы" },
        { "L85 Снайперская", "wpn_l85_m2", "Автоматы" },
        { "MP5 9x18 (Гадюка Юрика)", "wpn_mp5_m1", "ПП" },
        { "MP5 Бесшумная", "wpn_mp5_m2", "ПП" },
        { "Абакан Штурмовой", "wpn_abakan_m1", "Автоматы" },
        { "Абакан Снайперский", "wpn_abakan_m2", "Автоматы" },
        { "Кольт 1911 Модифицированный", "wpn_colt_m1", "Пистолеты" },
        { "Desert Eagle Большой Бэн (.39)", "wpn_eagle_m1", "Пистолеты" },
        { "Форт-12 Усиленный", "wpn_fort_m1", "Пистолеты" },
        { "Walther P99 9x18", "wpn_walther_m1", "Пистолеты" },
        { "Winchester Боевой чейзер", "wpn_winchester_m1", "Дробовики" },
        { "СПСА-14 Нарезной", "wpn_spas_m1", "Дробовики" },
        { "СВУ Снайперская (Макс)", "wpn_svu_m1", "Снайперские" }
    };

    static const ZoneLordItem s_Attachments[] = {
        { "Оптический прицел ПСО-1", "wpn_addon_scope", "Прицелы" },
        { "Прицел SUSAT (НАТО)", "wpn_addon_scope_susat", "Прицелы" },
        { "Глушитель 5.45/5.56/9mm", "wpn_addon_silencer", "Глушители" },
        { "Подствольник ГП-25 Костер", "wpn_addon_grenade_launcher", "Подствольники" },
        { "Подствольник M203 (НАТО)", "wpn_addon_grenade_launcher_m203", "Подствольники" }
    };

    static const ZoneLordItem s_Ammo[] = {
        { "9x18 мм FMJ", "ammo_9x18_fmj", "Пистолетные" },
        { "9x18 мм PMM", "ammo_9x18_pmm", "Пистолетные" },
        { "9x19 мм FMJ", "ammo_9x19_fmj", "Пистолетные" },
        { "9x19 мм PBP", "ammo_9x19_pbp", "Пистолетные" },
        { ".45 ACP FMJ", "ammo_11.43x23_fmj", "Пистолетные" },
        { ".45 ACP Hydro-Shock", "ammo_11.43x23_hydro", "Пистолетные" },
        { "12x70 Дробь", "ammo_12x70_buck", "Дробовые" },
        { "12x76 Дротик", "ammo_12x76_dart", "Дробовые" },
        { "12x76 Жекан", "ammo_12x76_zhekan", "Дробовые" },
        { "5.45x39 мм FMJ", "ammo_5.45x39_fmj", "Автоматные" },
        { "5.45x39 мм БП", "ammo_5.45x39_ap", "Автоматные" },
        { "5.56x45 мм SS109", "ammo_5.56x45_ss190", "Автоматные" },
        { "5.56x45 мм AP", "ammo_5.56x45_ap", "Автоматные" },
        { "7.62x54 мм 7H1 (Снайперские)", "ammo_7.62x54_7h1", "Винтовочные" },
        { "7.62x54 мм БП", "ammo_7.62x54_ap", "Винтовочные" },
        { "9x39 мм СП-5", "ammo_9x39_sp5", "Специальные" },
        { "9x39 мм СП-6 (Бронебойные)", "ammo_9x39_ap", "Специальные" },
        { "9x39 мм ПАБ-9", "ammo_9x39_pab9", "Специальные" },
        { "Гаусс-патроны", "ammo_gauss", "Специальные" },
        { "Заряд ОГ-7В (РПГ-7)", "ammo_og-7b", "Гранатометные" },
        { "ВОГ-25", "ammo_vog-25", "Гранатометные" },
        { "ВОГ-25П", "ammo_vog-25p", "Гранатометные" },
        { "M209 (НАТО)", "ammo_m209", "Гранатометные" },
    };

    static const ZoneLordItem s_Outfits[] = {
        { "Кожаная куртка новичка", "novice_outfit", "Легкая броня" },
        { "Бандитская куртка", "bandit_outfit", "Легкая броня" },
        { "Куртка бандита (Укрепленная)", "outfit_bandit_m1", "Уникальная броня" },
        { "Кольчужная куртка", "outfit_stalker_m1", "Уникальная броня" },
        { "Комбинезон 'Заря'", "stalker_outfit", "Средняя броня" },
        { "Комбинезон Призрака", "ghost_outfit", "Уникальная броня" },
        { "Лечебный Берилл", "outfit_specops_m1", "Уникальная броня" },
        { "Комбинезон 'СЕВА'", "scientific_outfit", "Научные костюмы" },
        { "Научный комбез СПС-99 (Эколог)", "ecolog_outfit", "Научные костюмы" },
        { "Научный комбез СПС-99М (Зеленый)", "protection_outfit", "Научные костюмы" },
        { "Комбинезон 'СЕВА' Монолита", "monolit_scientific_outfit", "Научные костюмы" },
        { "Экзоскелет", "exo_outfit", "Тяжелая броня" },
        { "Экзоскелет 'Монолита'", "monolit_exoskeleton", "Тяжелая броня" },
        { "Армейский бронекостюм СКАТ-9М", "military_outfit", "Военные костюмы" },
        { "Армейский бронекостюм СКАТ-9М (Спецназ)", "military_specops_outfit", "Военные костюмы" },
        { "Берилл-5М", "specops_outfit", "Военные костюмы" },
        { "Комбинезон 'Долга'", "dolg_outfit", "Группировки" },
        { "Броня 'Долга' ПСЗ-9Д", "dolg_scientific_outfit", "Группировки" },
        { "Комбинезон 'Свободы' (Ветер)", "svoboda_light_outfit", "Группировки" },
        { "Страж 'Свободы'", "svoboda_heavy_outfit", "Группировки" },
        { "Комбинезон Наемника", "killer_outfit", "Группировки" },
        { "Комбинезон 'Монолита'", "monolit_outfit", "Группировки" },
    };

    static const ZoneLordItem s_MonsterParts[] = {
        { "Хвост псевдособаки", "mutant_psevdodog_tail", "Трофеи" },
        { "Щупальца кровососа", "mutant_krovosos_jaw", "Трофеи" },
        { "Рука излома/бюрера", "mutant_burer_hand", "Трофеи" },
        { "Глаз плоти", "mutant_flesh_eye", "Трофеи" },
        { "Копыто кабана", "mutant_boar_leg", "Трофеи" },
        { "Хвост собаки", "mutant_dog_tail", "Трофеи" },
        { "Рука зомби / контролера", "mutant_zombie_hand", "Трофеи" },
        { "Рука псевдогиганта", "mutant_psevdogigant_hand", "Трофеи" },
        { "Стопа снорка", "mutant_snork_leg", "Трофеи" },
    };

    static const ZoneLordItem s_QuestItems[] = {
        { "Флешка Стрелка", "gunslinger_flash", "Документы & Флешки" },
        { "Документы из Агропрома", "quest_case_01", "Документы & Флешки" },
        { "Документы из X-18", "lab_x18_documents", "Документы & Флешки" },
        { "Документы из X-16", "lab_x16_documents", "Документы & Флешки" },
        { "Декодер тайника Припяти", "decoder", "Ключевые предметы" },
        { "КПК Призрака", "kruglov_flash", "Документы & Флешки" },
        { "КПК Борова", "borov_flash", "Документы & Флешки" },
        { "Флешка сумасшедшего", "crazy_flash", "Документы & Флешки" },
        { "Динамит Волкодава", "dynamite", "Спецпредметы" },
        { "Гитара Сидоровича", "guitar_a", "Разное" },
        { "Настроенный Пси-шлем", "good_psy_helmet", "Спецпредметы" },
        { "Ненастроенный Пси-шлем", "bad_psy_helmet", "Спецпредметы" },
    };

    static const ZoneLordItem s_Artifacts[] = {
        { "Медуза", "af_medusa", "Гравитационные" },
        { "Каменный цветок", "af_cristall_flower", "Гравитационные" },
        { "Ночная звезда", "af_night_star", "Гравитационные" },
        { "Выверт", "af_vyvert", "Гравитационные" },
        { "Грави", "af_gravi", "Гравитационные" },
        { "Золотая рыбка", "af_gold_fish", "Гравитационные" },
        { "Кровь камня", "af_blood", "Химические" },
        { "Ломоть мяса", "af_mincer_meat", "Химические" },
        { "Душа", "af_soul", "Химические" },
        { "Бенгальский огонь", "af_electra_sparkler", "Электрические" },
        { "Вспышка", "af_electra_flash", "Электрические" },
        { "Лунный свет", "af_electra_moonlight", "Электрические" },
        { "Капли", "af_drops", "Термические" },
        { "Огненный шар", "af_fireball", "Термические" },
        { "Кристалл", "af_cristall", "Термические" },
        { "Колючка", "af_rusty_thorn", "Радиационные" },
        { "Кристальная колючка", "af_rusty_kristall", "Радиационные" },
        { "Морской еж", "af_rusty_sea-urchin", "Радиационные" },
        { "Пружина", "af_dummy_spring", "Пустышки" },
        { "Пустышка", "af_dummy_dummy", "Пустышки" },
        { "Батарейка", "af_dummy_battery", "Пустышки" },
        { "Пленка", "af_dummy_pellicle", "Пустышки" },
        { "Мамины бусы", "af_dummy_glassbeads", "Пустышки" },
    };

    static const ZoneLordItem s_Supplies[] = {
        { "Аптечка (Индивидуальная)", "medkit", "Медицина" },
        { "Аптечка армейская", "medkit_army", "Медицина" },
        { "Аптечка научная", "medkit_scientic", "Медицина" },
        { "Бинт", "bandage", "Медицина" },
        { "Антирад", "antirad", "Медицина" },
        { "Водка 'Казаки'", "vodka", "Провизия" },
        { "Энергетик Non Stop", "energy_drink", "Провизия" },
        { "Батон", "bread", "Провизия" },
        { "Колбаса 'Диетическая'", "kolbasa", "Провизия" },
        { "Консервы 'Тушенка'", "conserva", "Провизия" },
        { "Фонарик", "device_torch", "Снаряжение" },
        { "Бинокль", "binocular_a", "Снаряжение" },
        { "Детектор аномалий", "detector_simple", "Снаряжение" },
        { "Гитара", "guitar_a", "Разное" },
        { "Гармошка", "harmonica_a", "Разное" },
    };

    static const ZoneLordItem s_Mutants[] = {
        { "Кровосос (Обычный)", "bloodsucker_normal", "Опасные мутанты" },
        { "Кровосос (Сильный)", "bloodsucker_strong", "Опасные мутанты" },
        { "Снорк (Обычный)", "snork_normal", "Опасные мутанты" },
        { "Снорк (Сильный)", "snork_strong", "Опасные мутанты" },
        { "Псевдогигант", "gigant_normal", "Опасные мутанты" },
        { "Контролер", "m_controller_normal", "Пси-мутанты" },
        { "Полтергейст", "m_poltergeist_normal_tele", "Пси-мутанты" },
        { "Бюрер", "burer_normal", "Пси-мутанты" },
        { "Химера", "chimera_normal", "Опасные мутанты" },
        { "Псевдособака", "pseudodog_normal", "Звери" },
        { "Пси-собака", "psy_dog", "Звери" },
        { "Слепой пес", "dog_normal", "Звери" },
        { "Кабан (Сильный)", "boar_strong", "Звери" },
        { "Плоть (Обычная)", "flesh_normal", "Звери" },
        { "Зомби гражданский", "zombie_normal", "Зомбированные" },
        { "Тушкан", "tushkano_normal", "Мелкие мутанты" },
    };

    static const ZoneLordItem s_Stalkers[] = {
        { "Одиночка (Новичок)", "stalker_novice", "Одиночки" },
        { "Одиночка (Ветеран)", "stalker_veteran", "Одиночки" },
        { "Одиночка (Мастер)", "stalker_master", "Одиночки" },
        { "Бандит (Обычный)", "sim_bandit_general", "Бандиты" },
        { "Бандит (Мастер)", "sim_bandit_master", "Бандиты" },
        { "Боец 'Долга' (Рядовой)", "sim_dolg_regular", "Долг" },
        { "Боец 'Долга' (Ветеран)", "sim_dolg_veteran", "Долг" },
        { "Боец 'Долга' (Мастер)", "sim_dolg_master", "Долг" },
        { "Боец 'Свободы' (Ветеран)", "sim_svoboda_veteran", "Свобода" },
        { "Боец 'Свободы' (Мастер)", "sim_svoboda_master", "Свобода" },
        { "Наемник (Спецназ)", "sim_killer_specnaz", "Наемники" },
        { "Наемник (Мастер)", "sim_killer_master", "Наемники" },
        { "Монолитовец (Рядовой)", "sim_monolith_regular", "Монолит" },
        { "Монолитовец (Мастер)", "sim_monolith_master", "Монолит" },
        { "Военный (Рядовой)", "sim_Soldier_regular", "Военные" },
        { "Военный спецназ", "sim_Soldier_specnaz", "Военные" },
        { "Военный командир", "sim_Soldier_commander", "Военные" },
        { "Ученый (Эколог)", "sim_ecolog_normal", "Экологи" },
    };

    struct ZoneLordTeleport {
        const char* name;
        const char* level;
        float x, y, z;
    };

    static const ZoneLordTeleport s_Teleports[] = {
        { "Деревня Новичков (Бункер Сидоровича)", "Кордон", -211.5f, -20.0f, -145.0f },
        { "Блокпост Военных", "Кордон", -150.0f, -30.0f, -360.0f },
        { "Железнодорожный мост", "Кордон", 35.0f, 3.0f, -50.0f },
        { "Стоянка брошенной техники", "Свалка", 40.0f, 0.5f, -80.0f },
        { "Ангар (Депо)", "Свалка", -10.0f, 1.5f, 90.0f },
        { "Блокпост Долга", "Свалка", 50.0f, 2.0f, 220.0f },
        { "База Сталкеров", "НИИ Агропром", 10.0f, 2.0f, 10.0f },
        { "Комплекс НИИ", "НИИ Агропром", -145.0f, 1.0f, -165.0f },
        { "Бар '100 Рентген'", "Бар Росток", 120.0f, -4.5f, 30.0f },
        { "Арена", "Бар Росток", 155.0f, -3.0f, 45.0f },
        { "База Борова", "Темная Долина", 100.0f, 0.5f, -240.0f },
        { "Лаборатория X-18 (Вход)", "Темная Долина", 115.0f, -2.0f, -300.0f },
        { "Бункер Сахарова", "Янтарь", 30.0f, -11.5f, -270.0f },
        { "База 'Свободы' (Главные ворота)", "Армейские Склады", -25.0f, -12.0f, 60.0f },
        { "Барьер", "Армейские Склады", 120.0f, -5.0f, 290.0f },
        { "Антенны / Выжигатель", "Радар", 380.0f, -30.0f, 150.0f },
        { "Площадь / Гостиница 'Полесье'", "Припять", 15.0f, 0.5f, 260.0f },
        { "Саркофаг ЧАЭС", "ЧАЭС", 25.0f, 0.0f, 60.0f },
    };
#endif

    void Menu::RenderZoneLordTab()
    {
        static int s_selectedCategory = 0;
        static char s_searchFilter[64] = "";
        static int s_spawnCount = 1;
        static bool s_toInventory = true;
        static float s_spawnDistance = 4.0f;
        static int s_customMoney = 50000;
        static std::string s_lastActionMsg = "";
        static float s_lastActionTime = 0.0f;

        float contentW = ImGui::GetContentRegionAvail().x;
        float leftColW = 180.0f * g_Config.settings.fUiScale;
        float rightColW = contentW - leftColW - 12.0f;

        // Left Card: Category Navigation
        BeginCard("Card_ZoneLord_Categories", "Категории", ImVec2(leftColW, 0));
        {
            const char* categories[] = {
                "Оружие",
                "Уникальное оружие",
                "Обвесы & Прицелы",
                "Боеприпасы",
                "Броня & Костюмы",
                "Артефакты",
                "Медицина & Еда",
                "Части монстров",
                "Квестовые предметы",
                "Мутанты & Монстры",
                "Сталкеры & NPC",
                "Деньги (Рубли)",
                "Телепортация",
                "Погода & Время"
            };

            for (int i = 0; i < IM_ARRAYSIZE(categories); ++i)
            {
                bool isSelected = (s_selectedCategory == i);
                if (isSelected)
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.58f, 0.12f, 1.00f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.08f, 0.08f, 0.09f, 1.00f));
                }
                else
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.19f, 0.21f, 1.00f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.85f, 0.85f, 1.00f));
                }

                if (ImGui::Button(categories[i], ImVec2(-1.0f, 28.0f * g_Config.settings.fUiScale)))
                {
                    s_selectedCategory = i;
                }
                ImGui::PopStyleColor(2);
            }
        }
        EndCard();

        ImGui::SameLine();

        // Right Card: Items / Spawner Controls
        BeginCard("Card_ZoneLord_Content", "Спавнер & Функции Зоны", ImVec2(rightColW, 0));
        {
            ImVec2 spawnerHeader = ImGui::GetCursorScreenPos();
            DrawMedusaArtifactIcon(ImGui::GetWindowDrawList(), ImVec2(spawnerHeader.x + 12.0f, spawnerHeader.y + 8.0f), 8.0f);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 26.0f);
            ImGui::TextColored(ImVec4(0.35f, 0.85f, 1.0f, 1.0f), "Повелитель Зоны [Medusa Spawner]");
            ImGui::Spacing();

            // Category 11: Money
            if (s_selectedCategory == 11)
            {
                ImGui::TextColored(ImVec4(0.95f, 0.68f, 0.15f, 1.0f), "Выдача денег (Рубли):");
                ImGui::Spacing();

                float btnW = (ImGui::GetContentRegionAvail().x - 12.0f) / 3.0f;
                if (ImGui::Button("+5 000 RU", ImVec2(btnW, 32)))
                {
                    XRay::Script::GiveMoney(5000);
                    s_lastActionMsg = "Выдано: +5 000 RU";
                }
                ImGui::SameLine();
                if (ImGui::Button("+10 000 RU", ImVec2(btnW, 32)))
                {
                    XRay::Script::GiveMoney(10000);
                    s_lastActionMsg = "Выдано: +10 000 RU";
                }
                ImGui::SameLine();
                if (ImGui::Button("+50 000 RU", ImVec2(btnW, 32)))
                {
                    XRay::Script::GiveMoney(5000);
                    s_lastActionMsg = "Выдано: +50 000 RU";
                }

                if (ImGui::Button("+100 000 RU", ImVec2(btnW, 32)))
                {
                    XRay::Script::GiveMoney(100000);
                    s_lastActionMsg = "Выдано: +100 000 RU";
                }
                ImGui::SameLine();
                if (ImGui::Button("+500 000 RU", ImVec2(btnW, 32)))
                {
                    XRay::Script::GiveMoney(500000);
                    s_lastActionMsg = "Выдано: +500 000 RU";
                }
                ImGui::SameLine();
                if (ImGui::Button("+1 000 000 RU", ImVec2(btnW, 32)))
                {
                    XRay::Script::GiveMoney(1000000);
                    s_lastActionMsg = "Выдано: +1 000 000 RU";
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::Text("Пользовательская сумма:");
                ImGui::SetNextItemWidth(200.0f);
                ImGui::InputInt("##CustomMoney", &s_customMoney, 1000, 10000);
                ImGui::SameLine();
                if (ImGui::Button("Выдать сумму", ImVec2(140, 24)))
                {
                    if (s_customMoney > 0)
                    {
                        XRay::Script::GiveMoney(s_customMoney);
                        s_lastActionMsg = "Выдано: +" + std::to_string(s_customMoney) + " RU";
                    }
                }
            }
            // Category 12: Fast Travel
            else if (s_selectedCategory == 12)
            {
                ImGui::TextColored(ImVec4(0.95f, 0.68f, 0.15f, 1.0f), "Быстрое перемещение (Телепортация):");
                ImGui::TextDisabled("Телепортирует Меченого в ключевые точки ЧЗО.");
                ImGui::Spacing();

                ImGui::BeginChild("TeleportsScroll", ImVec2(0, -35), true);
                for (int i = 0; i < IM_ARRAYSIZE(s_Teleports); ++i)
                {
                    const auto& tp = s_Teleports[i];
                    ImGui::PushID(i);

                    ImGui::TextColored(ImVec4(0.95f, 0.80f, 0.30f, 1.0f), "[%s]", tp.level);
                    ImGui::SameLine();
                    ImGui::Text("%s", tp.name);

                    ImGui::SameLine(ImGui::GetWindowWidth() - 110.0f);
                    if (ImGui::Button("Телепорт", ImVec2(90, 22)))
                    {
                        XRay::Script::Teleport(tp.x, tp.y, tp.z);
                        s_lastActionMsg = std::string("Телепортация: ") + tp.name;
                    }
                    ImGui::Separator();
                    ImGui::PopID();
                }
                ImGui::EndChild();
            }
            // Category 13: Weather & Time
            else if (s_selectedCategory == 13)
            {
                ImGui::TextColored(ImVec4(0.95f, 0.68f, 0.15f, 1.0f), "Смена погоды:");
                ImGui::Spacing();

                float btnW = (ImGui::GetContentRegionAvail().x - 8.0f) * 0.5f;
                if (ImGui::Button("Ясно (Default)", ImVec2(btnW, 30)))
                {
                    XRay::Script::SetWeather("default");
                    s_lastActionMsg = "Установлена погода: default (Ясно)";
                }
                ImGui::SameLine();
                if (ImGui::Button("Туман (Foggy)", ImVec2(btnW, 30)))
                {
                    XRay::Script::SetWeather("map_spots");
                    s_lastActionMsg = "Установлена погода: туман";
                }

                if (ImGui::Button("Дождь (Rain)", ImVec2(btnW, 30)))
                {
                    XRay::Script::SetWeather("rain");
                    s_lastActionMsg = "Установлена погода: дождь";
                }
                ImGui::SameLine();
                if (ImGui::Button("Гроза (Storm)", ImVec2(btnW, 30)))
                {
                    XRay::Script::SetWeather("storm");
                    s_lastActionMsg = "Установлена погода: гроза";
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::TextColored(ImVec4(0.95f, 0.68f, 0.15f, 1.0f), "Скорость течения времени (Time Factor):");
                ImGui::Spacing();

                float tfBtnW = (ImGui::GetContentRegionAvail().x - 16.0f) / 5.0f;
                if (ImGui::Button("1x (Норма)", ImVec2(tfBtnW, 26)))
                {
                    XRay::Script::SetTimeFactor(1.0f);
                    s_lastActionMsg = "Скорость времени: 1x";
                }
                ImGui::SameLine();
                if (ImGui::Button("5x", ImVec2(tfBtnW, 26)))
                {
                    XRay::Script::SetTimeFactor(5.0f);
                    s_lastActionMsg = "Скорость времени: 5x";
                }
                ImGui::SameLine();
                if (ImGui::Button("10x", ImVec2(tfBtnW, 26)))
                {
                    XRay::Script::SetTimeFactor(10.0f);
                    s_lastActionMsg = "Скорость времени: 10x";
                }
                ImGui::SameLine();
                if (ImGui::Button("60x (1с = 1м)", ImVec2(tfBtnW, 26)))
                {
                    XRay::Script::SetTimeFactor(60.0f);
                    s_lastActionMsg = "Скорость времени: 60x";
                }
                ImGui::SameLine();
                if (ImGui::Button("1000x (Быстро)", ImVec2(tfBtnW, 26)))
                {
                    XRay::Script::SetTimeFactor(1000.0f);
                    s_lastActionMsg = "Скорость времени: 1000x";
                }
            }
            // Categories 0-10: Items & Entities Spawner
            else
            {
                bool isEntity = (s_selectedCategory == 9 || s_selectedCategory == 10);

                // Top Options Bar
                ImGui::SetNextItemWidth(160.0f);
                ImGui::InputText("##SearchFilter", s_searchFilter, sizeof(s_searchFilter));
                ImGui::SameLine();
                ImGui::Text("Поиск");

                ImGui::SameLine(240.0f);
                ImGui::SetNextItemWidth(80.0f);
                ImGui::InputInt("Кол-во##SpawnCnt", &s_spawnCount);
                if (s_spawnCount < 1) s_spawnCount = 1;
                if (s_spawnCount > 100) s_spawnCount = 100;

                if (!isEntity)
                {
                    ImGui::SameLine(360.0f);
                    ImGui::Checkbox("В рюкзак", &s_toInventory);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Если включено - спавнит сразу в инвентарь игрока. Иначе - перед ногами.");
                }
                else
                {
                    ImGui::SameLine(360.0f);
                    ImGui::SetNextItemWidth(110.0f);
                    ImGui::SliderFloat("Дистанция (м)", &s_spawnDistance, 2.0f, 25.0f, "%.0f м");
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // Pick Array
                const ZoneLordItem* items = nullptr;
                int itemCount = 0;

                switch (s_selectedCategory)
                {
                case 0: items = s_Weapons; itemCount = IM_ARRAYSIZE(s_Weapons); break;
                case 1: items = s_UniqueWeapons; itemCount = IM_ARRAYSIZE(s_UniqueWeapons); break;
                case 2: items = s_Attachments; itemCount = IM_ARRAYSIZE(s_Attachments); break;
                case 3: items = s_Ammo; itemCount = IM_ARRAYSIZE(s_Ammo); break;
                case 4: items = s_Outfits; itemCount = IM_ARRAYSIZE(s_Outfits); break;
                case 5: items = s_Artifacts; itemCount = IM_ARRAYSIZE(s_Artifacts); break;
                case 6: items = s_Supplies; itemCount = IM_ARRAYSIZE(s_Supplies); break;
                case 7: items = s_MonsterParts; itemCount = IM_ARRAYSIZE(s_MonsterParts); break;
                case 8: items = s_QuestItems; itemCount = IM_ARRAYSIZE(s_QuestItems); break;
                case 9: items = s_Mutants; itemCount = IM_ARRAYSIZE(s_Mutants); break;
                case 10: items = s_Stalkers; itemCount = IM_ARRAYSIZE(s_Stalkers); break;
                }

                std::string filterLower = s_searchFilter;
                std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), ::tolower);

                ImGui::BeginChild("ItemsListScroll", ImVec2(0, -32), true);
                if (items && itemCount > 0)
                {
                    for (int i = 0; i < itemCount; ++i)
                    {
                        const auto& it = items[i];

                        // Filtering
                        if (!filterLower.empty())
                        {
                            std::string lblLower = it.label;
                            std::string secLower = it.section;
                            std::string grpLower = it.group;
                            std::transform(lblLower.begin(), lblLower.end(), lblLower.begin(), ::tolower);
                            std::transform(secLower.begin(), secLower.end(), secLower.begin(), ::tolower);
                            std::transform(grpLower.begin(), grpLower.end(), grpLower.begin(), ::tolower);

                            if (lblLower.find(filterLower) == std::string::npos &&
                                secLower.find(filterLower) == std::string::npos &&
                                grpLower.find(filterLower) == std::string::npos)
                            {
                                continue;
                            }
                        }

                        ImGui::PushID(i);

                        ImGui::TextColored(ImVec4(0.95f, 0.70f, 0.20f, 1.0f), "[%s]", it.group);
                        ImGui::SameLine();
                        ImGui::Text("%s", it.label);
                        ImGui::SameLine();
                        ImGui::TextDisabled("(%s)", it.section);

                        ImGui::SameLine(ImGui::GetWindowWidth() - 105.0f);
                        if (ImGui::Button("Спавнить", ImVec2(85, 22)))
                        {
                            if (isEntity)
                            {
                                XRay::Script::SpawnEntity(it.section, s_spawnCount, s_spawnDistance);
                                s_lastActionMsg = "Заспавнен(о): " + std::string(it.label) + " x" + std::to_string(s_spawnCount);
                            }
                            else
                            {
                                XRay::Script::SpawnItem(it.section, s_spawnCount, s_toInventory);
                                s_lastActionMsg = "Заспавнен(о): " + std::string(it.label) + " x" + std::to_string(s_spawnCount) +
                                                  (s_toInventory ? " [в рюкзак]" : " [перед собой]");
                            }
                        }

                        ImGui::Separator();
                        ImGui::PopID();
                    }
                }
                ImGui::EndChild();
            }

            // Bottom Status Feedback
            if (!s_lastActionMsg.empty())
            {
                ImGui::TextColored(ImVec4(0.30f, 0.95f, 0.40f, 1.0f), "> %s", s_lastActionMsg.c_str());
            }
            else
            {
                ImGui::TextDisabled("Готов к спавну.");
            }
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
#if defined(XRAY_COP)
        BeginCard("Card_World_Diagnostics", "X-Ray Engine 1.6 Diagnostics", ImVec2(colW, 0));
#else
        BeginCard("Card_World_Diagnostics", "X-Ray Engine 1.0006 Diagnostics", ImVec2(colW, 0));
#endif
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
            ImGui::Checkbox("Prevent Game Always-On-Top", &g_Config.settings.bPreventTopmost);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Removes WS_EX_TOPMOST from the game window. Allows Alt-Tab and lets browser/Discord stay in front.");

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

#if defined(XRAY_COP)
            if (ImGui::Button("Eject DLL From xrEngine.exe", ImVec2(-1.0f, 32)))
#else
            if (ImGui::Button("Eject DLL From XR_3DA.exe", ImVec2(-1.0f, 32)))
#endif
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
        ImGui::SetNextWindowSize(ImVec2(800.0f * s, 560.0f * s), ImGuiCond_FirstUseEver);

#if defined(XRAY_COP)
        if (ImGui::Begin("S.T.A.L.K.E.R. - Internal COP v1.6.02 [X-Ray 1.6]", &g_Config.bShowMenu,
                         ImGuiWindowFlags_NoCollapse))
        {
            // Animated Medusa Artifact Icon Banner
            ImVec2 headerStart = ImGui::GetCursorScreenPos();
            DrawMedusaArtifactIcon(ImGui::GetWindowDrawList(), ImVec2(headerStart.x + 14.0f * s, headerStart.y + 11.0f * s), 9.0f * s);

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 30.0f * s);
            ImGui::TextColored(ImVec4(0.95f, 0.70f, 0.20f, 1.0f), "PROJECT MEDUSA");
            ImGui::SameLine();
            ImGui::TextDisabled("| S.T.A.L.K.E.R. Call of Pripyat v1.6.02");
            ImGui::Spacing();
#else
        if (ImGui::Begin("S.T.A.L.K.E.R. - Internal SOC v1.0006 [X-Ray 1.0]", &g_Config.bShowMenu,
                         ImGuiWindowFlags_NoCollapse))
        {
            // Animated Medusa Artifact Icon Banner
            ImVec2 headerStart = ImGui::GetCursorScreenPos();
            DrawMedusaArtifactIcon(ImGui::GetWindowDrawList(), ImVec2(headerStart.x + 14.0f * s, headerStart.y + 11.0f * s), 9.0f * s);

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 30.0f * s);
            ImGui::TextColored(ImVec4(0.95f, 0.70f, 0.20f, 1.0f), "PROJECT MEDUSA");
            ImGui::SameLine();
            ImGui::TextDisabled("| S.T.A.L.K.E.R. Internal SOC v1.0006");
            ImGui::Spacing();
#endif

            // Horizontal Navigation Bar
            const char* tabNames[] = {
                "Visuals / ESP",
                "Chams",
                "Aimbot",
                "Rage & Exploits",
                "Trainers",
                "Повелитель зоны",
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
            case 3: RenderRageTab(); break;
            case 4: RenderTrainersTab(); break;
            case 5: RenderZoneLordTab(); break;
            case 6: RenderWorldTab(); break;
            case 7: RenderSettingsTab(); break;
            default: break;
            }
        }
        ImGui::End();

        ImGui::PopStyleVar(); // Pop Alpha
    }
}
