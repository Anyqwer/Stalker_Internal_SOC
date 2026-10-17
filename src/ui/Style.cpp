#include "Style.hpp"
#include <imgui.h>

namespace UI
{
    void Style::ApplyStalkerTheme()
    {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        // Rounding & sizes
        style.WindowRounding = 4.0f;
        style.ChildRounding = 3.0f;
        style.FrameRounding = 3.0f;
        style.PopupRounding = 3.0f;
        style.ScrollbarRounding = 2.0f;
        style.GrabRounding = 2.0f;
        style.TabRounding = 3.0f;

        style.WindowBorderSize = 1.0f;
        style.FrameBorderSize = 1.0f;
        style.PopupBorderSize = 1.0f;

        style.WindowPadding = ImVec2(10.0f, 10.0f);
        style.FramePadding = ImVec2(6.0f, 4.0f);
        style.ItemSpacing = ImVec2(8.0f, 6.0f);

        // Color palette: Zone Charcoal & Radioactive Amber
        colors[ImGuiCol_Text]                  = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
        colors[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_WindowBg]              = ImVec4(0.12f, 0.13f, 0.14f, 0.96f);
        colors[ImGuiCol_ChildBg]               = ImVec4(0.15f, 0.16f, 0.17f, 0.60f);
        colors[ImGuiCol_PopupBg]               = ImVec4(0.12f, 0.13f, 0.14f, 0.96f);
        colors[ImGuiCol_Border]                = ImVec4(0.28f, 0.30f, 0.32f, 0.80f);
        colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg]               = ImVec4(0.18f, 0.19f, 0.20f, 1.00f);
        colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.24f, 0.25f, 0.27f, 1.00f);
        colors[ImGuiCol_FrameBgActive]         = ImVec4(0.28f, 0.30f, 0.33f, 1.00f);
        colors[ImGuiCol_TitleBg]               = ImVec4(0.10f, 0.11f, 0.12f, 1.00f);
        colors[ImGuiCol_TitleBgActive]         = ImVec4(0.14f, 0.16f, 0.17f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.10f, 0.11f, 0.12f, 0.75f);
        colors[ImGuiCol_MenuBarBg]             = ImVec4(0.14f, 0.15f, 0.16f, 1.00f);
        colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.10f, 0.11f, 0.12f, 0.60f);
        colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.26f, 0.28f, 0.30f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.34f, 0.36f, 0.38f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.42f, 0.44f, 0.46f, 1.00f);
        colors[ImGuiCol_CheckMark]             = ImVec4(0.95f, 0.68f, 0.15f, 1.00f); // Hazard Amber
        colors[ImGuiCol_SliderGrab]            = ImVec4(0.85f, 0.58f, 0.12f, 1.00f);
        colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.98f, 0.72f, 0.18f, 1.00f);
        colors[ImGuiCol_Button]                = ImVec4(0.20f, 0.22f, 0.24f, 1.00f);
        colors[ImGuiCol_ButtonHovered]         = ImVec4(0.85f, 0.58f, 0.12f, 0.80f);
        colors[ImGuiCol_ButtonActive]          = ImVec4(0.95f, 0.68f, 0.15f, 1.00f);
        colors[ImGuiCol_Header]                = ImVec4(0.22f, 0.24f, 0.26f, 1.00f);
        colors[ImGuiCol_HeaderHovered]         = ImVec4(0.85f, 0.58f, 0.12f, 0.70f);
        colors[ImGuiCol_HeaderActive]          = ImVec4(0.95f, 0.68f, 0.15f, 0.90f);
        colors[ImGuiCol_Separator]             = ImVec4(0.26f, 0.28f, 0.30f, 0.60f);
        colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.85f, 0.58f, 0.12f, 0.78f);
        colors[ImGuiCol_SeparatorActive]       = ImVec4(0.95f, 0.68f, 0.15f, 1.00f);
        colors[ImGuiCol_ResizeGrip]            = ImVec4(0.26f, 0.28f, 0.30f, 0.50f);
        colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.85f, 0.58f, 0.12f, 0.70f);
        colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.95f, 0.68f, 0.15f, 0.90f);
        colors[ImGuiCol_Tab]                   = ImVec4(0.16f, 0.17f, 0.18f, 1.00f);
        colors[ImGuiCol_TabHovered]            = ImVec4(0.85f, 0.58f, 0.12f, 0.80f);
        colors[ImGuiCol_TabSelected]           = ImVec4(0.26f, 0.28f, 0.30f, 1.00f);
        colors[ImGuiCol_TabDimmed]             = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
        colors[ImGuiCol_TabDimmedSelected]     = ImVec4(0.20f, 0.22f, 0.24f, 1.00f);
    }
}
