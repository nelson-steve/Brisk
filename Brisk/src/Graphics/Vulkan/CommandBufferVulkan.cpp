// INCLUDES
#include "pch.hpp"
#include "Engine/Engine.hpp"
#include "GpuAdapterVulkan.hpp"
#include "CommandBufferVulkan.hpp"
//--------------------------------
#include "CommandBufferAllocatorVulkan.hpp"
//---------------

namespace Brisk {
	void CommandBufferVulkan::Allocate(PoolType type) {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		switch (type)
		{
			case PoolType::Graphics:
				allocInfo.commandPool = Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetGraphicsCommandPool();
				break;
			case PoolType::Compute:
				allocInfo.commandPool = Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetComputeCommandPool();
				break;
			case PoolType::Transfer:
				allocInfo.commandPool = Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetTransferCommandPool();
				break;
		}
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &allocInfo, &m_CommandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void CommandBufferVulkan::SetParentAllocator(VkCommandPool allocator) {
		m_ParentPool = allocator;
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