#pragma once

// INCLUDES
#include "RHI.hpp"
//----------------
#include <cstdint>
#include <vector>
#include <memory>
//---------------

namespace Brisk
{
	class Buffer {
	public:
		virtual void Init(uint64_t size,
			void* data, 
			std::vector<Core::BufferUsage> usageFlags, 
			std::vector<Core::MemoryProperty> memoryProperty, 
			bool mapPersistant) = 0;
		virtual void Release() = 0;
		virtual void UpdatePersistantData(uint32_t size, void* data) = 0;
		

		static std::shared_ptr<Buffer> Create();
	protected:
		inline uint64_t GetSize() const { 
			return m_Size; 
		}
	protected:
		uint64_t m_Size;
	};
}