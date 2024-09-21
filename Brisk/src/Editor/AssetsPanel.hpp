#pragma once

#include "IEditorPanel.hpp"

#include <string>
#include <vector>

namespace Brisk {
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
    private:
        std::vector<Asset> m_Assets;
    };
}