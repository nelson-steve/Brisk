#include "MaterialPanel.hpp"

#include "Editor.hpp"

namespace Brisk 
{
    void MaterialPanel::OnCreate() {
    }

    void MaterialPanel::OnUpdate() {
        ImGui::Begin("Material");

        auto& mesh = SceneManager::pActiveScene->Reg().get<MeshComponent>(SceneManager::pActiveScene->GetSelectedEntity());
//        Material* material = mesh.material; // Adjust if you use handle or ID

        if (material == nullptr) {
            ImGui::Text("No material assigned.");
            ImGui::End();
            return;
        }

        // Editable material properties
        ImGui::ColorEdit3("Albedo", glm::value_ptr(material->albedoColor));
        ImGui::SliderFloat("Metallic", &material->metallic, 0.0f, 1.0f);
        ImGui::SliderFloat("Roughness", &material->roughness, 0.0f, 1.0f);
        ImGui::SliderFloat("Emissive Intensity", &material->emissiveIntensity, 0.0f, 10.0f);

        // Toggles for using textures
        ImGui::Checkbox("Use Albedo Texture", &material->useTextureAlbedo);
        ImGui::Checkbox("Use Normal Map", &material->useTextureNormal);
        ImGui::Checkbox("Use Metal/Rough Texture", &material->useTextureMetalRough);

        ImGui::End();
    }

    void MaterialPanel::OnDestroy() {
    }
}