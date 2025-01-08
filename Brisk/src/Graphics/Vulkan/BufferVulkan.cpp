#include "BufferVulkan.hpp"
#include "Engine/Engine.hpp"
#include "UtilitiesVulkan.hpp"
#include "Engine/Model.hpp"
#include "GpuAdapterVulkan.hpp"
#include "Engine/Application.hpp"

#include <memory>

namespace Brisk 
{
	void BufferVulkan::Init(uint32_t size,
		void* data,
		std::vector<Core::BufferUsage> usageFlags,
		std::vector<Core::MemoryProperty> memoryProperty,
		bool mapPersistant) {
		VkBufferUsageFlags usage;
		VkMemoryPropertyFlags memory;
		for (auto& flag : usageFlags)
			usage |= UtilitiesVulkan::BufferUsageToVkFormat(flag);
		for (auto& flag : memoryProperty)
			memory |= UtilitiesVulkan::MemoryPropertyToVkFormat(flag);

		Create(size, usage);
		Allocate(memory);

		if (mapPersistant) {
			void* ref;
			vkMapMemory(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), m_Memory, 0, m_Size, 0, &ref);
			memcpy(ref, data, (size_t)m_Size);
		}
		else
		{
			void* ref;
			vkMapMemory(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), m_Memory, 0, m_Size, 0, &ref);
			memcpy(ref, m_PersistantPtr, (size_t)m_Size);
			vkUnmapMemory(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), m_Memory);
		}

	}

	void BufferVulkan::UpdatePersistantData(uint32_t size, void* data) {
		memcpy(m_PersistantPtr, data, size);
	}

	void BufferVulkan::Create(uint32_t bufferSize, VkBufferUsageFlags usageFlags) {
		VkBufferCreateInfo createInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		createInfo.pNext = nullptr;
		m_Size = bufferSize;
		createInfo.size = m_Size;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.usage = usageFlags;

		if (vkCreateBuffer(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), &createInfo, nullptr, &m_Handle) != VK_SUCCESS) {
			throw std::runtime_error("failed to create vertex buffer!");
		}
	}

	void BufferVulkan::Allocate(VkMemoryPropertyFlags properties) {
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), m_Handle, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = UtilitiesVulkan::FindMemoryType(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetPhysicalDevice(), memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), &allocInfo, nullptr, &m_Memory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}

		vkBindBufferMemory(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), m_Handle, m_Memory, 0);
	}

	//void BufferVulkan::MapMemory(std::vector<Point>& vertices) {
	//	void* data;
	//	vkMapMemory(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), m_Memory, 0, m_Size, 0, &data);
	//	memcpy(data, vertices.data(), (size_t)m_Size);
	//}

	void BufferVulkan::MapMemory(Vertex* vertices) {
		void* data;
		vkMapMemory(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), m_Memory, 0, m_Size, 0, &data);
		memcpy(data, vertices, (size_t)m_Size);
	}

	void BufferVulkan::MapMemory(void** data) {
		void* buffer;
		vkMapMemory(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), m_Memory, 0, m_Size, 0, &buffer);
		*data = buffer;
	}

	void BufferVulkan::UnMapMemory() {
		vkUnmapMemory(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), m_Memory);
	}

	void BufferVulkan::Release() {
		vkDestroyBuffer(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), m_Handle, nullptr);
		vkFreeMemory(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), m_Memory, nullptr);
	}
}