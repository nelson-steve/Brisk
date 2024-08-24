#include "BufferVulkan.hpp"
#include "Engine/Engine.hpp"
#include "VulkanUtilities.hpp"

namespace Brisk {
	void BufferVulkan::Create(std::vector<Point>& data, VkBufferUsageFlags usageFlags) {
		VkBufferCreateInfo createInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		createInfo.pNext = nullptr;
		m_Size = sizeof(data[0]) * data.size();
		createInfo.size = m_Size;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.usage = usageFlags;
		m_Data = data;

		if (vkCreateBuffer(Engine::s_PhysicalDevice->GetDevice(), &createInfo, nullptr, &m_Handle) != VK_SUCCESS) {
			throw std::runtime_error("failed to create vertex buffer!");
		}
	}

	void BufferVulkan::Allocate() {
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(Engine::s_PhysicalDevice->GetDevice(), m_Handle, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = VulkanUtilities::FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (vkAllocateMemory(Engine::s_PhysicalDevice->GetDevice(), &allocInfo, nullptr, &m_Memory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}

		vkBindBufferMemory(Engine::s_PhysicalDevice->GetDevice(), m_Handle, m_Memory, 0);
	}

	void BufferVulkan::MapMemory(std::vector<Point>& vertices) {
		void* data;
		vkMapMemory(Engine::s_PhysicalDevice->GetDevice(), m_Memory, 0, m_Size, 0, &data);
		memcpy(data, vertices.data(), (size_t)m_Size);
		vkUnmapMemory(Engine::s_PhysicalDevice->GetDevice(), m_Memory);
	}

	void BufferVulkan::Release() {
		vkDestroyBuffer(Engine::s_PhysicalDevice->GetDevice(), m_Handle, nullptr);
		vkFreeMemory(Engine::s_PhysicalDevice->GetDevice(), m_Memory, nullptr);
	}
}