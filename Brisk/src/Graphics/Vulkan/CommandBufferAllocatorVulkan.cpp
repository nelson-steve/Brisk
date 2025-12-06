#include "pch.hpp"
#include "CommandBufferAllocatorVulkan.hpp"
#include "GpuAdapterVulkan.hpp"
#include "CommandBufferVulkan.hpp"

namespace Brisk 
{
	void CommandBufferAllocatorVulkan::Init() {
		VkCommandPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		poolInfo.queueFamilyIndex = Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetGraphicsQueueFamily();
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (vkCreateCommandPool(Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS) {
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

		if (vkAllocateCommandBuffers(Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(),
			&allocInfo, 
			&std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get()) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate command buffer!");
		}
	}
}