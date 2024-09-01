#pragma once

#include "Engine/Renderer/Renderer.hpp"
#include "../GpuContextVulkan.hpp"
#include "../BufferVulkan.hpp"

namespace Brisk {
	class RendererVulkan : public Renderer {
	public:
		virtual void Create() override;
		virtual void Release() override;

		virtual void SetupRenderingPipeline(Swapchain* swapchain) override;

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

		GpuContextVulkan* m_GpuContext;
		RenderPassVulkan* m_RenderPass;
		GraphicsPipelineVulkan* m_Pipeline;
		SwapchainVulkan* m_Swapchain;
		BufferVulkan* m_VertexBuffer;
		BufferVulkan* m_UniformBuffer;
		void* m_UniformBufferData;

		CommandBufferVulkan* m_CommandBuffer;

		VkCommandPool m_CommandPool;
		VkFence m_InFlightFence;
		VkSemaphore m_RenderFinishedSemaphore;
		VkSemaphore m_ImageAvailableSemaphore;

		VkDescriptorPool m_DescriptorPool;
		VkDescriptorSet m_DescriptorSet;
		std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;

		std::vector<Point> Vertices;

		friend class RendererFactory;
	};
}