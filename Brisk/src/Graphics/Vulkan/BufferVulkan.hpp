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
		virtual void Init(const BufferDesc& desc) override;
		virtual void Release() override;
		virtual void UpdatePersistantData(uint32_t size, void* data, size_t ptrOffset = 0) override;
		virtual void Update(std::shared_ptr<CommandBuffer> cmd, uint32_t size, void* data, uint32_t dataOffset, std::shared_ptr<Buffer> scaratchBuffer) override;

		virtual void MemoryPipelineBarrier(std::shared_ptr<CommandBuffer> cmd, MemoryBarrierParams barrier) override;

		inline const VkBuffer Get() const { return m_Handle; }

		VkDescriptorBufferInfo* GetDescriptor() {
			bufferInfo.buffer = m_Handle;
			bufferInfo.offset = 0;
			bufferInfo.range = VK_WHOLE_SIZE;
			return &bufferInfo;
		}
	private:
		VmaAllocation m_Allocation;
		VkBuffer m_Handle;
		VkDescriptorBufferInfo bufferInfo;

		bool isMapped = false;
		void* mappedPtr = nullptr;
	};
}