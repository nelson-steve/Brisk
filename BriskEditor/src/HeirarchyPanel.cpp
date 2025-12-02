#include "HeirarchyPanel.hpp"
#include "Engine/Engine.hpp"
#include <functional>
#include <Core/Log.hpp>

namespace Brisk 
{
    void HeirarchyPanel::OnCreate() {
    }

    void HeirarchyPanel::DrawEntityNode(Entity entity) {
        auto& reg = SceneManager::pActiveScene->Reg();
        auto& name = reg.get<TagComponent>(entity).Tag;

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

        //bool hasChildren = entity.HasComponent<ChildrenComponent>();
        //if (!hasChildren)
            flags |= ImGuiTreeNodeFlags_Leaf;

        if (SceneManager::pActiveScene->GetSelectedEntity() == entity)
            flags |= ImGuiTreeNodeFlags_Selected;

        bool open = ImGui::TreeNodeEx((void*)(uint64_t)entity.m_EntityHandle, flags, "%s", name.c_str());

        if (ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
            SceneManager::pActiveScene->SetSelectedEntity(entity);
        }

        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Delete Entity")) {
            }
            ImGui::EndPopup();
        }

        if (open) {
            //if (hasChildren) {
            //    auto& children = reg.get<ChildrenComponent>(entity).children;
            //    for (auto child : children)
            //        DrawEntityNode(child);
            //}
            ImGui::TreePop();
        }
    }

    void HeirarchyPanel::OnUpdate(){
        ImGui::Begin("Hierarchy");

        //SceneManager::pActiveScene->Reg().view<TagComponent>().each([&](entt::entity entity, TagComponent& name) {
        //    //if (!SceneManager::pActiveScene->Reg().any_of<ParentComponent>(entity)) {
        //    Entity outEntity = { entity, SceneManager::pActiveScene.get() };
        //    DrawEntityNode(outEntity);
        //    //}
        //    });


        // Check if right-click is inside the window and not on any item
        bool isHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);
        bool isRightClick = ImGui::IsMouseReleased(ImGuiMouseButton_Right);

        if (isHovered && isRightClick) {
            ImGui::OpenPopup("ContextMenu");
        }

        if (ImGui::BeginPopup("ContextMenu")) {
            if (ImGui::MenuItem("Create Empty")) {
            }
            if (ImGui::MenuItem("Create Entity")) {
            }
            ImGui::EndPopup();
        }

        if (ImGui::BeginPopup("EntityContextMenu")) {
            if (ImGui::MenuItem("Create Entity")) {
                SceneManager::pActiveScene->CreateEntity("Test");
            }
            ImGui::EndPopup();
        }

        ImGui::End();
    }

    void HeirarchyPanel::OnDestroy(){
        
    }
}