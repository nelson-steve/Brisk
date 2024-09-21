#include "GamePanel.hpp"

#include "Editor.hpp"

namespace Brisk 
{
    void GamePanel::OnCreate() {
        //m_SceneTexture = BriskTexture::Create();
        //m_SceneDescriptorSet = Editor::AddTexToUI(m_SceneTexture);
    }

    void GamePanel::OnUpdate(){
        ImGui::Begin("Game");
        //ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        //m_ViewportSize = glm::vec2(viewportSize.x, viewportSize.y);
        //ImGui::Image((ImTextureID)m_SceneDescriptorSet, ImVec2{ viewportSize.x, viewportSize.y });

        ImGui::End();
    }

    void GamePanel::OnDestroy() {
        //m_SceneTexture->Release();
    }
}