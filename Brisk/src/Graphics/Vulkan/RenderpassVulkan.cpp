#include "pch.hpp"
#include "RenderPassVulkan.hpp"

#include "Engine/Engine.hpp"
#include "SwapchainVulkan.hpp"
#include "PipelineVulkan.hpp"
#include "UtilitiesVulkan.hpp"
#include "TextureVulkan.hpp"
#include "FramebufferVulkan.hpp"

namespace Brisk
{
    void RenderPassVulkan::Init(const std::vector<RenderPassDependency>& dependencies, const std::vector<RenderPassAttachment>& renderAttachments) {
        std::vector<VkAttachmentDescription> attachments;
        std::vector<VkAttachmentReference> colorAttachmentRefs;
        VkAttachmentReference depthAttachmentRef{};

        uint32_t attachmentIndex = 0;
        std::vector<std::vector<VkImageView>> imageViews;
        std::vector<VkAttachmentDescription> attachmentDescs;

        for (const auto& attachment : renderAttachments) {
            VkAttachmentDescription desc{};
            desc.format = UtilitiesVulkan::FormatToVkFormat(attachment.p_Format);
            desc.samples = VK_SAMPLE_COUNT_1_BIT;

            desc.loadOp = UtilitiesVulkan::LoadOpToVkType(attachment.p_LoadOp);
            desc.storeOp = UtilitiesVulkan::StoreOpToVkType(attachment.p_StoreOp);

            desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

            desc.initialLayout = UtilitiesVulkan::ImageLayoutToVkImageLayout(attachment.p_InitialLayout);
            desc.finalLayout = UtilitiesVulkan::ImageLayoutToVkImageLayout(attachment.p_FinalLayout);

            attachmentDescs.push_back(desc);

            VkAttachmentReference ref{};
            ref.attachment = attachment.p_Binding;

            if (attachment.p_AttachmentType == AttachmentType::Color || attachment.p_AttachmentType == AttachmentType::Swapchain) {
                ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                colorAttachmentRefs.push_back(ref);
                m_ColorAttachmentCount++;
            }
            if (attachment.p_AttachmentType == AttachmentType::Depth) {
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

            subpassDependency.srcSubpass = dependency.srcSubpass == -1 ? VK_SUBPASS_EXTERNAL : dependency.srcSubpass;
            subpassDependency.dstSubpass = dependency.dstSubpass == -1 ? VK_SUBPASS_EXTERNAL : dependency.dstSubpass;

            subpassDependency.srcStageMask = UtilitiesVulkan::PipelineStageToVkPipelineStageFlags(dependency.srcStage);
            subpassDependency.dstStageMask = UtilitiesVulkan::PipelineStageToVkPipelineStageFlags(dependency.dstStage);
            subpassDependency.srcAccessMask = UtilitiesVulkan::AccessTypeToVkAccessFlags(dependency.srcAccess);
            subpassDependency.dstAccessMask = UtilitiesVulkan::AccessTypeToVkAccessFlags(dependency.dstAccess);
            subpassDependency.dependencyFlags = 0;

            subpassDependencies.push_back(subpassDependency);
        }

        VkRenderPassCreateInfo renderPassInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
        renderPassInfo.pAttachments = attachmentDescs.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = subpassDependencies.size();
        renderPassInfo.pDependencies = subpassDependencies.data();

        if (vkCreateRenderPass(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan render pass");
        }

        VkDebugUtilsObjectNameInfoEXT nameInfo = {};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = VK_OBJECT_TYPE_RENDER_PASS;
        nameInfo.objectHandle = (uint64_t)m_RenderPass;
        nameInfo.pObjectName = "renderpass";

#if _DEBUG
        vkSetDebugUtilsObjectNameEXT(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), &nameInfo);
#endif
    }

    void RenderPassVulkan::Begin(std::shared_ptr<CommandBuffer> cmd, std::shared_ptr<Framebuffer> framebuffer) {
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

        BRISK_CORE_ASSERT(framebuffer->GetWidth() != 0 && framebuffer->GetHeight() != 0);

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_RenderPass;
        renderPassInfo.framebuffer = std::static_pointer_cast<FramebufferVulkan>(framebuffer)->GetFramebuffer();
        renderPassInfo.renderArea.offset = { 0, 0 };
        VkExtent2D extent{};
        extent.width = framebuffer->GetWidth();
        extent.height = framebuffer->GetHeight();
        renderPassInfo.renderArea.extent = extent;
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void RenderPassVulkan::End(std::shared_ptr<CommandBuffer> cmd) {
        vkCmdEndRenderPass(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get());
    }

    void RenderPassVulkan::Release() {
        vkDestroyRenderPass(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), m_RenderPass, nullptr);
    }
}