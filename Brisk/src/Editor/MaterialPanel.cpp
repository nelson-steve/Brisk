#include "MaterialPanel.hpp"

#include "Editor.hpp"

namespace Brisk 
{
    void MaterialPanel::OnCreate() {
    }

    void MaterialPanel::OnUpdate() {
        ImGui::Begin("Material");

        SceneManager::pActiveScene->Reg().get<WorldTransformComponent>(SceneManager::pActiveScene->GetSelectedEntity());

        Entity entity = { SceneManager::pActiveScene->GetSelectedEntity(), SceneManager::pActiveScene.get() };
        if (entity.HasComponent<MeshComponent>()) {
            for (auto& primitive : entity.GetComponent<MeshComponent>().p_Mesh->m_Meshes[entity.GetComponent<MeshComponent>().p_SubMeshIndex].primitives) {
                ImGui::SliderFloat("AlphaCutOff", &entity.GetComponent<MeshComponent>().p_Mesh->m_Materials[primitive.materialIndex].alphaCutoff, 0.0f, 1.0f);
                ImGui::SliderFloat("MetallicFactor", &entity.GetComponent<MeshComponent>().p_Mesh->m_Materials[primitive.materialIndex].metallicFactor, 0.0f, 1.0f);
                ImGui::SliderFloat("RoughnessFactor", &entity.GetComponent<MeshComponent>().p_Mesh->m_Materials[primitive.materialIndex].roughnessFactor, 0.0f, 1.0f);

                ImGui::SliderFloat("IOR", &entity.GetComponent<MeshComponent>().p_Mesh->m_Materials[primitive.materialIndex].ior, 0.0f, 1.0f);
                ImGui::SliderFloat("Dispersion", &entity.GetComponent<MeshComponent>().p_Mesh->m_Materials[primitive.materialIndex].dispersion, 0.0f, 1.0f);
                ImGui::SliderInt("DoubleSided", &entity.GetComponent<MeshComponent>().p_Mesh->m_Materials[primitive.materialIndex].doubleSided, 0.0f, 1.0f);
                ImGui::SliderInt("Unlit", &entity.GetComponent<MeshComponent>().p_Mesh->m_Materials[primitive.materialIndex].unlit, 0.0f, 1.0f);

                ImGui::SliderFloat("EmissiveStrength", &entity.GetComponent<MeshComponent>().p_Mesh->m_Materials[primitive.materialIndex].emissiveStrength, 0.0f, 1.0f);
            }
        }
        //auto& mesh = SceneManager::pActiveScene->Reg().get<MeshComponent>(SceneManager::pActiveScene->GetSelectedEntity());
        //Material* material = mesh.material;

        //if (material == nullptr) {
        //    ImGui::Text("No material assigned.");
        //    ImGui::End();
        //    return;
        //}

        //// Editable material properties
        //ImGui::ColorEdit3("Albedo", glm::value_ptr(material->albedoColor));
        //ImGui::SliderFloat("Metallic", &material->metallic, 0.0f, 1.0f);
        //ImGui::SliderFloat("Roughness", &material->roughness, 0.0f, 1.0f);
        //ImGui::SliderFloat("Emissive Intensity", &material->emissiveIntensity, 0.0f, 10.0f);

        //// Toggles for using textures
        //ImGui::Checkbox("Use Albedo Texture", &material->useTextureAlbedo);
        //ImGui::Checkbox("Use Normal Map", &material->useTextureNormal);
        //ImGui::Checkbox("Use Metal/Rough Texture", &material->useTextureMetalRough);

        ImGui::End();
    }

    void MaterialPanel::OnDestroy() {
    }
}