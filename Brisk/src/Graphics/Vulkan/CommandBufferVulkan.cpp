#include "CommandBufferVulkan.hpp"
#include "Engine/Engine.hpp"
#include "GpuContextVulkan.hpp"

namespace Brisk {
	void CommandBufferVulkan::Allocate(VkCommandPool pool) {
		m_ParentPool = pool;
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_ParentPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(GpuContextVulkan::s_GPUDevice->GetDevice(), &allocInfo, &m_CommandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void CommandBufferVulkan::Begin(VkCommandBufferUsageFlags usageFlags) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = usageFlags;
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(m_CommandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}
	}

	void CommandBufferVulkan::End() {
		vkEndCommandBuffer(m_CommandBuffer);
	}
}