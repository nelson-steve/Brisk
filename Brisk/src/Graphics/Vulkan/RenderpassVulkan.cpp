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
        std::vector<VkAttachmentReference> refs;
        VkAttachmentReference depthRef;
        bool isDepth = false;
        for (const auto& attachment : outputs) {
            VkAttachmentDescription colorAttachmentDesc{};
            colorAttachmentDesc.format = std::static_pointer_cast<TextureVulkan>(attachment.pImage)->GetFormat();
            colorAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
            colorAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            colorAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachmentDesc.initialLayout = std::static_pointer_cast<TextureVulkan>(attachment.pImage)->GetCurrentLayout();
            colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            m_ColorAttachmentsDescription.push_back(colorAttachmentDesc);

            VkAttachmentReference colorAttachmentRef{};
            colorAttachmentRef.attachment = index;
            colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            refs.push_back(colorAttachmentRef);

            if (isDepth) {
                VkAttachmentDescription depthAttachmentDesc{};
                depthAttachmentDesc.format = std::static_pointer_cast<TextureVulkan>(attachment.pImage)->GetFormat();
                depthAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
                depthAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                depthAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                depthAttachmentDesc.initialLayout = std::static_pointer_cast<TextureVulkan>(attachment.pImage)->GetCurrentLayout();
                depthAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                m_DepthAttachmentDescription = depthAttachmentDesc;

                VkAttachmentReference depthAttachmentRef{};
                depthAttachmentRef.attachment = index;
                depthAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                refs.push_back(depthAttachmentRef);
            }
        }

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = m_ColorAttachmentsDescription.size();
        subpass.pColorAttachments = refs.data();
        subpass.pDepthStencilAttachment = &depthRef;

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_RenderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(m_ColorAttachmentsDescription.size() + m_DepthAttachmentDescription.size());

        // Combine color and depth attachments into a single array for the framebuffer
        std::vector<VkImageView> allAttachments;
        for (const auto& attachment : attachments) {
            allAttachments.push_back(std::static_pointer_cast<TextureVulkan>(attachment.pImage)->GetView());
        }

        framebufferInfo.pAttachments = allAttachments.data();
        framebufferInfo.width = specs.pWidth;
        framebufferInfo.height = specs.pHeight;
        framebufferInfo.layers = specs.pLayers;

        // Create the framebuffer
        if (vkCreateFramebuffer(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &framebufferInfo, nullptr, &m_Framebuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }


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
        renderPassInfo.pAttachments = m_ColorAttachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_RenderPass);

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_RenderPass; // The render pass associated with this framebuffer
        framebufferInfo.attachmentCount = 4; // Number of attachments (position, normal, albedo, specular)
        framebufferInfo.pAttachments = new VkImageView[4]{
            m_ColorAttachments[0],   // G-buffer attachment for position
            normalImageView,     // G-buffer attachment for normal
            albedoImageView,     // G-buffer attachment for albedo (base color)
            specularImageView    // G-buffer attachment for specular (optional)
        };
        framebufferInfo.width = swapchainExtent.width;  // Width of the framebuffer
        framebufferInfo.height = swapchainExtent.height; // Height of the framebuffer
        framebufferInfo.layers = 1; // Only one layer for this pass (this is a 2D framebuffer)

        VkFramebuffer framebuffer;
        vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer);

    }

    void RenderPassVulkan::Begin(std::shared_ptr<CommandBuffer> cmd) {
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

        vkCmdBeginRenderPass(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void RenderPassVulkan::End(std::shared_ptr<CommandBuffer> cmd) {
        vkCmdEndRenderPass(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get());
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