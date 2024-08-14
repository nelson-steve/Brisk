#pragma once

#include "CommandBufferVulkan.hpp"
#include "Graphics/Renderpass.hpp"

#include <Volk/volk.h>

#include <vector>

namespace Brisk {
    class RenderPassVulkan : public RenderPass{
    public:
        void Create(uint16_t noOfFrameBuffers);
        void Release();

        virtual void BindPipeline(void* pipeline) override;
        virtual void BeginRenderPass(/*int imageIndex*/) override;
        virtual void EndRenderPass() override;

        void Reset() {
            vkResetCommandBuffer(m_CommandBuffer->Get(), /*VkCommandBufferResetFlagBits*/ 0);
        }
        VkCommandBuffer GetCommandBuffer() { return m_CommandBuffer->Get(); }
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