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
        static void RenderAimbotTab();
        static void RenderMiscTab();
        static void RenderWorldTab();
        static void RenderSettingsTab();
    };
}
