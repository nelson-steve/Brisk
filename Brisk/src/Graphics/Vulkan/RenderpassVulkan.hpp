#pragma once

#include "CommandBufferVulkan.hpp"
#include "Engine/Renderer/RenderPass.hpp"

#include <volk.h>
#include <iostream>
#include <Engine/Renderer/Framebuffer.hpp>

namespace Brisk 
{
    class RenderPassVulkan : public RenderPass {
    public:
        RenderPassVulkan() = default;

        virtual void Init(const std::vector<RenderPassDependency>& dependencies, const std::vector<RenderPassAttachment>& attachments) override;
        virtual void Release() override;

        virtual void Begin(std::shared_ptr<CommandBuffer> cmd, std::shared_ptr<Framebuffer> framebuffer) override;
        virtual void End(std::shared_ptr<CommandBuffer> cmd) override;

        virtual VkRenderPass GetRenderPass() { return m_RenderPass; }
    private:
        VkRenderPass m_RenderPass;
        //std::vector<VkFramebuffer> m_Framebuffers;
    };
}