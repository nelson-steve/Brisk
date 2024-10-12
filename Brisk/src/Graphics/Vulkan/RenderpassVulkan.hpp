#pragma once

#include "CommandBufferVulkan.hpp"
#include "Engine/Renderer/RenderPass.hpp"

#include <Volk/volk.h>

#include <vector>

namespace Brisk {
    class RenderPassVulkan : public RenderPass {
    public:
        void Init(const RenderPassSpecs& specs);

        void ReleaseFramebuffers();
        void CreateNAddFramebuffer(std::vector<VkImageView> attachments, uint32_t width, uint32_t height);

        //void BindPipeline(void* pipeline);
        void BeginRenderPass(CommandBufferVulkan* commandBuffer, int imageIndex);
        void EndRenderPass(CommandBufferVulkan* commandBuffer, bool endCmdBuffer = true);

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