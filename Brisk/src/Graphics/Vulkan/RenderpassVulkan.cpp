#include "RenderPassVulkan.hpp"

#include "Engine/Engine.hpp"
#include "SwapchainVulkan.hpp"
#include "PipelineVulkan.hpp"
#include "UtilitiesVulkan.hpp"
#include "TextureVulkan.hpp"

namespace Brisk 
{
    void RenderPassVulkan::AddInputAttachment(RenderPassAttachment attachment) {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = VK_FORMAT_R8G8B8A8_UNORM;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        m_ColorAttachments.push_back(colorAttachment);
    }

    void RenderPassVulkan::AddOutputAttachment(RenderPassAttachment attachment) {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = VK_FORMAT_R8G8B8A8_UNORM;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        m_ColorAttachments.push_back(colorAttachment);
    }

    void RenderPassVulkan::Init() {
        std::vector<VkAttachmentReference> colorRefs;
        for (int i = 0; i < m_ColorAttachments.size(); i++) { 
            VkAttachmentReference colorAttachmentRef{};
            colorAttachmentRef.attachment = i;
            colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorRefs.push_back(colorAttachmentRef);
        }

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = colorRefs.size() + 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = colorRefs.size();
        subpass.pColorAttachments = colorRefs.data();
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &m_ColorAttachments;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_RenderPass);
    }

    void RenderPassVulkan::Begin() {
        std::vector<VkClearValue> clearValues;
        clearValues.resize(m_ColorAttachments.size());
        for (auto& clear : clearValues) {
            clear.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
            clear.depthStencil = { 1.0f, 0 };
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_RenderPass;
        renderPassInfo.framebuffer = framebuffer;
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = extent;
        renderPassInfo.clearValueCount = 2;
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(cmd->Get(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void RenderPassVulkan::End() {
        vkCmdEndRenderPass(cmd->Get());
    }

	//void Init(std::vector<std::shared_ptr<Texture>> inputs, std::vector<std::shared_ptr<Texture>> outputs) {
 //       std::vector<VkAttachmentDescription> attachmentDescriptions;
 //       std::vector<VkAttachmentReference> colorAttachmentRefs;

 //       for (size_t i = 0; i < outputs.size(); ++i) {
 //           auto& texture = outputs[i];

 //           VkAttachmentDescription attachment{};
 //           attachment.format = std::make_shared<TextureVulkan>(texture)->GetFormat();
 //           attachment.samples = VK_SAMPLE_COUNT_1_BIT;
 //           attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
 //           attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
 //           attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
 //           attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
 //           attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
 //           attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

 //           attachmentDescriptions.push_back(attachment);

 //           VkAttachmentReference colorRef{};
 //           colorRef.attachment = static_cast<uint32_t>(i);
 //           colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

 //           colorAttachmentRefs.push_back(colorRef);
 //       }

 //       bool hasDepth = false;
 //       VkAttachmentReference depthAttachmentRef{};
 //       if (!outputs.empty() && outputs.back()->IsDepthFormat()) {
 //           hasDepth = true;

 //           auto& depthTex = outputs.back();
 //           VkAttachmentDescription depthAttachment{};
 //           depthAttachment.format = std::make_shared<TextureVulkan>(depthTex)->GetFormat();
 //           depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
 //           depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
 //           depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
 //           depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
 //           depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
 //           depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
 //           depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

 //           attachmentDescriptions.push_back(depthAttachment);

 //           depthAttachmentRef.attachment = static_cast<uint32_t>(attachmentDescriptions.size() - 1);
 //           depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
 //       }

 //       VkSubpassDescription subpass{};
 //       subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
 //       subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
 //       subpass.pColorAttachments = colorAttachmentRefs.data();
 //       if (hasDepth)
 //           subpass.pDepthStencilAttachment = &depthAttachmentRef;

 //       VkSubpassDependency dependency{};
 //       dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
 //       dependency.dstSubpass = 0;
 //       dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
 //       dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
 //       dependency.srcAccessMask = 0;
 //       dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

 //       VkRenderPassCreateInfo renderPassInfo{};
 //       renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
 //       renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
 //       renderPassInfo.pAttachments = attachmentDescriptions.data();
 //       renderPassInfo.subpassCount = 1;
 //       renderPassInfo.pSubpasses = &subpass;
 //       renderPassInfo.dependencyCount = 1;
 //       renderPassInfo.pDependencies = &dependency;

 //       if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS) {
 //           throw std::runtime_error("Failed to create render pass.");
 //       }
	//}
}