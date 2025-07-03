#pragma once

#include "CommandBufferVulkan.hpp"
#include "Engine/Renderer/RenderPass.hpp"

#include <volk.h>
#include <iostream>

namespace Brisk 
{
    class RenderPassVulkan : public RenderPass {
    public:
        RenderPassVulkan() = default;

        virtual void Init(const std::vector<RenderPassDependency>& dependencies, const std::vector<RenderPassAttachment>& outputs) override;
        virtual void Release() override;

        virtual void Begin(std::shared_ptr<CommandBuffer> cmd, uint32_t imageIndex = 0) override;
        virtual void End(std::shared_ptr<CommandBuffer> cmd) override;

        virtual VkRenderPass GetRenderPass() { return m_RenderPass; }
    private:
        std::vector<VkAttachmentDescription> m_AttachmentsDescriptions;

        uint32_t m_FramebufferWidth;
        uint32_t m_FramebufferHeight;

        VkRenderPass m_RenderPass;
        VkDevice device;
        std::vector<VkFramebuffer> m_Framebuffers;
        VkCommandBuffer commandBuffer;
    };
}