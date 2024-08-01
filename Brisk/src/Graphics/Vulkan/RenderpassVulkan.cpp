#include "RenderpassVulkan.hpp"

#include "Engine/Engine.hpp"
#include "SwapchainVulkan.hpp"

namespace Brisk {
    void RenderpassVulkan::Create(uint16_t noOfFrameBuffers) {
        VkAttachmentDescription colorAttachment{};
        //colorAttachment.format = Engine::m_Swapchain->GetFormat();
        colorAttachment.format = VK_FORMAT_B8G8R8A8_SRGB; // TODO: DO NOT use hardcoded value
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        if (vkCreateRenderPass(Engine::s_PhysicalDevice->GetDevice(), &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }

        m_Framebuffers.resize(noOfFrameBuffers);
        for (size_t i = 0; i < m_Framebuffers.size(); i++) {
            VkImageView attachments[] = {
                static_cast<SwapchainVulkan*>(Engine::s_Swapchain)->GetSwapchainImageViews()[i]
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_RenderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = static_cast<SwapchainVulkan*>(Engine::s_Swapchain)->GetExtentWidth();
            framebufferInfo.height = static_cast<SwapchainVulkan*>(Engine::s_Swapchain)->GetExtentHeight();
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(Engine::s_PhysicalDevice->GetDevice(), &framebufferInfo, nullptr, &m_Framebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    void RenderpassVulkan::Release() {
        for (int i = 0; i < m_Framebuffers.size(); i++) {
            vkDestroyFramebuffer(Engine::s_PhysicalDevice->GetDevice(), m_Framebuffers[i], nullptr);
        }
        vkDestroyRenderPass(Engine::s_PhysicalDevice->GetDevice(), m_RenderPass, nullptr);
    }
}