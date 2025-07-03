#pragma once

#include "IEditorPanel.hpp"
#include "Engine/Renderer/Texture.hpp"

#include <string>
#include <vector>

namespace Brisk 
{
    struct Asset
    {
        std::string name;
        bool isFolder;
        std::vector<Asset> children; // Only used if isFolder is true
    };

    class AssetsPanel : public IEditorPanel {
    public:
        virtual void OnCreate() override;
        virtual void OnUpdate() override;
        virtual void OnDestroy() override;
    private:
        void ShowAssetFolder(const Asset& asset);

        std::shared_ptr<Texture> m_FBX;
        std::shared_ptr<Texture> m_Folder;
        std::shared_ptr<Texture> m_OBJ;
        std::shared_ptr<Texture> m_Photo;
    };
}