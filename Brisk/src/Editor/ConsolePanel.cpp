#include "ConsolePanel.hpp"

namespace Brisk
{
    void ConsolePanel::OnCreate() {
    }

    void ConsolePanel::OnUpdate() {
        // Begin the assets window
        ImGui::Begin("Console");

        // End the assets window
        ImGui::End();
    }

    void ConsolePanel::OnDestroy() {
        //m_SceneTexture->Release();
    }
}