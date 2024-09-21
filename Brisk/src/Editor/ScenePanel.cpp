#include "ScenePanel.hpp"

#include "Editor.hpp"

namespace Brisk {
    void ScenePanel::OnCreate() {
        m_SceneTexture = BriskTexture::Create(Engine::s_Swapchain->GetExtentWidth(), Engine::s_Swapchain->GetExtentHeight(), BriskTexture::BGR, BriskTexture::TEXTURE2D);
        m_SceneDescriptorSet = Editor::AddTexToUI(m_SceneTexture);
        Engine::s_Renderer->AddRenderTarget({ m_SceneTexture });

    }

    void ScenePanel::OnUpdate() {
        ImGui::Begin("Scene");
        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        m_ViewportSize = glm::vec2(viewportSize.x, viewportSize.y);
        ImGui::Image((ImTextureID)m_SceneDescriptorSet, ImVec2{ viewportSize.x, viewportSize.y });

        ImGui::End();
    }

    void ScenePanel::OnDestroy() {
        //m_SceneTexture->Release();
    }
}