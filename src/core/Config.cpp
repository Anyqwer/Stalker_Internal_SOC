#include "Config.hpp"
#include "Logger.hpp"
#include <cstdio>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

CheatConfig g_Config;

const char* KeyCodeToString(int vk)
{
    if (vk == 0) return "None";

    switch (vk)
    {
    case VK_LBUTTON: return "Mouse 1";
    case VK_RBUTTON: return "Mouse 2";
    case VK_MBUTTON: return "Mouse 3";
    case VK_XBUTTON1: return "Mouse 4";
    case VK_XBUTTON2: return "Mouse 5";
    case VK_SHIFT: case VK_LSHIFT: case VK_RSHIFT: return "Shift";
    case VK_CONTROL: case VK_LCONTROL: case VK_RCONTROL: return "Ctrl";
    case VK_MENU: case VK_LMENU: case VK_RMENU: return "Alt";
    case VK_SPACE: return "Space";
    case VK_TAB: return "Tab";
    case VK_CAPITAL: return "Caps Lock";
    case VK_ESCAPE: return "Escape";
    case VK_RETURN: return "Enter";
    case VK_BACK: return "Backspace";
    case VK_DELETE: return "Delete";
    case VK_INSERT: return "Insert";
    case VK_HOME: return "Home";
    case VK_END: return "End";
    case VK_PRIOR: return "Page Up";
    case VK_NEXT: return "Page Down";
    case VK_LEFT: return "Left";
    case VK_RIGHT: return "Right";
    case VK_UP: return "Up";
    case VK_DOWN: return "Down";
    default:
        if (vk >= VK_F1 && vk <= VK_F12)
        {
            static char fBuf[8];
            snprintf(fBuf, sizeof(fBuf), "F%d", vk - VK_F1 + 1);
            return fBuf;
        }
        if (vk >= 'A' && vk <= 'Z')
        {
            static char cBuf[4];
            snprintf(cBuf, sizeof(cBuf), "%c", vk);
            return cBuf;
        }
        if (vk >= '0' && vk <= '9')
        {
            static char dBuf[4];
            snprintf(dBuf, sizeof(dBuf), "%c", vk);
            return dBuf;
        }
        if (vk >= VK_NUMPAD0 && vk <= VK_NUMPAD9)
        {
            static char numBuf[12];
            snprintf(numBuf, sizeof(numBuf), "Num %d", vk - VK_NUMPAD0);
            return numBuf;
        }
        static char hexBuf[16];
        snprintf(hexBuf, sizeof(hexBuf), "0x%02X", vk);
        return hexBuf;
    }
}

bool RenderKeyBindWidget(const char* label, KeyBind& bind, float width)
{
    ImGui::PushID(label);

    const char* display = bind.is_binding ? "[Press Key...]" : KeyCodeToString(bind.vk_code);
    ImVec2 btnSize(width * g_Config.settings.fUiScale, 24.0f * g_Config.settings.fUiScale);

    if (bind.is_binding)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.45f, 0.10f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    }

    bool clicked = ImGui::Button(display, btnSize);

    if (bind.is_binding)
    {
        ImGui::PopStyleColor(2);
    }

    if (clicked)
    {
        bind.is_binding = !bind.is_binding;
    }

    bool changed = false;
    if (bind.is_binding)
    {
        // Cancel on Escape
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
        {
            bind.is_binding = false;
            changed = true;
        }
        else
        {
            // Check mouse buttons
            for (int vk = VK_LBUTTON; vk <= VK_XBUTTON2; ++vk)
            {
                if (vk == VK_LBUTTON && ImGui::IsItemHovered())
                    continue; // Skip the click that opened it

                if (GetAsyncKeyState(vk) & 0x8000)
                {
                    bind.vk_code = vk;
                    bind.is_binding = false;
                    changed = true;
                    break;
                }
            }

            // Check keyboard keys
            if (bind.is_binding)
            {
                for (int vk = 8; vk <= 255; ++vk)
                {
                    if (vk == VK_ESCAPE) continue;
                    if (GetAsyncKeyState(vk) & 0x8000)
                    {
                        bind.vk_code = vk;
                        bind.is_binding = false;
                        changed = true;
                        break;
                    }
                }
            }
        }
    }

    ImGui::SameLine();
    ImGui::Text("%s", label);

    ImGui::PopID();
    return changed;
}

