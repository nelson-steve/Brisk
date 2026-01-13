#include "MaterialPanel.hpp"

#include "EditorLayer.hpp"

namespace Brisk 
{
    void MaterialPanel::OnCreate() {
    }

    void MaterialPanel::OnUpdate() {
        ImGui::Begin("Material");

        //SceneManager::pActiveScene->Reg().get<TransformComponent>(SceneManager::pActiveScene->GetSelectedEntity());

        Entity entity = { SceneManager::pActiveScene->GetSelectedEntity(), SceneManager::pActiveScene.get() };
        if (entity.HasComponent<MeshComponent>()) {
            //for (int i = 0; i < entity.GetComponent<MeshComponent>().get.size(); i++) {
            //    ImGui::PushID(i);
            //    uint32_t matIndex = 0;
            //    //uint32_t matIndex = SceneManager::pActiveScene->GetMeshes()[i].materialIndex;

            //    if (ImGui::CollapsingHeader(("Material " + std::to_string(matIndex)).c_str())) {
            //        //ImGui::SliderFloat("AlphaCutOff", &SceneManager::pActiveScene->m_Materials[matIndex].alphaCutoff, 0.0f, 1.0f);
            //        //ImGui::SliderFloat("MetallicFactor", &SceneManager::pActiveScene->m_Materials[matIndex].metallicFactor, 0.0f, 1.0f);
            //        //ImGui::SliderFloat("RoughnessFactor", &SceneManager::pActiveScene->m_Materials[matIndex].roughnessFactor, 0.0f, 1.0f);
            //        //ImGui::SliderFloat("EmissiveStrength", &SceneManager::pActiveScene->m_Materials[matIndex].emissiveStrength, 0.0f, 1.0f);
            //    }

            //    ImGui::PopID();
            //}

        }
        ImGui::End();
    }

    void MaterialPanel::OnDestroy() {
    }
}