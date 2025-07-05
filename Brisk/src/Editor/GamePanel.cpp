#include "GamePanel.hpp"

#include "Editor.hpp"

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