#include "FramebufferVulkan.hpp"
#include "Engine/Engine.hpp"
#include "SwapchainVulkan.hpp"

namespace Brisk {
	void FramebufferVulkan::Create() {
        m_SwapchainFramebuffers.resize(static_cast<SwapchainVulkan*>(Engine::m_Swapchain)->GetSwapchainImageViews().size());
        for (size_t i = 0; i < m_SwapchainFramebuffers.size(); i++) {
            VkImageView attachments[] = {
                static_cast<SwapchainVulkan*>(Engine::m_Swapchain)->GetSwapchainImageViews()[i]
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = static_cast<GraphicsDeviceVulkan*>(Engine::m_GPUDevice)->m_GraphicsPipeline->GetRenderPass();
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = static_cast<SwapchainVulkan*>(Engine::m_Swapchain)->GetExtentWidth();
            framebufferInfo.height = static_cast<SwapchainVulkan*>(Engine::m_Swapchain)->GetExtentHeight();
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(Engine::s_PhysicalDevice->GetDevice(), &framebufferInfo, nullptr, &m_SwapchainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
	}

    void FramebufferVulkan::Release() {
        for (auto framebuffer : m_SwapchainFramebuffers) {
            vkDestroyFramebuffer(Engine::s_PhysicalDevice->GetDevice(), framebuffer, nullptr);
        }
    }
}