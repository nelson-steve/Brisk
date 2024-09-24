#include "ScenePanel.hpp"
#include "Graphics/Vulkan/VulkanRenderer/RendererVulkan.hpp"

#include "Editor.hpp"

namespace Brisk 
{
    void ScenePanel::OnCreate() {
        m_SceneTexture = BriskTexture::Create(Engine::s_Swapchain->GetExtentWidth(), Engine::s_Swapchain->GetExtentHeight(), BriskTexture::BGR, BriskTexture::TEXTURE2D);
        m_SceneDescriptorSet = Editor::AddTexToUI(m_SceneTexture);

        Engine::s_Renderer->AddRenderTarget({ m_SceneTexture });
    }

    void ScenePanel::OnUpdate() {
        ImGui::Begin("Scene");
        ImVec2 viewportSize = ImGui::GetContentRegionAvail();

        float aspectRatio = 1.778f;

        ImVec2 imageSize;
        if (viewportSize.x / viewportSize.y > aspectRatio) {
            imageSize.y = viewportSize.y;
            imageSize.x = viewportSize.y * aspectRatio;
        }
        else {
            imageSize.x = viewportSize.x;
            imageSize.y = viewportSize.x / aspectRatio;
        }

        // Calculate the position to center the image
        ImVec2 padding = ImVec2((viewportSize.x - imageSize.x) * 0.5f, (viewportSize.y - imageSize.y) * 0.5f);

        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x + padding.x, ImGui::GetCursorPos().y + padding.y));

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        ImGui::Image((void*)(intptr_t)m_SceneDescriptorSet, imageSize);

        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar();

        ImGui::End();
    }

    void ScenePanel::OnDestroy() {
        //m_SceneTexture->Release();
    }
}