#include "pch.hpp"
#include "CSMRenderPassVulkan.hpp"
#include <Graphics/Vulkan/GpuAdapterVulkan.hpp>
#include "TextureVulkan.hpp"
#include "CommandBufferVulkan.hpp"

namespace Brisk
{
	void CSMRenderPassVulkan::Init(const std::vector<std::shared_ptr<Texture>> framebuffers) {
		VkAttachmentDescription attachment{};
		attachment.format = VK_FORMAT_D16_UNORM;
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference attachmentRef{};
		attachmentRef.attachment = 0;
		attachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.pDepthStencilAttachment = &attachmentRef;

		VkSubpassDependency dependency1{};
		dependency1.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency1.dstSubpass = 0;
		dependency1.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependency1.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency1.srcAccessMask = VK_ACCESS_NONE;
		dependency1.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependency1.dependencyFlags = 0;

		VkRenderPassCreateInfo renderPassInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &attachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency1;

		if (vkCreateRenderPass(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create Vulkan render pass");
		}

		for (int i = 0; i < framebuffers.size(); i++) {
			VkFramebuffer framebuffer;
			VkFramebufferCreateInfo framebufferInfo { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
			framebufferInfo.renderPass = m_RenderPass;
			framebufferInfo.attachmentCount = 1;
			VkImageView view = std::static_pointer_cast<TextureVulkan>(framebuffers[i])->GetView();
			framebufferInfo.pAttachments = &view;
			framebufferInfo.width = 2048;
			framebufferInfo.height = 2048;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create Vulkan render pass");
			}
			m_Framebuffers.push_back(framebuffer);
		}
	}

	void CSMRenderPassVulkan::Release() {
		for (auto framebuffer : m_Framebuffers)
			vkDestroyFramebuffer(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), framebuffer, nullptr);

		vkDestroyRenderPass(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), m_RenderPass, nullptr);
	}

	void CSMRenderPassVulkan::Begin(std::shared_ptr<CommandBuffer> cmd, uint32_t framebuffer) {
		VkClearValue depthClear{};
		depthClear.depthStencil = { 1.0f, 0 };
		VkRenderPassBeginInfo renderPassBegin { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		renderPassBegin.renderPass = m_RenderPass;
		renderPassBegin.framebuffer = m_Framebuffers[framebuffer];
		renderPassBegin.renderArea.offset = { 0, 0 };
		renderPassBegin.renderArea.extent = { 2048, 2048 };
		renderPassBegin.clearValueCount = 1;
		renderPassBegin.pClearValues = &depthClear;

		vkCmdBeginRenderPass(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);
	}

	void CSMRenderPassVulkan::End(std::shared_ptr<CommandBuffer> cmd) {
		vkCmdEndRenderPass(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get());
	}
}