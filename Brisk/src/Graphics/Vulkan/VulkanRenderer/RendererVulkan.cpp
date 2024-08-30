#include "RendererVulkan.hpp"
#include "Graphics/Vulkan/SwapchainVulkan.hpp"
#include "Graphics/Vulkan/RenderPassVulkan.hpp"

namespace Brisk {
	void RendererVulkan::Create() {
		m_GpuContext = new GpuContextVulkan();
		m_GpuContext->Create();
	
		std::vector<GpuDeviceVulkan::QueueType> queueTypes;
		std::vector<GpuDeviceVulkan::DeviceFeatures> features;
		GpuDeviceVulkan::GpuRequirements req;
		m_GpuContext->CreateDevice(req);
	}

    void RendererVulkan::SetupSwapchain(const Swapchain* swap) {
        const SwapchainVulkan* swapchain = static_cast<const SwapchainVulkan*>(swap);

        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapchain->GetFormat().format;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentDescription depthAttatchment{};
        depthAttatchment.format = swapchain->GetDepthFormat();
        depthAttatchment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttatchment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttatchment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttatchment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttatchment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttatchment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttatchment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::vector<VkAttachmentDescription> attachments = { colorAttachment, depthAttatchment };

        RenderPassVulkan* m_RenderPass = new RenderPassVulkan();
        m_RenderPass->Create(attachments, { subpass }, { dependency });
        for (int i = 0; i < swapchain->GetImageCount(); i++) {
            std::vector<VkImageView> attachments = {
                swapchain->GetSwapchainImageViews()[i],
                swapchain->GetDepthImageView(),
            };
            m_RenderPass->CreateNAddFramebuffer(attachments, swapchain->GetExtentWidth(), swapchain->GetExtentHeight());
        }
    }

	void RendererVulkan::Release() {

	}

	void RendererVulkan::PreRender() {

	}

	void RendererVulkan::Render() {

	}

	void RendererVulkan::PostRender() {

	}
}