#pragma once

#include "CommandBufferVulkan.hpp"
#include "Engine/Renderer/RenderPass.hpp"

#include <Volk/volk.h>
#include <iostream>

namespace Brisk 
{
    class RenderPassVulkan : public RenderPass {
    public:
        RenderPassVulkan() = default;
        virtual void AddInputAttachment(RenderPassAttachment attachment) override;
        virtual void AddOutputAttachments(const std::vector<RenderPassAttachment>& attachments) override;

        virtual void Init() override;

        virtual void Begin(std::shared_ptr<CommandBuffer> cmd) override;
        virtual void End(std::shared_ptr<CommandBuffer> cmd) override;
    private:
        std::vector<VkAttachmentDescription> m_ColorAttachmentsDescription;
        VkAttachmentDescription m_DepthAttachmentDescription;

        VkRenderPass m_RenderPass;
        VkDevice device;
        VkFramebuffer m_Framebuffer;
        VkCommandBuffer commandBuffer;
    };
}