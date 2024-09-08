#pragma once

#include "Engine/Renderer/Renderer.hpp"
#include "../GpuContextVulkan.hpp"
#include "../BufferVulkan.hpp"
#include "../SwapchainVulkan.hpp"

#include "ImGuiBackends/imgui_impl_vulkan.h"

namespace Brisk {
	class RendererVulkan : public Renderer {
	public:
		virtual void Create() override;
		virtual void Release() override;

		virtual void SetupRenderingPipeline(Swapchain* swapchain) override;
		void SetupImGuiData(ImGui_ImplVulkan_InitInfo& data);

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
		void CreateViewportResources();
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
		VkSemaphore m_RenderFinishedSemaphore;
		VkSemaphore m_ImageAvailableSemaphore;
		VkCommandPool m_CommandPool;
		VkFence m_InFlightFence;

		VkDescriptorPool m_DescriptorPool;
		VkDescriptorSet m_DescriptorSet;
		std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;

		// Viewport
		std::vector<VkImage> m_ViewportImages;
		std::vector<VkDeviceMemory> m_ViewportImageMemory;
		std::vector<VkImageView> m_ViewportImageViews;

		RenderPassVulkan* m_ViewportRenderPass;
		GraphicsPipelineVulkan* m_ViewportPipeline;
		VkCommandPool m_ViewportCommandPool;
		CommandBufferVulkan* m_ViewportCommandBuffer;

		VkImage m_StagingImage;
		VkSampler m_StagingSampler;
		VkImageView m_StagingImageView;
		VkDeviceMemory m_StagingMemory;
		VkDescriptorSet m_ImGuiDescriptorSet;
		// Viewport end

		std::vector<Point> Vertices;

		friend class RendererFactory;
		friend class ViewportVulkan;
	};
}