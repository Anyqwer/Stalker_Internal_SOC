#pragma once

#include <windows.h>
#include <imgui.h>


struct CheatConfig
{
    // Menu state
    bool bShowMenu = false;
    int iActiveTab = 0; // 0 = Visuals, 1 = Aimbot, 2 = Misc, 3 = Radar/Entities, 4 = Settings

    // ESP / Visuals
    struct
    {
        bool bEnabled = true;
        bool bBox2D = true;
        int iBoxType = 0; // 0 = Corner Box, 1 = Full 2D Box
        bool bSnaplines = false;
        int iSnaplinePos = 0; // 0 = Bottom of screen, 1 = Crosshair
        bool bDistance = true;
        bool bHealthBar = true;
        bool bName = true;
        bool bSkeleton = false;

        // Entity Filters
        bool bShowEnemies = true;
        bool bShowNeutrals = true;
        bool bShowFriends = true;
        bool bShowMutants = true;
        bool bShowArtifacts = true;
        bool bShowWeapons = false;

        float fMaxDistance = 300.0f; // in meters

        // Colors
        ImVec4 colEnemy = ImVec4(1.0f, 0.25f, 0.25f, 1.0f);     // Red
        ImVec4 colNeutral = ImVec4(1.0f, 0.9f, 0.2f, 1.0f);     // Yellow
        ImVec4 colFriend = ImVec4(0.25f, 1.0f, 0.35f, 1.0f);    // Green
        ImVec4 colMutant = ImVec4(1.0f, 0.5f, 0.1f, 1.0f);      // Orange
        ImVec4 colArtifact = ImVec4(0.2f, 0.8f, 1.0f, 1.0f);    // Cyan
        ImVec4 colWeapon = ImVec4(0.8f, 0.8f, 0.8f, 0.9f);      // Light Grey
        ImVec4 colCrosshair = ImVec4(1.0f, 1.0f, 1.0f, 0.8f);

        // Crosshair & FOV visuals
        bool bCrosshair = false;
        bool bDrawAimbotFov = true;
    } visuals;

    // Aimbot
    struct
    {
        bool bEnabled = false;
        bool bSilent = false;
        float fFov = 60.0f;           // FOV in pixels / angle
        float fSmoothness = 5.0f;     // Smoothing factor (1 = snap, 20 = very slow)
        int iTargetBone = 0;          // 0 = Head, 1 = Chest
        int iTargetPriority = 0;      // 0 = Closest to Crosshair, 1 = Closest Distance
        bool bTargetEnemies = true;
        bool bTargetMutants = true;
        bool bTargetFriends = false;
        bool bVisCheck = false;       // Raycast visibility check
    } aimbot;

    // Misc / Player Exploits
    struct
    {
        bool bGodmode = false;
        bool bInfiniteAmmo = false;
        bool bNoRecoil = false;
        bool bInfiniteWeight = false;
        float fWeightLimit = 5000.0f;
        bool bSpeedhack = false;
        float fSpeedMultiplier = 2.0f;
        bool bHighJump = false;
        float fJumpHeight = 12.0f;
        bool bNightVision = false;
    } misc;

    // Settings
    struct
    {
        int iMenuKey = VK_INSERT;
        int iUnloadKey = VK_END;
        float fUiScale = 1.0f;
        bool bShowConsole = true;
    } settings;
};

extern CheatConfig g_Config;
