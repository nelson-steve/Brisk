#pragma once

#include "CommandBufferVulkan.hpp"

#include <Volk/volk.h>

#include <vector>

namespace Brisk {
    class RenderPassVulkan {
    public:
        void Create(std::vector<VkAttachmentDescription> attachments, std::vector<VkSubpassDescription> subpasses, std::vector<VkSubpassDependency> dependencies);
        void Release();
        void CreateFramebuffers();
        void ReleaseFramebuffers();
        void CreateNAddFramebuffer(std::vector<VkImageView> attachments, uint32_t width, uint32_t height);

        void BindPipeline(void* pipeline);
        void BeginRenderPass(/*int imageIndex*/);
        void EndRenderPass();

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