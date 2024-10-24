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
			std::vector<Core::BufferUsage> usageFlags,
			std::vector<Core::MemoryProperty> memoryProperty,
			bool mapPersistant) override;
		virtual void Release() override;

		void UpdatePersistantData(uint32_t size, void* data);

		inline const VkBuffer& Get() const {
			return m_Handle;
		}
	private:
		void Create(uint32_t size, VkBufferUsageFlags usageFlags);
		void Allocate(VkMemoryPropertyFlags properties);
		void MapMemory(std::vector<Point>& data);
		void MapMemory(Vertex* data);
		void MapMemory(void** data);
		void UnMapMemory();
		void Release();
		const VkDeviceMemory GetMemory() const { return m_Memory; }
	private:
		void* m_PersistantPtr;
		VkBuffer m_Handle;
		VkDeviceMemory m_Memory;
	};
}