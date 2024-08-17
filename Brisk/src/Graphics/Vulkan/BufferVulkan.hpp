#pragma once

#include <Volk/volk.h>

namespace Brisk {
	class BufferVulkan {
	public:
		void Create(uint64_t size, VkBufferUsageFlags usageFlags);
		void Allocate();
		void Release();
	private:
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	private:
		VkBuffer m_Handle;
		VkDeviceMemory m_Memory;
	};
}