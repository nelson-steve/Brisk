#include "RenderPassVulkan.hpp"

#include "Engine/Engine.hpp"
#include "SwapchainVulkan.hpp"
#include "PipelineVulkan.hpp"
#include "UtilitiesVulkan.hpp"
#include "TextureVulkan.hpp"

namespace Brisk
{
    void RenderPassVulkan::Init(const std::vector<RenderPassAttachment>& inputs, const std::vector<RenderPassAttachment>& outputs) {
        std::vector<VkAttachmentDescription> attachments;
        std::vector<VkAttachmentReference> colorAttachmentRefs;
        std::vector<VkAttachmentReference> inputAttachmentRefs;
        VkAttachmentReference depthAttachmentRef{};
        bool hasSwapchain = false;

        std::vector<VkImageView> imageViews;
        uint32_t width = 1920, height = 1061;

        int attachmentIndex = 0;
        for (const auto& attachment : outputs) {
            if (attachment.pAttachmentType == AttachmentType::SwapchainAttachment) {
                m_ColorAttachmentCount++;
                hasSwapchain = true;

                VkAttachmentDescription desc{};
                desc.format = std::static_pointer_cast<SwapchainVulkan>(Engine::s_Application->GetRenderer()->GetSwapchain())->GetFormat();
                desc.samples = VK_SAMPLE_COUNT_1_BIT;
                desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                desc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                
                desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                attachments.push_back(desc);

                VkAttachmentReference ref{};
                ref.attachment = attachmentIndex;

                ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                colorAttachmentRefs.push_back(ref);
            }
            else {
                if (!attachment.pImage->IsDepth()) {
                    m_ClearCount++;
                    m_ColorAttachmentCount++;
                }
                auto texture = std::static_pointer_cast<TextureVulkan>(attachment.pImage);
                VkAttachmentDescription desc{};
                desc.format = texture->GetFormat();
                desc.samples = VK_SAMPLE_COUNT_1_BIT;
                desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                desc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                if ((attachment.pImage->GetSpecs().p_Usage & Texture::TextureUsage::ImageUsageColorAttachment) != Texture::TextureUsage::Undefined) {
                    desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                }
                if ((attachment.pImage->GetSpecs().p_Usage & Texture::TextureUsage::ImageUsageTransferSrc) != Texture::TextureUsage::Undefined) {
                    desc.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                }
                if (attachment.pImage->IsDepth()) {
                    desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                    desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                }

                attachments.push_back(desc);
                imageViews.push_back(texture->GetView());

                VkAttachmentReference ref{};
                ref.attachment = attachmentIndex;

                if (attachment.pAttachmentType == AttachmentType::Color) {
                    ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    colorAttachmentRefs.push_back(ref);
                }
                if (attachment.pAttachmentType == AttachmentType::Depth) {
                    ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                    depthAttachmentRef = ref;
                    m_HasDepth = true;
                }
            }
            attachmentIndex++;
        }

        for (const auto& attachment : inputs) {
            auto texture = std::static_pointer_cast<TextureVulkan>(attachment.pImage);
            VkAttachmentDescription desc{};
            desc.format = texture->GetFormat();
            desc.samples = VK_SAMPLE_COUNT_1_BIT;
            desc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            desc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            desc.initialLayout = attachment.pAttachmentType == AttachmentType::Depth ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            desc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            attachments.push_back(desc);
            imageViews.push_back(texture->GetView());

            VkAttachmentReference ref{};
            ref.attachment = attachmentIndex;

            if (attachment.pAttachmentType == AttachmentType::Color) {
                ref.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                inputAttachmentRefs.push_back(ref);
            }
            if (attachment.pAttachmentType == AttachmentType::Depth) {
                ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
                inputAttachmentRefs.push_back(ref);
            }
            attachmentIndex++;
        }

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        if (colorAttachmentRefs.size() > 0) {
            subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
            subpass.pColorAttachments = colorAttachmentRefs.data();
        }        
        if (inputAttachmentRefs.size() > 0) {
            subpass.inputAttachmentCount = static_cast<uint32_t>(inputAttachmentRefs.size());
            subpass.pInputAttachments = inputAttachmentRefs.data();
        }
        if (m_HasDepth)
            subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkRenderPassCreateInfo renderPassInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 0;
        renderPassInfo.pDependencies = nullptr;

        if (vkCreateRenderPass(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan render pass");
        }

        if (hasSwapchain) {
            m_Framebuffers.resize(2);
            for (int i = 0; i < std::static_pointer_cast<SwapchainVulkan>(Engine::s_Application->GetRenderer()->GetSwapchain())->GetSwapchainImageViews().size(); i++) {
                std::vector<VkImageView> attachmentImages;
                attachmentImages.push_back(std::static_pointer_cast<SwapchainVulkan>(Engine::s_Application->GetRenderer()->GetSwapchain())->GetSwapchainImageViews()[i]);
                for (VkImageView image : imageViews) {
                    attachmentImages.push_back(image);
                }

                VkFramebufferCreateInfo framebufferInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
                framebufferInfo.renderPass = m_RenderPass;
                framebufferInfo.attachmentCount = static_cast<uint32_t>(attachmentImages.size());
                framebufferInfo.pAttachments = attachmentImages.data();
                framebufferInfo.width = width;
                framebufferInfo.height = height;
                framebufferInfo.layers = 1;

                if (vkCreateFramebuffer(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &framebufferInfo, nullptr, &m_Framebuffers[i]) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create Vulkan framebuffer");
                }
            }
        }
        else {
            m_Framebuffers.resize(1);
            VkFramebufferCreateInfo framebufferInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
            framebufferInfo.renderPass = m_RenderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(imageViews.size());
            framebufferInfo.pAttachments = imageViews.data();
            framebufferInfo.width = width;
            framebufferInfo.height = height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &framebufferInfo, nullptr, &m_Framebuffers[0]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create Vulkan framebuffer");
            }
        }
    }

    void RenderPassVulkan::Begin(std::shared_ptr<CommandBuffer> cmd, uint32_t framebuffer) {
        std::vector<VkClearValue> clearValues;
        clearValues.resize(m_ClearCount);

        for (auto& clear : clearValues) {
            clear.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
        }
        if (m_HasDepth) {
            VkClearValue depthClear{};
            depthClear.depthStencil = { 1.0f, 0 };
            clearValues.push_back(depthClear);
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_RenderPass;
        renderPassInfo.framebuffer = m_Framebuffers[framebuffer];
        renderPassInfo.renderArea.offset = { 0, 0 };
        VkExtent2D extent{ 1920, 1061 };
        renderPassInfo.renderArea.extent = extent;
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void RenderPassVulkan::End(std::shared_ptr<CommandBuffer> cmd) {
        vkCmdEndRenderPass(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get());
    }
}