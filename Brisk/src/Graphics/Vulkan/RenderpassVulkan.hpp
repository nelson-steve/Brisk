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

        virtual void Init(const std::vector<RenderPassAttachment>& inputs, const std::vector<RenderPassAttachment>& outputs) override;

        virtual void Begin(std::shared_ptr<CommandBuffer> cmd) override;
        virtual void End(std::shared_ptr<CommandBuffer> cmd) override;
    private:
        std::vector<VkAttachmentDescription> m_AttachmentsDescriptions;
        //VkAttachmentDescription m_DepthAttachmentDescription;

        VkRenderPass m_RenderPass;
        VkDevice device;
        VkFramebuffer m_Framebuffer;
        VkCommandBuffer commandBuffer;
    };
}