#pragma once

#include <windows.h>
#include <imgui.h>
#include <string>
#include <vector>

struct KeyBind
{
    int vk_code = 0;
    bool is_binding = false;
};

struct CheatConfig
{
    // Menu state
    bool bShowMenu = false;
    int iActiveTab = 0; // 0 = Visuals, 1 = Chams, 2 = Aimbot, 3 = Trainers, 4 = World/SDK, 5 = Settings/Configs

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

        // Bone Skeletons
        bool bSkeleton = true;
        bool bSkeletonVisibleCheck = false;
        ImVec4 colSkeleton = ImVec4(1.0f, 1.0f, 1.0f, 0.85f);
        ImVec4 colSkeletonVisible = ImVec4(0.25f, 1.0f, 0.35f, 0.95f);
        ImVec4 colSkeletonOccluded = ImVec4(1.0f, 0.25f, 0.25f, 0.85f);

        // Anomalies ESP
        bool bAnomalies = true;
        float fAnomalyDistance = 150.0f;
        ImVec4 colAnomaly = ImVec4(0.95f, 0.40f, 0.10f, 0.90f);

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

    // Chams (DirectX 9 Material / Shader Hooking)
    struct
    {
        bool bEnabled = false;
        bool bThroughWalls = true;
        int iChamsType = 0; // 0 = Flat/Solid, 1 = Wireframe, 2 = Shaded
        ImVec4 colVisible = ImVec4(0.20f, 1.00f, 0.35f, 1.00f);
        ImVec4 colHidden = ImVec4(1.00f, 0.20f, 0.20f, 1.00f);
        bool bChamsStalkers = true;
        bool bChamsMutants = true;
    } chams;

    // Aimbot & Triggerbot
    struct
    {
        bool bEnabled = false;
        KeyBind bindKey = { VK_RBUTTON, false };
        bool bSilent = false;
        float fFov = 60.0f;           // FOV in pixels / angle
        float fSmoothness = 5.0f;     // Smoothing factor (1 = snap, 20 = very slow)
        int iTargetBone = 0;          // 0 = Head, 1 = Neck, 2 = Chest, 3 = Pelvis
        int iTargetPriority = 0;      // 0 = Closest to Crosshair, 1 = Closest Distance
        bool bTargetEnemies = true;
        bool bTargetMutants = true;
        bool bTargetFriends = false;
        bool bVisCheck = true;        // Engine raycast check

        // Triggerbot
        bool bTriggerbot = false;
        bool bTriggerbotAlways = false; // Always on without holding key
        KeyBind keyTriggerbot = { 0, false };
        float fTriggerbotDelay = 25.0f; // ms delay
    } aimbot;

    // Environment & Weather
    struct
    {
        bool bNightmode = false;
        ImVec4 colNightmode = ImVec4(0.12f, 0.16f, 0.28f, 1.0f);
        bool bCustomSkybox = false;
        ImVec4 colSkybox = ImVec4(0.08f, 0.12f, 0.25f, 1.0f);
        bool bCustomAmbient = false;
        ImVec4 colAmbient = ImVec4(0.15f, 0.18f, 0.25f, 1.0f);

        bool bTimeOverride = false;
        float fTimeOfDay = 12.0f;     // Hour (0..24)
        int iSelectedWeather = 0;     // 0 = Normal, 1 = Clear, 2 = Cloudy, 3 = Rain, 4 = Storm, 5 = Radar
    } environment;

    // Singleplayer Trainers & Exploits
    struct
    {
        bool bGodmode = false;
        bool bInfiniteAmmo = false;
        bool bNoRecoil = false;
        bool bInfiniteWeight = false;
        float fWeightLimit = 50000.0f;
        bool bSpeedhack = false;
        float fSpeedMultiplier = 2.0f;
        bool bSuperJump = false;
        float fJumpSpeed = 15.0f;
        bool bBunnyhop = false;
        bool bNightVision = false;
    } misc;

    // Settings & Keybinds
    struct
    {
        KeyBind keyMenu = { VK_INSERT, false };
        KeyBind keyUnload = { VK_END, false };
        KeyBind keyAimbot = { VK_RBUTTON, false };
        KeyBind keyGodmode = { 0, false };
        KeyBind keySpeedhack = { 0, false };
        KeyBind keySuperJump = { 0, false };

        float fUiScale = 1.0f;
        bool bShowConsole = true;
        char szSelectedConfig[64] = "Default";
        char szNewConfigName[64] = "";
    } settings;

    void ResetToDefaults();
    bool SaveConfig(const char* configName);
    bool LoadConfig(const char* configName);
    std::vector<std::string> GetAvailableConfigs();
};

extern CheatConfig g_Config;

const char* KeyCodeToString(int vk);
bool RenderKeyBindWidget(const char* label, KeyBind& bind, float width = 110.0f);