void CheatConfig::ResetToDefaults()
{
    *this = CheatConfig();
}

static std::string GetConfigDirectory()
{
    std::string path = ".\\stalker_configs";
    std::error_code ec;
    fs::create_directories(path, ec);
    return path;
}

bool CheatConfig::SaveConfig(const char* configName)
{
    if (!configName || configName[0] == '\0')
        configName = "Default";

    std::string dir = GetConfigDirectory();
    std::string filePath = dir + "\\" + configName + ".ini";

    std::ofstream file(filePath);
    if (!file.is_open())
    {
        Core::Logger::Error("Failed to open file for saving config: %s", filePath.c_str());
        return false;
    }

    file << "; S.T.A.L.K.E.R. Shadow of Chernobyl Internal Cheat Profile\n";
    file << "; Config: " << configName << "\n\n";

    file << "[Visuals]\n";
    file << "Enabled=" << visuals.bEnabled << "\n";
    file << "Box2D=" << visuals.bBox2D << "\n";
    file << "BoxType=" << visuals.iBoxType << "\n";
    file << "Snaplines=" << visuals.bSnaplines << "\n";
    file << "SnaplinePos=" << visuals.iSnaplinePos << "\n";
    file << "Distance=" << visuals.bDistance << "\n";
    file << "HealthBar=" << visuals.bHealthBar << "\n";
    file << "Name=" << visuals.bName << "\n";
    file << "Skeleton=" << visuals.bSkeleton << "\n";
    file << "Anomalies=" << visuals.bAnomalies << "\n";
    file << "AnomalyDistance=" << visuals.fAnomalyDistance << "\n";
    file << "ShowEnemies=" << visuals.bShowEnemies << "\n";
    file << "ShowNeutrals=" << visuals.bShowNeutrals << "\n";
    file << "ShowFriends=" << visuals.bShowFriends << "\n";
    file << "ShowMutants=" << visuals.bShowMutants << "\n";
    file << "ShowArtifacts=" << visuals.bShowArtifacts << "\n";
    file << "ShowWeapons=" << visuals.bShowWeapons << "\n";
    file << "MaxDistance=" << visuals.fMaxDistance << "\n";
    file << "Skeleton=" << visuals.bSkeleton << "\n";
    file << "SkeletonVisibleCheck=" << visuals.bSkeletonVisibleCheck << "\n";
    file << "ColSkelVisR=" << visuals.colSkeletonVisible.x << "\n";
    file << "ColSkelVisG=" << visuals.colSkeletonVisible.y << "\n";
    file << "ColSkelVisB=" << visuals.colSkeletonVisible.z << "\n";
    file << "ColSkelOccR=" << visuals.colSkeletonOccluded.x << "\n";
    file << "ColSkelOccG=" << visuals.colSkeletonOccluded.y << "\n";
    file << "ColSkelOccB=" << visuals.colSkeletonOccluded.z << "\n";
    file << "Crosshair=" << visuals.bCrosshair << "\n";
    file << "DrawAimbotFov=" << visuals.bDrawAimbotFov << "\n";

    file << "\n[Chams]\n";
    file << "Enabled=" << chams.bEnabled << "\n";
    file << "ThroughWalls=" << chams.bThroughWalls << "\n";
    file << "ChamsType=" << chams.iChamsType << "\n";
    file << "ChamsStalkers=" << chams.bChamsStalkers << "\n";
    file << "ChamsMutants=" << chams.bChamsMutants << "\n";
    file << "ColVisR=" << chams.colVisible.x << "\n";
    file << "ColVisG=" << chams.colVisible.y << "\n";
    file << "ColVisB=" << chams.colVisible.z << "\n";
    file << "ColHidR=" << chams.colHidden.x << "\n";
    file << "ColHidG=" << chams.colHidden.y << "\n";
    file << "ColHidB=" << chams.colHidden.z << "\n";

    file << "\n[Aimbot]\n";
    file << "Enabled=" << aimbot.bEnabled << "\n";
    file << "Key=" << aimbot.bindKey.vk_code << "\n";
    file << "FOV=" << aimbot.fFov << "\n";
    file << "Smoothness=" << aimbot.fSmoothness << "\n";
    file << "TargetBone=" << aimbot.iTargetBone << "\n";
    file << "TargetEnemies=" << aimbot.bTargetEnemies << "\n";
    file << "TargetMutants=" << aimbot.bTargetMutants << "\n";
    file << "VisCheck=" << aimbot.bVisCheck << "\n";
    file << "Triggerbot=" << aimbot.bTriggerbot << "\n";
    file << "TriggerbotAlways=" << aimbot.bTriggerbotAlways << "\n";
    file << "TriggerbotKey=" << aimbot.keyTriggerbot.vk_code << "\n";
    file << "TriggerbotDelay=" << aimbot.fTriggerbotDelay << "\n";

    file << "\n[Environment]\n";
    file << "Nightmode=" << environment.bNightmode << "\n";
    file << "TimeOverride=" << environment.bTimeOverride << "\n";
    file << "TimeOfDay=" << environment.fTimeOfDay << "\n";
    file << "SelectedWeather=" << environment.iSelectedWeather << "\n";

    file << "\n[FOV]\n";
    file << "Enabled=" << fov.bEnabled << "\n";
    file << "FOV=" << fov.fFov << "\n";

    file << "\n[Rage]\n";
    file << "RageAimbot=" << rage.bRageAimbot << "\n";
    file << "AutoShoot=" << rage.bAutoShoot << "\n";
    file << "RapidFire=" << rage.bRapidFire << "\n";
    file << "RapidFireDelay=" << rage.fRapidFireDelay << "\n";
    file << "360Fov=" << rage.b360Fov << "\n";
    file << "TargetMutants=" << rage.bTargetMutants << "\n";
    file << "TargetEnemies=" << rage.bTargetEnemies << "\n";
    file << "RageBone=" << rage.iRageBone << "\n";
    file << "Blink=" << rage.bBlink << "\n";
    file << "BlinkKey=" << rage.keyBlink.vk_code << "\n";
    file << "BlinkDist=" << rage.fBlinkDist << "\n";
    file << "Backstab=" << rage.bBackstab << "\n";
    file << "BackstabKey=" << rage.keyBackstab.vk_code << "\n";
    file << "OffscreenArrows=" << rage.bOffscreenArrows << "\n";
    file << "ArrowRadius=" << rage.fArrowRadius << "\n";

    file << "\n[Trainers]\n";
    file << "Godmode=" << misc.bGodmode << "\n";
    file << "InfiniteAmmo=" << misc.bInfiniteAmmo << "\n";
    file << "NoRecoil=" << misc.bNoRecoil << "\n";
    file << "InfiniteWeight=" << misc.bInfiniteWeight << "\n";
    file << "WeightLimit=" << misc.fWeightLimit << "\n";
    file << "Speedhack=" << misc.bSpeedhack << "\n";
    file << "SpeedMultiplier=" << misc.fSpeedMultiplier << "\n";
    file << "SuperJump=" << misc.bSuperJump << "\n";
    file << "JumpSpeed=" << misc.fJumpSpeed << "\n";
    file << "ThirdPerson=" << misc.bThirdPerson << "\n";
    file << "ThirdPersonMode=" << misc.iThirdPersonMode << "\n";
    file << "ThirdPersonKey=" << misc.keyThirdPerson.vk_code << "\n";

    file << "\n[Settings]\n";
    file << "MenuKey=" << settings.keyMenu.vk_code << "\n";
    file << "UnloadKey=" << settings.keyUnload.vk_code << "\n";
    file << "UiScale=" << settings.fUiScale << "\n";

    file.close();
    Core::Logger::Success("Config saved successfully to %s", filePath.c_str());
    return true;
}

