#include "pch.hpp"
#include "CommandBufferAllocatorVulkan.hpp"
#include "GpuAdapterVulkan.hpp"
#include "CommandBufferVulkan.hpp"

namespace Brisk 
{
	void CommandBufferAllocatorVulkan::Init() {
		VkCommandPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		poolInfo.queueFamilyIndex = std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetGraphicsQueueFamily();
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (vkCreateCommandPool(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create command pool!");
		}
	}

	void CommandBufferAllocatorVulkan::Allocate(std::shared_ptr<CommandBuffer> cmd) {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		std::static_pointer_cast<CommandBufferVulkan>(cmd)->SetParentAllocator(m_CommandPool);

		if (vkAllocateCommandBuffers(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(),
			&allocInfo, 
			&std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get()) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate command buffer!");
		}
	}
}