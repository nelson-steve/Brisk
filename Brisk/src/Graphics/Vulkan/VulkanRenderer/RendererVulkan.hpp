#pragma once

#include "Engine/Renderer/Renderer.hpp"
#include "../GpuContextVulkan.hpp"
#include "../BufferVulkan.hpp"

namespace Brisk {
	class RendererVulkan : public Renderer {
	public:
		virtual void Create() override;
		virtual void Release() override;

		void CreateGraphicsPipeline();
		virtual void SetupRenderingPipeline(Swapchain* swapchain) override;

		virtual void PreRender() override;
		virtual void Render() override;
		virtual void PostRender() override;
	private:
		RendererVulkan() = default;
	private:
		uint32_t m_ImageIndex;

		GpuContextVulkan* m_GpuContext;
		RenderPassVulkan* m_RenderPass;
		GraphicsPipelineVulkan* m_Pipeline;
		SwapchainVulkan* m_Swapchain;
		BufferVulkan* m_VertexBuffer;

		CommandBufferVulkan* m_CommandBuffer;

		VkCommandPool m_CommandPool;
		VkFence m_InFlightFence;
		VkSemaphore m_RenderFinishedSemaphore;
		VkSemaphore m_ImageAvailableSemaphore;

		friend class RendererFactory;
	};
}