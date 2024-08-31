#pragma once

#include <Volk/volk.h>
#include "GraphicsPipelineVulkan.hpp"
#include "Engine/Engine.hpp"

namespace Brisk {
	class BufferVulkan {
	public:
		void Create(std::vector<Point>& data, VkBufferUsageFlags usageFlags);
		void Allocate();
		void MapMemory(std::vector<Point>& data);
		void Release();

		const std::vector<Point>& GetData() { return m_Data; }
		VkBuffer Get() { return m_Handle; }
	private:
		std::vector<Point> m_Data;
		uint64_t m_Size;
		VkBuffer m_Handle;
		VkDeviceMemory m_Memory;
	};
}