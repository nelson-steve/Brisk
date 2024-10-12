#include "RenderPassVulkan.hpp"

#include "Engine/Engine.hpp"
#include "SwapchainVulkan.hpp"
#include "GraphicsPipelineVulkan.hpp"

namespace Brisk 
{
    void RenderPassVulkan::Init(const RenderPassSpecs& specs) {
        std::vector<VkAttachmentDescription> attachments;
        std::vector<VkAttachmentReference> colorAttachmentsRefs;
        VkAttachmentReference depthAttachmentRef;
        for (int i = 0; i < m_Attachments.size(); i++) {
            attachments.reserve(5);
            VkAttachmentDescription attachment{};
            attachment.format = m_Attachments[0].format;
            attachment.samples = VK_SAMPLE_COUNT_1_BIT;
            if (specs.pAttachments[i].pClear) {
                attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            }
            else {
                attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            }
            attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            if (specs.pAttachments[i].pIsDepth) {
                VkAttachmentReference depthAttachmentRef{};
                depthAttachmentRef.attachment = 0;
                depthAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                colorAttachmentsRefs.push_back(depthAttachmentRef);
            }
            else {
                VkAttachmentReference colorAttachmentRef{};
                colorAttachmentRef.attachment = 0;
                colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                colorAttachmentsRefs.push_back(colorAttachmentRef);
            }
        }

        std::vector<VkSubpassDescription> subpasses;
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentsRefs.size());
        subpass.pColorAttachments = colorAttachmentsRefs.data();
        subpass.pDepthStencilAttachment = &depthAttachmentRef;
        subpasses.push_back(subpass);

        std::vector<VkSubpassDependency> dependencies;
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies.push_back(dependency);

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
    }

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
        m_Framebuffers.clear();
    }

    void RenderPassVulkan::BeginRenderPass(CommandBufferVulkan* commandBuffer, int imageIndex) {
        commandBuffer->Begin();
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_RenderPass;
        renderPassInfo.framebuffer = m_Framebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = static_cast<SwapchainVulkan*>(Engine::s_Swapchain)->GetExtent();

        std::vector<VkClearValue> clearColors = { {{0.2f, 0.2f, 0.2f, 1.0f}}, { 1.0f, 0 } };
        renderPassInfo.clearValueCount = clearColors.size();
        renderPassInfo.pClearValues = clearColors.data();

        vkCmdBeginRenderPass(commandBuffer->Get(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void RenderPassVulkan::EndRenderPass(CommandBufferVulkan* commandBuffer, bool endCmdBuffer) {
        vkCmdEndRenderPass(commandBuffer->Get());
        if (!endCmdBuffer) return;
        if (vkEndCommandBuffer(commandBuffer->Get()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer!");
        }
    }

    void RenderPassVulkan::Release() {
        for (int i = 0; i < m_Framebuffers.size(); i++) {
            vkDestroyFramebuffer(GpuContextVulkan::s_GPUDevice->GetDevice(), m_Framebuffers[i], nullptr);
        }
        vkDestroyRenderPass(GpuContextVulkan::s_GPUDevice->GetDevice(), m_RenderPass, nullptr);
    }
}