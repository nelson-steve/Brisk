// INCLUDES
#include "BufferVulkan.hpp"
#include "Engine/Model.hpp"
#include "Engine/Engine.hpp"
#include "UtilitiesVulkan.hpp"
#include "GpuAdapterVulkan.hpp"
#include "Engine/Application.hpp"
//-------------------------------
#include <memory>
//---------------
namespace Brisk 
{
	void BufferVulkan::Init(uint64_t size,
		void* data,
		std::vector<Core::BufferUsage> usageFlags,
		std::vector<Core::MemoryProperty> memoryProperty,
		bool mapPersistant) {
		VkBufferUsageFlags usage{};
		VmaMemoryUsage vmaUsage = VMA_MEMORY_USAGE_CPU_TO_GPU; // slow, usage staging buffer

		for (auto& flag : usageFlags)
			usage |= UtilitiesVulkan::BufferUsageToVkFormat(flag);

		Create(size, usage, vmaUsage);

		if (mapPersistant) {
			void* ref;
			vmaMapMemory(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetVmaAllocator(), m_Allocation, &ref);
			memcpy(ref, data, static_cast<size_t>(m_Size));
			vmaUnmapMemory(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetVmaAllocator(), m_Allocation);
		}
		else {
			void* ref;
			vmaMapMemory(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetVmaAllocator(), m_Allocation, &ref);
			m_MappedPointerHandle = ref;
			if (data)
				memcpy(m_MappedPointerHandle, data, static_cast<size_t>(m_Size));
		}
	}

	void BufferVulkan::UpdatePersistantData(uint32_t size, void* data) {
		memcpy(m_MappedPointerHandle, data, m_Size);
	}

	void BufferVulkan::Create(uint64_t bufferSize, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage) {
		m_Size = bufferSize;

		VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferInfo.size = m_Size;
		bufferInfo.usage = usageFlags;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = memoryUsage;

		if (vmaCreateBuffer(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetVmaAllocator(), &bufferInfo, &allocCreateInfo, &m_Handle, &m_Allocation, nullptr) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create VMA buffer");
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

	void BufferVulkan::MapMemory(MeshData* vertices) {
		void* data;
		vmaMapMemory(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetVmaAllocator(), m_Allocation, &data);
		memcpy(data, vertices, static_cast<size_t>(m_Size));
		vmaUnmapMemory(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetVmaAllocator(), m_Allocation);
	}

	void BufferVulkan::MapMemory(void** data) {
		void* mapped;
		vmaMapMemory(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetVmaAllocator(), m_Allocation, &mapped);
		*data = mapped;
	}

	void BufferVulkan::UnMapMemory() {
		vmaUnmapMemory(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetVmaAllocator(), m_Allocation);
	}

	void BufferVulkan::Release() {
		vmaDestroyBuffer(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetVmaAllocator(), m_Handle, m_Allocation);
	}
}