#pragma once

namespace UI
{
    class Menu
    {
    public:
        // Render the ImGui graphical cheat interface
        static void Render();

    private:
        static void RenderVisualsTab();
        static void RenderChamsTab();
        static void RenderAimbotTab();
        static void RenderTrainersTab();
        static void RenderWorldTab();
        static void RenderSettingsTab();
    };
}
