#pragma once

// INCLUDES
#include "RHI.hpp"
//----------------
#include <cstdint>
#include <vector>
#include <memory>
//
namespace Brisk
{
	class Buffer {
	public:
		virtual void Init(uint32_t size, 
			void* data, 
			std::vector<Core::BufferUsage> usageFlags, 
			std::vector<Core::MemoryProperty> memoryProperty, 
			bool mapPersistant) = 0;
		virtual void Release() = 0;

		static std::shared_ptr<Buffer> Create();
	protected:
		inline uint32_t GetSize() const { 
			return m_Size; 
		}
	protected:
		uint32_t m_Size;
	};
}