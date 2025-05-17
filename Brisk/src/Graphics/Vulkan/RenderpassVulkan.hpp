#pragma once

#include "CommandBufferVulkan.hpp"
#include "Engine/Renderer/RenderPass.hpp"

#include <Volk/volk.h>

#include <iostream>
#include <vector>

namespace Brisk 
{
    class RenderPassVulkan : public RenderPass {
    public:
        RenderPassVulkan() = default;

        virtual void Init(const std::vector<RenderPassAttachment>& inputs, const std::vector<RenderPassAttachment>& outputs) override;

        virtual void Begin(std::shared_ptr<CommandBuffer> cmd, uint32_t framebuffer) override;
        virtual void End(std::shared_ptr<CommandBuffer> cmd) override;

        virtual VkRenderPass GetRenderPass() { return m_RenderPass; }
    private:
        std::vector<VkAttachmentDescription> m_AttachmentsDescriptions;
        //VkAttachmentDescription m_DepthAttachmentDescription;

        VkRenderPass m_RenderPass;
        VkDevice device;
        std::vector<VkFramebuffer> m_Framebuffers;
        VkCommandBuffer commandBuffer;
    };
}