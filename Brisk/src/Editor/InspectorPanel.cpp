#include "InspectorPanel.hpp"

namespace Brisk
{
    void InspectorPanel::OnCreate() {
    }

    void InspectorPanel::OnUpdate() {
        ImGui::Begin("Inspector");
        //RenderTransformUI(t);
        ImGui::End();
    }

    //void AssetsPanel::RenderTransformUI(BTransform& transform) {
    //    //ImGui::Begin("Transform");

    //    // Position
    //    ImGui::Text("Position");
    //    ImGui::DragFloat3("##Position", &transform.position[0], 0.1f);

    //    // Rotation
    //    ImGui::Text("Rotation");
    //    ImGui::DragFloat3("##Rotation", &transform.rotation[0], 0.1f);

    //    // Scale
    //    ImGui::Text("Scale");
    //    ImGui::DragFloat3("##Scale", &transform.scale[0], 0.1f);

    //    //ImGui::End();
    //}

    void InspectorPanel::OnDestroy() {
        //m_SceneTexture->Release();
    }
}