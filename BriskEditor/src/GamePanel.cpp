#include "GamePanel.hpp"

#include "EditorLayer.hpp"

namespace Brisk 
{
    void GamePanel::OnCreate() {
    }

    void GamePanel::OnUpdate(){
        ImGui::Begin("Game");
        ImGui::End();
    }

    void GamePanel::OnDestroy() {
    }
}