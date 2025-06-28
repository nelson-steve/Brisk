#pragma once

// INCLUDES
#include "Engine/Engine.hpp"
#include "PipelineVulkan.hpp"
#include "Engine/Renderer/Buffer.hpp"
#include "Engine/Camera.hpp"
//-----------------------------------
#include <vk_mem_alloc.h>
#include <volk.h>
//--------------------

namespace Brisk {
	class BufferVulkan : public Buffer {
	public:
		virtual void Init(uint32_t size, void* data, Core::BufferUsage usageFlags, Core::MemoryProperty memoryProperty, bool mapPersistant) override;
		virtual void Release() override;
		virtual void UpdatePersistantData(uint32_t size, void* data) override;

		inline const VkBuffer Get() const {
			return m_Handle;
		}

		inline const VkDeviceMemory GetMemory() const {
			return m_Memory;
		}

		const VkDescriptorBufferInfo* GetDescriptor() {
			bufferInfo.buffer = m_Handle;
			bufferInfo.offset = 0;
			bufferInfo.range = VK_WHOLE_SIZE;
			return &bufferInfo;
		}
	private:
		VmaAllocation m_Allocation;
		VkBuffer m_Handle;
		VkDeviceMemory m_Memory;
		VkDescriptorBufferInfo bufferInfo;

		bool isMapped = false;
		void* mappedPtr = nullptr;
	};
}