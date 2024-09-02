#pragma once

#include <Volk/volk.h>
#include "GraphicsPipelineVulkan.hpp"
#include "Engine/Engine.hpp"

namespace Brisk {
	class BufferVulkan {
	public:
		void Create(uint32_t, VkBufferUsageFlags usageFlags);
		void Allocate(VkMemoryPropertyFlags properties);
		void MapMemory(std::vector<Point>& data);
		void* MapMemory();
		void UnMapMemory();
		void Release();

		//const std::vector<Point>& GetData() { return m_Data; }
		const VkBuffer& Get() const { return m_Handle; }
		const VkDeviceMemory GetMemory() const { return m_Memory; }
	private:
		uint64_t m_Size;
		VkBuffer m_Handle;
		VkDeviceMemory m_Memory;
	};
}