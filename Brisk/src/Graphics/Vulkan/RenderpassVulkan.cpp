#include "RenderPassVulkan.hpp"

#include "Engine/Engine.hpp"
#include "SwapchainVulkan.hpp"
#include "PipelineVulkan.hpp"
#include "UtilitiesVulkan.hpp"
#include "TextureVulkan.hpp"

namespace Brisk
{
    void RenderPassVulkan::Init(const std::vector<RenderPassAttachment>& inputs, const std::vector<RenderPassAttachment>& outputs) {
        int index = 0;
        std::vector<VkAttachmentReference> colorRefs;
        VkAttachmentReference depthRef;
        bool isDepth = false;
        for (const auto& attachment : outputs) {
            VkAttachmentDescription colorAttachmentDesc{};
            colorAttachmentDesc.format = std::static_pointer_cast<TextureVulkan>(attachment.pImage)->GetFormat();
            colorAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
            colorAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            colorAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            //colorAttachmentDesc.initialLayout = std::static_pointer_cast<TextureVulkan>(attachment.pImage)->GetCurrentLayout();
            colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            m_AttachmentsDescriptions.push_back(colorAttachmentDesc);

            VkAttachmentReference colorAttachmentRef{};
            colorAttachmentRef.attachment = index;
            colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorRefs.push_back(colorAttachmentRef);

            // TODO: Check if it is depth or not
            if (isDepth) {
                VkAttachmentDescription depthAttachmentDesc{};
                depthAttachmentDesc.format = std::static_pointer_cast<TextureVulkan>(attachment.pImage)->GetFormat();
                depthAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
                depthAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                depthAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                //depthAttachmentDesc.initialLayout = std::static_pointer_cast<TextureVulkan>(attachment.pImage)->GetCurrentLayout();
                depthAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                m_AttachmentsDescriptions.push_back(depthAttachmentDesc);

                VkAttachmentReference depthAttachmentRef{};
                depthAttachmentRef.attachment = index;
                depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
                depthRef = depthAttachmentRef;
            }
        }

        std::vector<VkAttachmentReference> inputRefs;
        for (const auto& attachment : inputs) {
            VkAttachmentDescription colorAttachmentDesc{};
            colorAttachmentDesc.format = std::static_pointer_cast<TextureVulkan>(attachment.pImage)->GetFormat();
            colorAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
            colorAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            colorAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            //colorAttachmentDesc.initialLayout = std::static_pointer_cast<TextureVulkan>(attachment.pImage)->GetCurrentLayout();
            colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            m_AttachmentsDescriptions.push_back(colorAttachmentDesc);

            VkAttachmentReference colorAttachmentRef{};
            colorAttachmentRef.attachment = index;
            colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorRefs.push_back(colorAttachmentRef);

            // TODO: Check if it is depth or not
            if (isDepth) {
                VkAttachmentDescription depthAttachmentDesc{};
                depthAttachmentDesc.format = std::static_pointer_cast<TextureVulkan>(attachment.pImage)->GetFormat();
                depthAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
                depthAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                depthAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                //depthAttachmentDesc.initialLayout = std::static_pointer_cast<TextureVulkan>(attachment.pImage)->GetCurrentLayout();
                depthAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                m_AttachmentsDescriptions.push_back(depthAttachmentDesc);

                VkAttachmentReference depthAttachmentRef{};
                depthAttachmentRef.attachment = index;
                depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
                depthRef = depthAttachmentRef;
            }
        }

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = colorRefs.size();
        subpass.pColorAttachments = colorRefs.data();
        subpass.pDepthStencilAttachment = &depthRef;
        subpass.pInputAttachments = ;
        subpass.inputAttachmentCount = ;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassCreateInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
        renderPassCreateInfo.attachmentCount = m_AttachmentsDescriptions.size();
        renderPassCreateInfo.pAttachments = m_AttachmentsDescriptions.data();
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpass;
        renderPassCreateInfo.dependencyCount = 1;
        renderPassCreateInfo.pDependencies = &dependency;

        std::vector<VkImageView> views;
        for (auto& image : outputs) {
            views.push_back(std::static_pointer_cast<TextureVulkan>(image.pImage)->GetView());
        }

        VkFramebufferCreateInfo framebufferInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        framebufferInfo.renderPass = m_RenderPass;
        framebufferInfo.attachmentCount = views.size();
        framebufferInfo.pAttachments = views.data();
        framebufferInfo.width = 1920;
        framebufferInfo.height = 1080;
        framebufferInfo.layers = 1;

        vkCreateFramebuffer(device, &framebufferInfo, nullptr, &m_Framebuffer);

    }

    void RenderPassVulkan::Begin(std::shared_ptr<CommandBuffer> cmd) {
        std::vector<VkClearValue> clearValues;
        clearValues.resize(m_AttachmentsDescriptions.size());
        for (auto& clear : clearValues) {
            clear.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
            clear.depthStencil = { 1.0f, 0 };
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_RenderPass;
        renderPassInfo.framebuffer = m_Framebuffer;
        renderPassInfo.renderArea.offset = { 0, 0 };
        VkExtent2D extent{ 1020, 1080 };
        renderPassInfo.renderArea.extent = extent;
        renderPassInfo.clearValueCount = clearValues.size();
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void RenderPassVulkan::End(std::shared_ptr<CommandBuffer> cmd) {
        vkCmdEndRenderPass(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get());
    }
}