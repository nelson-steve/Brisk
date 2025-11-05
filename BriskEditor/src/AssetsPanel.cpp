#include "AssetsPanel.hpp"

#include "Graphics/Vulkan/TextureVulkan.hpp"
#include "ImGuiBackends/imgui_impl_vulkan.h"

#include <filesystem>

namespace Brisk 
{
    std::vector<std::filesystem::directory_entry> assets;
    std::filesystem::directory_entry selectedAsset;
    std::string CurrentPath;
    VkDescriptorSet FBXDescriptorSet;
    VkDescriptorSet OBJDescriptorSet;
    VkDescriptorSet FolderDescriptorSet;
    VkDescriptorSet PhotoDescriptorSet;

    void LoadAssets(const std::string& path) {
        CurrentPath = path;
        assets.clear();
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (entry.is_regular_file() || entry.is_directory()) {
                assets.push_back(entry);
            }
        }
    }
    void AssetsPanel::OnCreate() {
        LoadAssets("../Data");

        m_FBX = Texture::Create();
        m_Folder = Texture::Create();
        m_OBJ = Texture::Create();
        m_Photo = Texture::Create();

        m_FBX->Init("../Data/Images/fbx.png");
        m_Folder->Init("../Data/Images/folder.png");
        m_OBJ->Init("../Data/Images/obj.png");
        m_Photo->Init("../Data/Images/photo.png");

        FBXDescriptorSet = ImGui_ImplVulkan_AddTexture(std::static_pointer_cast<TextureVulkan>(m_FBX)->GetSampler(), std::static_pointer_cast<TextureVulkan>(m_FBX)->GetView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        FolderDescriptorSet = ImGui_ImplVulkan_AddTexture(std::static_pointer_cast<TextureVulkan>(m_Folder)->GetSampler(), std::static_pointer_cast<TextureVulkan>(m_Folder)->GetView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        OBJDescriptorSet = ImGui_ImplVulkan_AddTexture(std::static_pointer_cast<TextureVulkan>(m_OBJ)->GetSampler(), std::static_pointer_cast<TextureVulkan>(m_OBJ)->GetView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        PhotoDescriptorSet = ImGui_ImplVulkan_AddTexture(std::static_pointer_cast<TextureVulkan>(m_Photo)->GetSampler(), std::static_pointer_cast<TextureVulkan>(m_Photo)->GetView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    void AssetsPanel::OnUpdate() {
        ImGui::Begin("Assets");
        float windowVisibleX2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

        if (ImGui::Button(" Load Model")) {
            SceneManager::pActiveScene->LoadGltfScene("../Data/Models/damaged_helmet/DamagedHelmet.gltf");
        }

        if (ImGui::Button(" Load Model1")) {
            SceneManager::pActiveScene->LoadGltfScene("../Data/Models/mecha_ramen_high_poly/scene.gltf");
        }

        if (ImGui::Button(" Load Model2")) {
            SceneManager::pActiveScene->LoadGltfScene("../Data/Models/modural_robot_mecha_chimera_dyan_high-poly_mesh/scene.gltf");
        }

        if (CurrentPath != "../Data") {
            if (ImGui::Button(" <-- ")) {
                std::filesystem::path parentPath = std::filesystem::path(CurrentPath).parent_path();
                LoadAssets(parentPath.string());
            }
        }

        for (int i = 0; i < assets.size(); i++) {
            std::string fileName = assets[i].path().filename().string();
            std::string extension = assets[i].path().extension().string();

            if (extension == ".png" || extension == ".jpg") {
                ImGui::Image((void*)(intptr_t)PhotoDescriptorSet, ImVec2(64, 64));
            }
            else if (extension == ".obj") {
                ImGui::Image((void*)(intptr_t)OBJDescriptorSet, ImVec2(64, 64));
            }
            else if (extension == ".fbx") {
                ImGui::Image((void*)(intptr_t)FBXDescriptorSet, ImVec2(64, 64));
            }
            else if(assets[i].is_directory()){
                ImGui::Image((void*)(intptr_t)FolderDescriptorSet, ImVec2(64, 64));
            }

            float lastItemX2 = ImGui::GetItemRectMax().x;
            float nextItemX2 = lastItemX2 + ImGui::GetStyle().ItemSpacing.x + 64;

            if (nextItemX2 < windowVisibleX2) {
                ImGui::SameLine();
            }

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                if (assets[i].is_directory()) {
                    LoadAssets(assets[i].path().string());
                    i = -1;
                }
                else {
                    selectedAsset = assets[i];
                }
            }
            else if (ImGui::IsItemClicked()) {
                selectedAsset = assets[i];
            }
        }

        ImGui::End();
    }

    void AssetsPanel::ShowAssetFolder(const Asset& asset)
    {
        if (asset.isFolder)
        {
            if (ImGui::TreeNode(asset.name.c_str()))
            {
                for (const auto& child : asset.children)
                {
                    ShowAssetFolder(child);
                }
                ImGui::TreePop();
            }
        }
        else
        {
            ImGui::Text(asset.name.c_str());
        }
    }

    void AssetsPanel::OnDestroy() {
        m_FBX->Release();
        m_Folder->Release();
        m_OBJ->Release();
        m_Photo->Release();
    }
}