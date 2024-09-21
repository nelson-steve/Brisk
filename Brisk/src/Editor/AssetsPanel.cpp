#include "AssetsPanel.hpp"

namespace Brisk 
{
    void AssetsPanel::OnCreate() {
        m_Assets = {
            {"Folder 1", true, {{"Asset 1", false}, {"Asset 2", false}}},
            {"Folder 2", true, {{"Asset 3", false}, {"Folder 2.1", true, {{"Asset 4", false}}}}},
            {"Asset 5", false}
        };
    }

    void AssetsPanel::OnUpdate() {
        // Begin the assets window
        ImGui::Begin("Assets");

        // Iterate over the root-level assets (could be folders or files)
        for (const auto& asset : m_Assets)
        {
            ShowAssetFolder(asset);
        }

        // End the assets window
        ImGui::End();
    }

    void AssetsPanel::ShowAssetFolder(const Asset& asset)
    {
        if (asset.isFolder)
        {
            if (ImGui::TreeNode(asset.name.c_str())) // Folder node
            {
                for (const auto& child : asset.children)
                {
                    ShowAssetFolder(child); // Recursively show child assets/folders
                }
                ImGui::TreePop(); // Close the folder node
            }
        }
        else
        {
            // Show individual asset
            ImGui::Text(asset.name.c_str());
        }
    }

    void AssetsPanel::OnDestroy() {
        //m_SceneTexture->Release();
    }
}