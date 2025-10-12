#include "MaterialPanel.hpp"

#include "EditorLayer.hpp"

namespace Brisk 
{
    void MaterialPanel::OnCreate() {
    }

    void MaterialPanel::OnUpdate() {
        ImGui::Begin("Material");

        SceneManager::pActiveScene->Reg().get<TransformComponent>(SceneManager::pActiveScene->GetSelectedEntity());

        Entity entity = { SceneManager::pActiveScene->GetSelectedEntity(), SceneManager::pActiveScene.get() };
        if (entity.HasComponent<MeshComponent>()) {
            //for (auto& primitive : entity.GetComponent<MeshComponent>().p_Mesh->m_Meshes[entity.GetComponent<MeshComponent>().p_SubMeshIndex].primitives) {
            //    ImGui::SliderFloat("AlphaCutOff", &entity.GetComponent<MeshComponent>().p_Mesh->m_Materials[primitive.materialIndex].alphaCutoff, 0.0f, 1.0f);
            //    ImGui::SliderFloat("MetallicFactor", &entity.GetComponent<MeshComponent>().p_Mesh->m_Materials[primitive.materialIndex].metallicFactor, 0.0f, 1.0f);
            //    ImGui::SliderFloat("RoughnessFactor", &entity.GetComponent<MeshComponent>().p_Mesh->m_Materials[primitive.materialIndex].roughnessFactor, 0.0f, 1.0f);

            //    ImGui::SliderFloat("IOR", &entity.GetComponent<MeshComponent>().p_Mesh->m_Materials[primitive.materialIndex].ior, 0.0f, 1.0f);
            //    ImGui::SliderFloat("Dispersion", &entity.GetComponent<MeshComponent>().p_Mesh->m_Materials[primitive.materialIndex].dispersion, 0.0f, 1.0f);
            //    ImGui::SliderInt("DoubleSided", &entity.GetComponent<MeshComponent>().p_Mesh->m_Materials[primitive.materialIndex].doubleSided, 0.0f, 1.0f);
            //    ImGui::SliderInt("Unlit", &entity.GetComponent<MeshComponent>().p_Mesh->m_Materials[primitive.materialIndex].unlit, 0.0f, 1.0f);

            //    ImGui::SliderFloat("EmissiveStrength", &entity.GetComponent<MeshComponent>().p_Mesh->m_Materials[primitive.materialIndex].emissiveStrength, 0.0f, 1.0f);
            //}
        }
        ImGui::End();
    }

    void MaterialPanel::OnDestroy() {
    }
}