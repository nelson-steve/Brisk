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

        //TextureVulkan* m_Texture1 = new TextureVulkan();
        //m_Texture1->Create("../Data/Images/fbx.png");
        //FBXDescriptorSet = ImGui_ImplVulkan_AddTexture(m_Texture1->GetSampler(), m_Texture1->GetView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        //TextureVulkan* m_Texture2 = new TextureVulkan();
        //m_Texture2->Create("../Data/Images/folder.png");
        //FolderDescriptorSet = ImGui_ImplVulkan_AddTexture(m_Texture2->GetSampler(), m_Texture2->GetView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        //TextureVulkan* m_Texture3 = new TextureVulkan();
        //m_Texture3->Create("../Data/Images/obj.png");
        //OBJDescriptorSet = ImGui_ImplVulkan_AddTexture(m_Texture3->GetSampler(), m_Texture3->GetView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        //TextureVulkan* m_Texture4 = new TextureVulkan();
        //m_Texture4->Create("../Data/Images/photo.png");
        //PhotoDescriptorSet = ImGui_ImplVulkan_AddTexture(m_Texture4->GetSampler(), m_Texture4->GetView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    void AssetsPanel::OnUpdate() {
        ImGui::Begin("Assets");
        float windowVisibleX2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

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
        //m_SceneTexture->Release();
    }
}