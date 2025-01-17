// INCLUDES
#include "Engine/Engine.hpp"
#include "GpuAdapterVulkan.hpp"
#include "CommandBufferVulkan.hpp"
//--------------------------------
#include <memory>
//---------------

namespace Brisk {
	void CommandBufferVulkan::Allocate(VkCommandPool pool) {
		m_ParentPool = pool;
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_ParentPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), &allocInfo, &m_CommandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void CommandBufferVulkan::Reset() {
		vkResetCommandBuffer(m_CommandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
	}

	void CommandBufferVulkan::Bind(/*VkCommandBufferUsageFlags usageFlags*/) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		//beginInfo.flags = usageFlags;
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(m_CommandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}
	}

	void CommandBufferVulkan::UnBind() {
		vkEndCommandBuffer(m_CommandBuffer);
	}
}