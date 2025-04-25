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
        bool hasDepth = false;

        uint32_t attachmentIndex = 0;
        std::vector<VkImageView> imageViews;
        uint32_t width = 0, height = 0;

        auto processAttachment = [&](const RenderPassAttachment& attachment, bool isInput) {
            auto texture = std::static_pointer_cast<TextureVulkan>(attachment.pImage);
            VkAttachmentDescription desc{};
            desc.format = texture->GetFormat();
            desc.samples = VK_SAMPLE_COUNT_1_BIT;
            desc.loadOp = isInput ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
            desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            desc.finalLayout = isInput ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            attachments.push_back(desc);
            imageViews.push_back(texture->GetView());

            if (width == 0 && height == 0) {
                width = texture->GetWidth();
                height = texture->GetHeight();
            }

            VkAttachmentReference ref{};
            ref.attachment = attachmentIndex;

            if (attachment.pAttachmentType == AttachmentType::Color && !isInput) {
                ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                colorAttachmentRefs.push_back(ref);
            }
            else if (attachment.pAttachmentType == AttachmentType::Depth && !isInput) {
                ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                depthAttachmentRef = ref;
                hasDepth = true;
            }
            else if (isInput) {
                ref.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                inputAttachmentRefs.push_back(ref);
            }

            ++attachmentIndex;
            };

        for (const auto& in : inputs) processAttachment(in, true);
        for (const auto& out : outputs) processAttachment(out, false);

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
        if (hasDepth)
            subpass.pDepthStencilAttachment = &depthAttachmentRef;

        if (outputs) {

        }
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

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