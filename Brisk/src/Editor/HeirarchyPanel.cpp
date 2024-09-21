#include "HeirarchyPanel.hpp"
#include "Engine/Engine.hpp"
#include <functional>

namespace Brisk {
    void HeirarchyPanel::OnCreate() {
    }

    void HeirarchyPanel::OnUpdate(){
        ImGui::Begin("Hierarchy");

        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();

        bool isElementSelected = false;
        // Function to recursively display elements and their children
        std::function<void(int)> DisplayElementWithChildren = [&](int elementIndex) {
            auto& element = Engine::m_Scene->Elements[elementIndex];

            ImGuiTreeNodeFlags flags = 0;
            if (elementIndex == Engine::m_Scene->SelectedElement)
                flags |= ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_OpenOnArrow;
            else
                flags |= ImGuiTreeNodeFlags_OpenOnArrow ;
            if (Engine::m_Scene->Elements[elementIndex].children.size() <= 0) {
                flags |= ImGuiTreeNodeFlags_Leaf; // Mark as a leaf node (no arrow)
            }
            // Start the tree node for the element
            if (ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)elementIndex,  flags, element.name.c_str())) {
                if (ImGui::BeginPopupContextItem())
                {
                    if (ImGui::MenuItem("Delete Entity"))
                        ;
                        //entityDeleted = true;

                    ImGui::EndPopup();
                }

                // If right-clicked, set as the target for context menu
                if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                    isElementSelected = true;
                    Engine::m_Scene->SelectedElement = elementIndex;
                }
                else if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                    isElementSelected = true;
                    Engine::m_Scene->SelectedElement = elementIndex;
                }
                else {
                    isElementSelected = false;

                }

                // Recursively display children
                for (int childIndex : element.children) {
                    DisplayElementWithChildren(childIndex);
                }

                ImGui::TreePop();
            }
        };

        // Traverse the hierarchy and display each element
        for (int i = 0; i < Engine::m_Scene->Elements.size(); i++) {
            // Only display root elements (elements without parents)
            if (Engine::m_Scene->Elements[i].IsRoot) {
                DisplayElementWithChildren(i);
            }
        }

        // Check if right-click is inside the window and not on any item
        bool isHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);
        bool isRightClick = ImGui::IsMouseReleased(ImGuiMouseButton_Right);

        if (isHovered && isRightClick) {
            if(isElementSelected)
                // Open context menu only if mouse is inside the window
                ImGui::OpenPopup("ContextMenu");
            else
                ImGui::OpenPopup("ElementContextMenu");

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

        // Render the context menu
        if (ImGui::BeginPopup("ElementContextMenu")) {
            if (ImGui::MenuItem("Create Element")) {
                Engine::m_Scene->AddChildElement(Engine::m_Scene->SelectedElement);
            }
            ImGui::EndPopup();
        }

        ImGui::End();
    }

    void HeirarchyPanel::OnDestroy(){
        
    }
}