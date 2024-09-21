#include "HeirarchyPanel.hpp"
#include "Engine/Engine.hpp"

namespace Brisk {
    void HeirarchyPanel::OnCreate() {
    }

    void HeirarchyPanel::OnUpdate(){
        ImGui::Begin("Hierarchy");

        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();


        // Example items in the hierarchy
        for (int i = 0; i < Engine::m_Scene->Elements.size(); i++) {
            if (ImGui::Selectable(Engine::m_Scene->Elements[i].name.c_str())) {
            }
        }

        // Check if right-click is inside the window and not on any item
        bool isHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);
        bool isRightClick = ImGui::IsMouseReleased(ImGuiMouseButton_Right);

        if (isHovered && isRightClick) {
            // Open context menu only if mouse is inside the window
            ImGui::OpenPopup("ContextMenu");
        }

        // Render the context menu
        if (ImGui::BeginPopup("ContextMenu")) {
            if (ImGui::MenuItem("Create Empty")) {
                Engine::AddEmptyElement();
            }
            if (ImGui::MenuItem("Create Element")) {
                Engine::AddEmptyElement();
            }
            ImGui::EndPopup();
        }

        ImGui::End();
    }

    void HeirarchyPanel::OnDestroy(){
        
    }
}