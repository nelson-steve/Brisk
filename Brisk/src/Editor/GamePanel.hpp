#pragma once

#include "IEditorPanel.hpp"
#include "Graphics/Vulkan/TextureVulkan.hpp"

namespace Brisk {
    class GamePanel : public IEditorPanel {
    public:
        virtual void OnCreate() override;
        virtual void OnUpdate() override;
        virtual void OnDestroy() override;
    private:
        BriskTexture* m_SceneTexture;
        glm::vec2 m_ViewportSize;
        VkDescriptorSet m_SceneDescriptorSet;
    };
}