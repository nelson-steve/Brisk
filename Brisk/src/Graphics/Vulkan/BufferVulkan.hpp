#pragma once

// INCLUDES
#include "Engine/Model.hpp"
#include "Engine/Engine.hpp"
#include "PipelineVulkan.hpp"
#include "Engine/Renderer/Buffer.hpp"
#include "Engine/Camera.hpp"
//-----------------------------------
#include <Volk/volk.h>
//--------------------

namespace Brisk {
	class BufferVulkan : public Buffer {
	public:
		virtual void Init(uint64_t size,
			void* data,
			std::vector<Core::BufferUsage> usageFlags,
			std::vector<Core::MemoryProperty> memoryProperty,
			bool mapPersistant) override;
		virtual void Release() override;
		virtual void UpdatePersistantData(uint32_t size, void* data) override;

		inline const VkBuffer& Get() const {
			return m_Handle;
		}
		const VkDescriptorBufferInfo* GetDescriptor() {
			bufferInfo.buffer = m_Handle;
			bufferInfo.offset = 0;
			bufferInfo.range = VK_WHOLE_SIZE;
			return &bufferInfo;
		}
	private:
		void Create(uint64_t size, VkBufferUsageFlags usageFlags);
		void Allocate(VkMemoryPropertyFlags properties);
		//void MapMemory(std::vector<Point>& data);
		void MapMemory(MeshData* data);
		void MapMemory(void** data);
		void UnMapMemory();
		//void Release();
		const VkDeviceMemory GetMemory() const { return m_Memory; }
	private:
		void* m_MappedPointerHandle;
		VkBuffer m_Handle;
		VkDeviceMemory m_Memory;
		VkDescriptorBufferInfo bufferInfo;
	};
}