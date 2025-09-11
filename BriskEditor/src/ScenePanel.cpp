#include "ScenePanel.hpp"

#include "EditorLayer.hpp"

namespace Brisk 
{
    void ScenePanel::OnCreate() {
    }

    void ScenePanel::OnUpdate() {
        ImGui::Begin("Scene");
        ImVec2 viewportSize = ImGui::GetContentRegionAvail();

        float aspectRatio = static_cast<float>(Application::GetRenderer()->m_LightingOutput->GetWidth()) / static_cast<float>(Application::GetRenderer()->m_LightingOutput->GetHeight());

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
        ImGui::Image((ImTextureID)Application::GetRenderer()->m_ImGuiIdScene, imageSize);

        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar();

        ImGui::End();
    }

    void ScenePanel::OnDestroy() {
        //m_SceneTexture->Release();
    }
}