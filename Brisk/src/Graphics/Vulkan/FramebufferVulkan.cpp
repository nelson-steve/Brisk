#include "pch.hpp"
#include "FramebufferVulkan.hpp"
#include "RenderpassVulkan.hpp"
#include "TextureVulkan.hpp"
#include "GpuAdapterVulkan.hpp"
#include "SwapchainVulkan.hpp"

namespace Brisk 
{
	void FramebufferVulkan::Init(const FramebufferSpecs& specs) {
        m_Width = specs.p_Width;
        m_Height = specs.p_Height;

        std::vector<VkImageView> imageViews;

        if (specs.swapchainIndex != -1) {
            imageViews.push_back(std::static_pointer_cast<SwapchainVulkan>(Application::GetRenderer()->GetSwapchain())->GetSwapchainImageViews()[specs.swapchainIndex]);
        }
        else {
            for (const auto& attachment : specs.p_Attachments) {
                std::shared_ptr<TextureVulkan> texture = std::static_pointer_cast<TextureVulkan>(attachment);
                imageViews.push_back(texture->GetView());
            }
        }

        VkFramebufferCreateInfo framebufferInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        framebufferInfo.renderPass = std::static_pointer_cast<RenderPassVulkan>(specs.p_RenderPass)->GetRenderPass();
        framebufferInfo.attachmentCount = static_cast<uint32_t>(imageViews.size());
        framebufferInfo.pAttachments = imageViews.data();
        framebufferInfo.width = specs.p_Width;
        framebufferInfo.height = specs.p_Height;
        framebufferInfo.layers = 1;

        BRISK_CORE_INFO("Framebuffer width: {} height: {}", framebufferInfo.width, framebufferInfo.height);

        if (vkCreateFramebuffer(Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &framebufferInfo, nullptr, &m_Framebuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan framebuffer");
        }
	}

	void FramebufferVulkan::Destroy() {

	}
}