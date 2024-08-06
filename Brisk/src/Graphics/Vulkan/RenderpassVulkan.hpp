#pragma once

#include <Volk/volk.h>

#include <vector>
#include "CommandBufferVulkan.hpp"

namespace Brisk {
    class RenderpassVulkan {
    public:
        void Create(uint16_t noOfFrameBuffers);
        void Release();

        void BeginRenderPass(int imageIndex);
        void EndRenderPass(int imageIndex);
        void BindPipeline(VkPipeline pipeline);
        VkRenderPass GetRenderPass() const { return m_RenderPass; }
        const std::vector<VkFramebuffer> GetFramebuffers() const { return m_Framebuffers; }

    private:
        CommandBufferVulkan* m_CommandBuffer;
        VkRenderPass m_RenderPass;
        std::vector<VkFramebuffer> m_Framebuffers;

        std::vector<VkAttachmentDescription> m_Attachments;
        std::vector<VkSubpassDescription> m_Subpasses;
        std::vector<VkSubpassDependency> m_Dependencies;

    };
}