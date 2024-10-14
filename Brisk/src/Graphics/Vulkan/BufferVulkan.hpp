#pragma once

#include "PipelineVulkan.hpp"
#include "Engine/Engine.hpp"

#include <Volk/volk.h>

namespace Brisk {
	struct Vertex;
	class BufferVulkan {
	public:
		void Create(uint32_t size, VkBufferUsageFlags usageFlags);
		void Allocate(VkMemoryPropertyFlags properties);
		void MapMemory(std::vector<Point>& data);
		void MapMemory(Vertex* data);
		void MapMemory(void** data);
		void UnMapMemory();
		void Release();

		uint64_t GetSize() { 
			return m_Size; 
		}
		const VkBuffer& Get() const { 
			return m_Handle; 
		}
		const VkDeviceMemory GetMemory() const { return m_Memory; }
	private:
		uint64_t m_Size;
		VkBuffer m_Handle;
		VkDeviceMemory m_Memory;
	};
}