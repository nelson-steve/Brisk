#pragma once

// INCLUDES
#include "PipelineVulkan.hpp"
#include "Engine/Engine.hpp"
#include "Engine/Renderer/Buffer.hpp"
//--------------------
#include <Volk/volk.h>
//

namespace Brisk {
	struct Vertex;
	class BufferVulkan : public Buffer {
	public:
		virtual void Init(uint32_t size,
			void* data,
			std::vector<BufferUsage> usageFlags,
			std::vector<MemoryProperty> memoryProperty,
			bool mapPersistant) override;
		virtual void Release() override;
	private:
		void Create(uint32_t size, VkBufferUsageFlags usageFlags);
		void Allocate(VkMemoryPropertyFlags properties);
		void MapMemory(std::vector<Point>& data);
		void MapMemory(Vertex* data);
		void MapMemory(void** data);
		void UnMapMemory();
		void Release();

		inline const VkBuffer& Get() const { 
			return m_Handle; 
		}
		const VkDeviceMemory GetMemory() const { return m_Memory; }
	private:
		void* m_Data;
		VkBuffer m_Handle;
		VkDeviceMemory m_Memory;
	};
}