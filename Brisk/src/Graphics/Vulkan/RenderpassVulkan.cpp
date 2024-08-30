#include "RenderPassVulkan.hpp"

#include "Engine/Engine.hpp"
#include "SwapchainVulkan.hpp"
#include "GraphicsPipelineVulkan.hpp"
#include "GPUContextVulkan.hpp"

namespace Brisk {
    void RenderPassVulkan::Create(std::vector<VkAttachmentDescription> attachments, std::vector<VkSubpassDescription> subpasses, std::vector<VkSubpassDependency> dependencies) {
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
        renderPassInfo.pSubpasses = subpasses.data();
        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pDependencies = dependencies.data();

        if (vkCreateRenderPass(GpuContextVulkan::s_GPUDevice->GetDevice(), &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }

        m_CommandBuffer = new CommandBufferVulkan();
        m_CommandBuffer->Allocate(GpuContextVulkan::GetCommandPool());
    }

    void RenderPassVulkan::CreateNAddFramebuffer(std::vector<VkImageView> attachments, uint32_t width, uint32_t height) {
        VkFramebuffer framebuffer;
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_RenderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = width;
        framebufferInfo.height = height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(GpuContextVulkan::s_GPUDevice->GetDevice(), &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }

        m_Framebuffers.push_back(framebuffer);
    }

    void RenderPassVulkan::ReleaseFramebuffers() {
        for (size_t i = 0; i < m_Framebuffers.size(); i++) {
            vkDestroyFramebuffer(GpuContextVulkan::s_GPUDevice->GetDevice(), m_Framebuffers[i], nullptr);
        }
    }

    void RenderPassVulkan::BeginRenderPass(/*int imageIndex*/) {
        Reset();
        m_CommandBuffer->Begin();
        int imageIndex = GpuContextVulkan::GetImageIndex(); // TODO: Take this value from Swapchain
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_RenderPass;
        renderPassInfo.framebuffer = m_Framebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = static_cast<SwapchainVulkan*>(Engine::s_Swapchain)->GetExtent();

        std::vector<VkClearValue> clearColors = { {{0.0f, 0.0f, 0.0f, 1.0f}}, { 1.0f, 0 } };
        renderPassInfo.clearValueCount = clearColors.size();
        renderPassInfo.pClearValues = clearColors.data();

        vkCmdBeginRenderPass(m_CommandBuffer->Get(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void RenderPassVulkan::EndRenderPass() {
        vkCmdEndRenderPass(m_CommandBuffer->Get());

        if (vkEndCommandBuffer(m_CommandBuffer->Get()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer!");
        }
    }

    void RenderPassVulkan::BindPipeline(void* pipeline) {
        vkCmdBindPipeline(m_CommandBuffer->Get(), VK_PIPELINE_BIND_POINT_GRAPHICS,
            static_cast<GraphicsPipelineVulkan*>(pipeline)->GetPipeline());
    }

    void RenderPassVulkan::Release() {
        for (int i = 0; i < m_Framebuffers.size(); i++) {
            vkDestroyFramebuffer(GpuContextVulkan::s_GPUDevice->GetDevice(), m_Framebuffers[i], nullptr);
        }
        vkDestroyRenderPass(GpuContextVulkan::s_GPUDevice->GetDevice(), m_RenderPass, nullptr);
    }
}