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
        virtual void AddOutputAttachment(RenderPassAttachment attachment)  override;
        virtual void Init() override;

        virtual void Begin(std::shared_ptr<CommandBuffer> cmd) override;
        virtual void End(std::shared_ptr<CommandBuffer> cmd) override;
    private:
        std::vector<VkAttachmentDescription> m_ColorAttachments;
        VkAttachmentDescription m_DepthAttachment;

        VkRenderPass m_RenderPass;
        VkDevice device;
        VkFramebuffer framebuffer;
        VkCommandBuffer commandBuffer;
    };
}