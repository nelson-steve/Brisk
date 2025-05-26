#include "RenderPassVulkan.hpp"

#include "Engine/Engine.hpp"
#include "SwapchainVulkan.hpp"
#include "PipelineVulkan.hpp"
#include "UtilitiesVulkan.hpp"
#include "TextureVulkan.hpp"

namespace Brisk
{
    void RenderPassVulkan::Init(const std::vector<RenderPassDependency>& dependencies, const std::vector<RenderPassAttachment>& outputs) {
        std::vector<VkAttachmentDescription> attachments;
        std::vector<VkAttachmentReference> colorAttachmentRefs;
        VkAttachmentReference depthAttachmentRef{};

        uint32_t attachmentIndex = 0;
        std::vector<VkImageView> imageViews;
        uint32_t width = 0, height = 0;

        for (const auto& attachment : outputs) {
            m_ClearCount++;
            if (!attachment.pImage->IsDepth()) {
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
            desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

            if ((attachment.pImage->GetSpecs().p_Usage & Core::TextureUsage::ImageUsageColorAttachment) != Core::TextureUsage::Undefined) {
                desc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }
            if ((attachment.pImage->GetSpecs().p_Usage & Core::TextureUsage::ImageUsageTransferSrc) != Core::TextureUsage::Undefined) {
                desc.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            }
            if (attachment.pImage->IsDepth()) {
                desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            }

            attachments.push_back(desc);
            imageViews.push_back(texture->GetView());

            if (width == 0 && height == 0) {
                width = texture->GetWidth();
                height = texture->GetHeight();
            }

            VkAttachmentReference ref{};
            ref.attachment = attachment.pBinding;

            if (attachment.pAttachmentType == AttachmentType::Color) {
                ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                colorAttachmentRefs.push_back(ref);
            }
            if (attachment.pAttachmentType == AttachmentType::Depth) {
                ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                depthAttachmentRef = ref;
                m_HasDepth = true;
            }

            ++attachmentIndex;
        };

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        if (colorAttachmentRefs.size() > 0) {
            subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
            subpass.pColorAttachments = colorAttachmentRefs.data();
        }
        if (m_HasDepth)
            subpass.pDepthStencilAttachment = &depthAttachmentRef;

        std::vector<VkSubpassDependency> subpassDependencies;
        for (const auto& dependency : dependencies) {
            VkSubpassDependency subpassDependency{};
            if (dependency.srcExternalPass) {
                subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
                subpassDependency.dstSubpass = 0;
            }
            else {
                subpassDependency.srcSubpass = 0;
                subpassDependency.dstSubpass = VK_SUBPASS_EXTERNAL;
            }
            subpassDependency.srcStageMask = UtilitiesVulkan::PipelineStageToVkPipelineStageFlags(dependency.srcStage);
            subpassDependency.dstStageMask = UtilitiesVulkan::PipelineStageToVkPipelineStageFlags(dependency.dstStage);
            subpassDependency.srcAccessMask = UtilitiesVulkan::AccessTypeToVkAccessFlags(dependency.srcAccess);
            subpassDependency.dstAccessMask = UtilitiesVulkan::AccessTypeToVkAccessFlags(dependency.dstAccess);
            subpassDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            subpassDependencies.push_back(subpassDependency);
        }

        VkRenderPassCreateInfo renderPassInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = subpassDependencies.size();
        renderPassInfo.pDependencies = subpassDependencies.data();

        if (vkCreateRenderPass(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan render pass");
        }

        VkFramebufferCreateInfo framebufferInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        framebufferInfo.renderPass = m_RenderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(imageViews.size());
        framebufferInfo.pAttachments = imageViews.data();
        framebufferInfo.width = width;
        framebufferInfo.height = height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &framebufferInfo, nullptr, &m_Framebuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan framebuffer");
        }
    }

    void RenderPassVulkan::Begin(std::shared_ptr<CommandBuffer> cmd) {
        std::vector<VkClearValue> clearValues;
        clearValues.resize(m_ColorAttachmentCount);

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
        renderPassInfo.framebuffer = m_Framebuffer;
        renderPassInfo.renderArea.offset = { 0, 0 };
        VkExtent2D extent{ 1920, 1080 };
        renderPassInfo.renderArea.extent = extent;
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void RenderPassVulkan::End(std::shared_ptr<CommandBuffer> cmd) {
        vkCmdEndRenderPass(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get());
    }
}