#include "BufferVulkan.hpp"
#include "Engine/Engine.hpp"
#include "VulkanUtilities.hpp"
#include "GpuContextVulkan.hpp"

namespace Brisk {
	void BufferVulkan::Create(uint32_t bufferSize, VkBufferUsageFlags usageFlags) {
		VkBufferCreateInfo createInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		createInfo.pNext = nullptr;
		m_Size = bufferSize;
		createInfo.size = m_Size;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.usage = usageFlags;

		if (vkCreateBuffer(GpuContextVulkan::s_GPUDevice->GetDevice(), &createInfo, nullptr, &m_Handle) != VK_SUCCESS) {
			throw std::runtime_error("failed to create vertex buffer!");
		}
	}

	void BufferVulkan::Allocate(VkMemoryPropertyFlags properties) {
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(GpuContextVulkan::s_GPUDevice->GetDevice(), m_Handle, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = VulkanUtilities::FindMemoryType(GpuContextVulkan::s_GPUDevice->GetPhysicalDevice(), memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(GpuContextVulkan::s_GPUDevice->GetDevice(), &allocInfo, nullptr, &m_Memory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}

		vkBindBufferMemory(GpuContextVulkan::s_GPUDevice->GetDevice(), m_Handle, m_Memory, 0);
	}

	void BufferVulkan::MapMemory(std::vector<Point>& vertices) {
		void* data;
		vkMapMemory(GpuContextVulkan::s_GPUDevice->GetDevice(), m_Memory, 0, m_Size, 0, &data);
		memcpy(data, vertices.data(), (size_t)m_Size);
	}

	void BufferVulkan::MapMemory(void* data, VkDeviceSize size) {
		vkMapMemory(GpuContextVulkan::s_GPUDevice->GetDevice(), m_Memory, 0, size, 0, &data);
	}

	void BufferVulkan::UnMapMemory() {
		vkUnmapMemory(GpuContextVulkan::s_GPUDevice->GetDevice(), m_Memory);
	}

	void BufferVulkan::Release() {
		vkDestroyBuffer(GpuContextVulkan::s_GPUDevice->GetDevice(), m_Handle, nullptr);
		vkFreeMemory(GpuContextVulkan::s_GPUDevice->GetDevice(), m_Memory, nullptr);
	}
}