#pragma once

// INCLUDES
#include "RHI.hpp"
#include "CommandBuffer.hpp"
//----------------
#include <cstdint>
#include <vector>
#include <memory>
//---------------

namespace Brisk
{
	class Buffer {
	public:
		virtual void Init(uint32_t size, void* data, Core::BufferUsage usageFlags, Core::MemoryProperty memoryProperty, bool mapPersistant) = 0;
		virtual void Release() = 0;
		virtual void UpdatePersistantData(uint32_t size, void* data) = 0;
		
		virtual void MemoryPipelineBarrier(std::shared_ptr<CommandBuffer> cmd) = 0;

		static std::shared_ptr<Buffer> Create();
	protected:
		inline uint64_t GetSize() const { 
			return m_Size; 
		}
	protected:
		uint64_t m_Size;
	};
}