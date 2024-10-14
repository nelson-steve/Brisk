#include "HeirarchyPanel.hpp"
#include "Engine/Engine.hpp"
#include <functional>

namespace Brisk 
{
    void HeirarchyPanel::OnCreate() {
    }

    void HeirarchyPanel::OnUpdate(){
        ImGui::Begin("Hierarchy");

        // Get the draw list to draw lines
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 windowPos = ImGui::GetWindowPos();  // Get window position to offset drawing

        ImVec2 mousePos = ImGui::GetMousePos();
        //ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();

        bool isElementSelected = false;
        // Function to recursively display elements and their children
        std::function<void(int, float)> DisplayElementWithChildren = [&](int elementIndex, float parentXPos) {
            //auto& element = Engine::m_ActiveScene->Elements[elementIndex];

            ImGuiTreeNodeFlags flags = 0;
            //if (elementIndex == Engine::m_ActiveScene->SelectedElement)
                flags |= ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_OpenOnArrow;
            //else
                flags |= ImGuiTreeNodeFlags_OpenOnArrow ;
            //if (Engine::m_ActiveScene->Elements[elementIndex].children.size() <= 0) {
            //    flags |= ImGuiTreeNodeFlags_Leaf; // Mark as a leaf node (no arrow)
            //}
            // Start the tree node for the element
            //if (ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)elementIndex,  flags, element.name.c_str())) 
            {
                // Get the position of the current element to draw the line from parent to child
                ImVec2 nodePos = ImGui::GetCursorScreenPos();  // Screen position of the node (for line drawing)
                ImVec2 nodeTextPos = ImGui::GetItemRectMin();  // Top-left position of the text (for line alignment)
                ImVec2 nodeBottomPos = ImGui::GetItemRectMax();  // Bottom-right of the text

                // If there is a parent, draw a line from parent to this child
                if (parentXPos != -1.0f) {
                    ImVec2 parentPos = ImVec2(parentXPos-1000.0f, (nodeTextPos.y + nodeBottomPos.y) * 0.5f);  // Midpoint of the parent node vertically
                    ImVec2 childPos = ImVec2(nodeTextPos.x - 0.0f, (nodeTextPos.y + nodeBottomPos.y) * 0.5f);  // Midpoint of child node
                    drawList->AddLine(parentPos, childPos, IM_COL32(150, 150, 150, 255), 4.0f);  // Draw line from parent to child
                }   

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
                    //Engine::m_ActiveScene->SelectedElement = elementIndex;
                }
                else if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                    isElementSelected = true;
                    //Engine::m_ActiveScene->SelectedElement = elementIndex;
                }
                else {
                    isElementSelected = false;

                }

                // Recursively display children
                //for (int childIndex : element.children) {
                //    DisplayElementWithChildren(childIndex, nodeTextPos.x);
                //}

                ImGui::TreePop();
            }
        };

        // Traverse the hierarchy and display each element
        //for (int i = 0; i < Engine::m_ActiveScene->Elements.size(); i++) {
        //    // Only display root elements (elements without parents)
        //    if (Engine::m_ActiveScene->Elements[i].IsRoot) {
        //        DisplayElementWithChildren(i, -1.0f);
        //    }
        //}

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
            }
            if (ImGui::MenuItem("Create Element")) {
            }
            ImGui::EndPopup();
        }

        // Render the context menu
        if (ImGui::BeginPopup("ElementContextMenu")) {
            if (ImGui::MenuItem("Create Element")) {
            }
            ImGui::EndPopup();
        }

        ImGui::End();
    }

    void HeirarchyPanel::OnDestroy(){
        
    }
}