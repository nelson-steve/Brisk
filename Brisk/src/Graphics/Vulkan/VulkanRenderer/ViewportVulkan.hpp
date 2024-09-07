#pragma once

#include "../GpuContextVulkan.hpp"

#include <Volk/volk.h>

#include <vector>

namespace Brisk {
	class ViewportVulkan {
	public:
		void Init();
		void Update();
		void Destroy();
	private:
		std::vector<VkImage> m_Images;
		std::vector<VkDeviceMemory> m_ImageMemory;
		std::vector<VkImageView> m_ImageViews;

		RenderPassVulkan* m_RenderPass;
		GraphicsPipelineVulkan* m_Pipeline;
		VkCommandPool m_CommandPool;
		std::vector<CommandBufferVulkan*> m_CommandBuffers;
	};
}