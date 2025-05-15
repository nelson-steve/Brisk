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

            if ((attachment.pImage->GetSpecs().p_Usage & Texture::TextureUsage::ImageUsageColorAttachment) != Texture::TextureUsage::Undefined) {
                desc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }
            if ((attachment.pImage->GetSpecs().p_Usage & Texture::TextureUsage::ImageUsageTransferSrc) != Texture::TextureUsage::Undefined) {
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

        //VkSubpassDependency dependency1{};
        //dependency1.srcSubpass = VK_SUBPASS_EXTERNAL;
        //dependency1.dstSubpass = 0;
        //dependency1.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        //dependency1.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        //dependency1.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        //dependency1.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        //dependency1.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkSubpassDependency dependency1{};
        dependency1.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency1.dstSubpass = 0;
        dependency1.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency1.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency1.srcAccessMask = 0;
        dependency1.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependency1.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkSubpassDependency dependency2{};
        dependency2.srcSubpass = 0;
        dependency2.dstSubpass = VK_SUBPASS_EXTERNAL;
        dependency2.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency2.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependency2.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency2.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependency2.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkSubpassDependency dependencyDepth{};
        dependencyDepth.srcSubpass = 0;
        dependencyDepth.dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencyDepth.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependencyDepth.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencyDepth.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencyDepth.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencyDepth.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;


        ///
        
        VkSubpassDependency lightingDependencyIn{};
        lightingDependencyIn.srcSubpass = VK_SUBPASS_EXTERNAL;  // previous pass
        lightingDependencyIn.dstSubpass = 0; // lighting pass subpass index
        lightingDependencyIn.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        lightingDependencyIn.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        lightingDependencyIn.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        lightingDependencyIn.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        lightingDependencyIn.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkSubpassDependency lightingDependencyOut{};
        lightingDependencyOut.srcSubpass = 0;  // lighting pass
        lightingDependencyOut.dstSubpass = VK_SUBPASS_EXTERNAL;
        lightingDependencyOut.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        lightingDependencyOut.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        lightingDependencyOut.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        lightingDependencyOut.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        lightingDependencyOut.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;


        ///

        std::vector<VkSubpassDependency> dependencies;
        if (m_ColorAttachmentCount > 1) {
            dependencies.push_back(dependency1);
            dependencies.push_back(dependency2);
            if (m_HasDepth)
                dependencies.push_back(dependencyDepth);
        }
        else {
            //dependencies.push_back(dependency1);
            dependencies.push_back(lightingDependencyIn);
            dependencies.push_back(lightingDependencyOut);
        }

        VkRenderPassCreateInfo renderPassInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = dependencies.size();
        renderPassInfo.pDependencies = dependencies.data();

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