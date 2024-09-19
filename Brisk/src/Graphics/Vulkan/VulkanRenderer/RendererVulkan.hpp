#pragma once

#include "Engine/Renderer/Renderer.hpp"
#include "../GpuContextVulkan.hpp"
#include "../BufferVulkan.hpp"
#include "../SwapchainVulkan.hpp"
#include "../TextureVulkan.hpp"

#include "ImGuiBackends/imgui_impl_vulkan.h"

namespace Brisk {
	class RendererVulkan : public Renderer {
	public:
		virtual void Create() override;
		virtual void Release() override;

		virtual void SetupRenderingPipeline(Swapchain* swapchain) override;
		void CreateTexture();
		void CreateOffscreenResources();
		void SetupImGuiData(ImGui_ImplVulkan_InitInfo& data);

		void InsertImageMemoryBarrier(VkCommandBuffer cmdbuffer,
			VkImage image,
			VkAccessFlags srcAccessMask,
			VkAccessFlags dstAccessMask,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkPipelineStageFlags srcStageMask,
			VkPipelineStageFlags dstStageMask,
			VkImageSubresourceRange subresourceRange)
		{
			VkImageMemoryBarrier imageMemoryBarrier{};
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.srcAccessMask = srcAccessMask;
			imageMemoryBarrier.dstAccessMask = dstAccessMask;
			imageMemoryBarrier.oldLayout = oldImageLayout;
			imageMemoryBarrier.newLayout = newImageLayout;
			imageMemoryBarrier.image = image;
			imageMemoryBarrier.subresourceRange = subresourceRange;

			vkCmdPipelineBarrier(
				cmdbuffer,
				srcStageMask,
				dstStageMask,
				0,
				0, nullptr,
				0, nullptr,
				1, &imageMemoryBarrier);
		}

		VkCommandBuffer GetCommandBuffer() const { return m_CommandBuffer->Get(); }
		const VkPipeline GetPipeline() const { return m_Pipeline->GetPipeline(); }

		virtual void PreRender() override;
		virtual void Render() override;
		virtual void PostRender() override;
		virtual void WaitDeviceIdle() override;
	private:
		RendererVulkan() = default;
		void CreateGraphicsPipeline();
		void CreateDescriptorSet();
		void UpdateUniformBuffer(uint32_t currentImage);
	private:
		uint32_t m_ImageIndex;

		// API specific handle for Swapchain - does not need to be released here(it will get released by the main Engine class)
		SwapchainVulkan* m_Swapchain;

		GpuContextVulkan* m_GpuContext;
		RenderPassVulkan* m_RenderPass;
		GraphicsPipelineVulkan* m_Pipeline;
		BufferVulkan* m_VertexBuffer;
		BufferVulkan* m_UniformBuffer;
		void* m_UniformBufferData;

		CommandBufferVulkan* m_CommandBuffer;
		CommandBufferVulkan* m_ImGuiCommandBuffer;
		VkSemaphore m_RenderFinishedSemaphore;
		VkSemaphore m_ImageAvailableSemaphore;
		VkCommandPool m_CommandPool;
		VkFence m_InFlightFence;


		std::vector<VkImageView> m_ViewportImageViews;
		std::vector<VkImage> m_ViewportImages;
		std::vector<VkDeviceMemory> m_ViewportImageMemory;
		VkDescriptorSet m_ImGuiDescriptorSet;

		RenderPassVulkan* m_ViewportRenderPass;
		GraphicsPipelineVulkan* m_ViewportPipeline;

		VkDescriptorPool m_DescriptorPool;
		VkDescriptorSet m_DescriptorSet;
		std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;

		VkSemaphore m_UIFinshedSemaphore;

		//VkSampler m_ViewportSampler;
		std::vector<VkDescriptorSet> m_ImGuiDescriptorSets;

		//RenderPassVulkan* m_ImguiRenderPass;

		struct Viewport {
			VkImage ColorImage;
			VkImageView pColorImageView;
			VkDeviceMemory ColorMemory;
			VkSampler pSampler;

			GraphicsPipelineVulkan* pPipeline;
			RenderPassVulkan* pRenderpass;
		} m_Viewport;
		//struct Offscreen {
		//	VkImage Image;
		//	VkImageView ImageView;
		//	VkDeviceMemory Memory;
		//} m_Offscreen;

		glm::vec2 m_ViewportSize;
		TextureVulkan* m_Texture;

		friend class RendererFactory;
	};
}