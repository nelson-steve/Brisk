#include "UniformBufferVulkan.hpp"
#include "Engine/Engine.hpp"
#include "VulkanUtilities.hpp"

namespace Brisk {
	void UniformBufferVulkan::Create(uint32_t count) {
		m_Buffers.resize(count);
		m_BuffersMemory.resize(count);
		m_BuffersMapped.resize(count);

		for (size_t i = 0; i < count; i++) {
			VkBufferCreateInfo createInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			createInfo.pNext = nullptr;
			m_BufferSize = sizeof(UBO);
			createInfo.size = m_BufferSize;
			createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

			if (vkCreateBuffer(Engine::s_PhysicalDevice->GetDevice(), &createInfo, nullptr, &m_Buffers[i]) != VK_SUCCESS) {
				//throw std::runtime_error("failed to create vertex buffer!");
			}

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(Engine::s_PhysicalDevice->GetDevice(), m_Buffers[i], &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = VulkanUtilities::FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			if (vkAllocateMemory(Engine::s_PhysicalDevice->GetDevice(), &allocInfo, nullptr, &m_BuffersMemory[i]) != VK_SUCCESS) {
				//throw std::runtime_error("failed to allocate vertex buffer memory!");
			}

			vkBindBufferMemory(Engine::s_PhysicalDevice->GetDevice(), m_Buffers[i], m_BuffersMemory[i], 0);

			vkMapMemory(Engine::s_PhysicalDevice->GetDevice(), m_BuffersMemory[i], 0, m_BufferSize, 0, &m_BuffersMapped[i]);
		}
	}

	void UniformBufferVulkan::Release() {
		for (size_t i = 0; i < m_Buffers.size(); i++) {
			vkDestroyBuffer(Engine::s_PhysicalDevice->GetDevice(), m_Buffers[i], nullptr);
			vkFreeMemory(Engine::s_PhysicalDevice->GetDevice(), m_BuffersMemory[i], nullptr);
		}
	}
}