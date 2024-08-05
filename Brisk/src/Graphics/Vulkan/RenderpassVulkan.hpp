#pragma once

#include <Volk/volk.h>

#include <vector>

namespace Brisk {
    class RenderpassVulkan {
    public:
        void Create(uint16_t noOfFrameBuffers);
        void Release();

        void BeginRenderPass(VkCommandBuffer commandBuffer, int imageIndex);
        VkRenderPass GetRenderPass() const { return m_RenderPass; }
        const std::vector<VkFramebuffer> GetFramebuffers() const { return m_Framebuffers; }

    private:
        VkRenderPass m_RenderPass;
        std::vector<VkFramebuffer> m_Framebuffers;

        std::vector<VkAttachmentDescription> m_Attachments;
        std::vector<VkSubpassDescription> m_Subpasses;
        std::vector<VkSubpassDependency> m_Dependencies;

    };
}