bool CheatConfig::LoadConfig(const char* configName)
{
    if (!configName || configName[0] == '\0')
        configName = "Default";

    std::string dir = GetConfigDirectory();
    std::string filePath = dir + "\\" + configName + ".ini";

    std::ifstream file(filePath);
    if (!file.is_open())
    {
        Core::Logger::Warning("Config file not found: %s", filePath.c_str());
        return false;
    }

    std::string line, section;
    while (std::getline(file, line))
    {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (line.empty() || line[0] == ';') continue;

        if (line.front() == '[' && line.back() == ']')
        {
            section = line.substr(1, line.length() - 2);
            continue;
        }

        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) continue;

        std::string key = line.substr(0, eqPos);
        std::string val = line.substr(eqPos + 1);

        auto toBool = [](const std::string& s) { return s == "1" || s == "true"; };
        auto toFloat = [](const std::string& s) { return std::stof(s); };
        auto toInt = [](const std::string& s) { return std::stoi(s); };

        try
        {
            if (section == "Visuals")
            {
                if (key == "Enabled") visuals.bEnabled = toBool(val);
                else if (key == "Box2D") visuals.bBox2D = toBool(val);
                else if (key == "BoxType") visuals.iBoxType = toInt(val);
                else if (key == "Snaplines") visuals.bSnaplines = toBool(val);
                else if (key == "SnaplinePos") visuals.iSnaplinePos = toInt(val);
                else if (key == "Distance") visuals.bDistance = toBool(val);
                else if (key == "HealthBar") visuals.bHealthBar = toBool(val);
                else if (key == "Name") visuals.bName = toBool(val);
                else if (key == "Skeleton") visuals.bSkeleton = toBool(val);
                else if (key == "SkeletonVisibleCheck") visuals.bSkeletonVisibleCheck = toBool(val);
                else if (key == "ColSkelVisR") visuals.colSkeletonVisible.x = toFloat(val);
                else if (key == "ColSkelVisG") visuals.colSkeletonVisible.y = toFloat(val);
                else if (key == "ColSkelVisB") visuals.colSkeletonVisible.z = toFloat(val);
                else if (key == "ColSkelOccR") visuals.colSkeletonOccluded.x = toFloat(val);
                else if (key == "ColSkelOccG") visuals.colSkeletonOccluded.y = toFloat(val);
                else if (key == "ColSkelOccB") visuals.colSkeletonOccluded.z = toFloat(val);
                else if (key == "Anomalies") visuals.bAnomalies = toBool(val);
                else if (key == "AnomalyDistance") visuals.fAnomalyDistance = toFloat(val);
                else if (key == "ShowEnemies") visuals.bShowEnemies = toBool(val);
                else if (key == "ShowNeutrals") visuals.bShowNeutrals = toBool(val);
                else if (key == "ShowFriends") visuals.bShowFriends = toBool(val);
                else if (key == "ShowMutants") visuals.bShowMutants = toBool(val);
                else if (key == "ShowArtifacts") visuals.bShowArtifacts = toBool(val);
                else if (key == "ShowWeapons") visuals.bShowWeapons = toBool(val);
                else if (key == "MaxDistance") visuals.fMaxDistance = toFloat(val);
                else if (key == "Crosshair") visuals.bCrosshair = toBool(val);
                else if (key == "DrawAimbotFov") visuals.bDrawAimbotFov = toBool(val);
            }
            else if (section == "Chams")
            {
                if (key == "Enabled") chams.bEnabled = toBool(val);
                else if (key == "ThroughWalls") chams.bThroughWalls = toBool(val);
                else if (key == "ChamsType") chams.iChamsType = toInt(val);
                else if (key == "ChamsStalkers") chams.bChamsStalkers = toBool(val);
                else if (key == "ChamsMutants") chams.bChamsMutants = toBool(val);
                else if (key == "ColVisR") chams.colVisible.x = toFloat(val);
                else if (key == "ColVisG") chams.colVisible.y = toFloat(val);
                else if (key == "ColVisB") chams.colVisible.z = toFloat(val);
                else if (key == "ColHidR") chams.colHidden.x = toFloat(val);
                else if (key == "ColHidG") chams.colHidden.y = toFloat(val);
                else if (key == "ColHidB") chams.colHidden.z = toFloat(val);
            }
            else if (section == "Aimbot")
            {
                if (key == "Enabled") aimbot.bEnabled = toBool(val);
                else if (key == "Key") aimbot.bindKey.vk_code = toInt(val);
                else if (key == "FOV") aimbot.fFov = toFloat(val);
                else if (key == "Smoothness") aimbot.fSmoothness = toFloat(val);
                else if (key == "TargetBone") aimbot.iTargetBone = toInt(val);
                else if (key == "TargetEnemies") aimbot.bTargetEnemies = toBool(val);
                else if (key == "TargetMutants") aimbot.bTargetMutants = toBool(val);
                else if (key == "VisCheck") aimbot.bVisCheck = toBool(val);
                else if (key == "Triggerbot") aimbot.bTriggerbot = toBool(val);
                else if (key == "TriggerbotAlways") aimbot.bTriggerbotAlways = toBool(val);
                else if (key == "TriggerbotKey") aimbot.keyTriggerbot.vk_code = toInt(val);
                else if (key == "TriggerbotDelay") aimbot.fTriggerbotDelay = toFloat(val);
            }
            else if (section == "Environment")
            {
                if (key == "Nightmode") environment.bNightmode = toBool(val);
                else if (key == "TimeOverride") environment.bTimeOverride = toBool(val);
                else if (key == "TimeOfDay") environment.fTimeOfDay = toFloat(val);
                else if (key == "SelectedWeather") environment.iSelectedWeather = toInt(val);
            }
            else if (section == "FOV")
            {
                if (key == "Enabled") fov.bEnabled = toBool(val);
                else if (key == "FOV") fov.fFov = toFloat(val);
            }
            else if (section == "Rage")
            {
                if (key == "RageAimbot") rage.bRageAimbot = toBool(val);
                else if (key == "AutoShoot") rage.bAutoShoot = toBool(val);
                else if (key == "RapidFire") rage.bRapidFire = toBool(val);
                else if (key == "RapidFireDelay") rage.fRapidFireDelay = toFloat(val);
                else if (key == "360Fov") rage.b360Fov = toBool(val);
                else if (key == "TargetMutants") rage.bTargetMutants = toBool(val);
                else if (key == "TargetEnemies") rage.bTargetEnemies = toBool(val);
                else if (key == "RageBone") rage.iRageBone = toInt(val);
                else if (key == "Blink") rage.bBlink = toBool(val);
                else if (key == "BlinkKey") rage.keyBlink.vk_code = toInt(val);
                else if (key == "BlinkDist") rage.fBlinkDist = toFloat(val);
                else if (key == "Backstab") rage.bBackstab = toBool(val);
                else if (key == "BackstabKey") rage.keyBackstab.vk_code = toInt(val);
                else if (key == "OffscreenArrows") rage.bOffscreenArrows = toBool(val);
                else if (key == "ArrowRadius") rage.fArrowRadius = toFloat(val);
            }
            else if (section == "Trainers")
            {
                if (key == "Godmode") misc.bGodmode = toBool(val);
                else if (key == "InfiniteAmmo") misc.bInfiniteAmmo = toBool(val);
                else if (key == "NoRecoil") misc.bNoRecoil = toBool(val);
                else if (key == "InfiniteWeight") misc.bInfiniteWeight = toBool(val);
                else if (key == "WeightLimit") misc.fWeightLimit = toFloat(val);
                else if (key == "Speedhack") misc.bSpeedhack = toBool(val);
                else if (key == "SpeedMultiplier") misc.fSpeedMultiplier = toFloat(val);
                else if (key == "SuperJump") misc.bSuperJump = toBool(val);
                else if (key == "JumpSpeed") misc.fJumpSpeed = toFloat(val);
                else if (key == "ThirdPerson") misc.bThirdPerson = toBool(val);
                else if (key == "ThirdPersonMode") misc.iThirdPersonMode = toInt(val);
                else if (key == "ThirdPersonKey") misc.keyThirdPerson.vk_code = toInt(val);
            }
            else if (section == "Settings")
            {
                if (key == "MenuKey") settings.keyMenu.vk_code = toInt(val);
                else if (key == "UnloadKey") settings.keyUnload.vk_code = toInt(val);
                else if (key == "UiScale") settings.fUiScale = toFloat(val);
            }
        }
        catch (...)
        {
        }
    }

    file.close();
    strncpy_s(settings.szSelectedConfig, configName, sizeof(settings.szSelectedConfig) - 1);
    Core::Logger::Success("Config loaded successfully from %s", filePath.c_str());
    return true;
}

std::vector<std::string> CheatConfig::GetAvailableConfigs()
{
    std::vector<std::string> configs;
    std::string dir = GetConfigDirectory();

    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(dir, ec))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".ini")
        {
            configs.push_back(entry.path().stem().string());
        }
    }

    if (configs.empty())
    {
        configs.push_back("Default");
    }

    return configs;
}
