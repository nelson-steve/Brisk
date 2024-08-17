#include "BufferVulkan.hpp"
#include "Engine/Engine.hpp"

namespace Brisk {
	void BufferVulkan::Create(uint64_t size, VkBufferUsageFlags usageFlags) {
		VkBufferCreateInfo createInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		createInfo.pNext = nullptr;
		createInfo.size = size;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.usage = usageFlags;
	}

	void BufferVulkan::Allocate() {
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(Engine::s_PhysicalDevice->GetDevice(), m_Handle, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (vkAllocateMemory(Engine::s_PhysicalDevice->GetDevice(), &allocInfo, nullptr, &m_Memory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}
	}

	uint32_t BufferVulkan::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(Engine::s_PhysicalDevice->GetPhysicalDevice(), &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	void BufferVulkan::Release() {
		vkDestroyBuffer(Engine::s_PhysicalDevice->GetDevice(), m_Handle, nullptr);
		vkFreeMemory(Engine::s_PhysicalDevice->GetDevice(), m_Memory, nullptr);
	}
}