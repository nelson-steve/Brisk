#pragma once

#include "CommandBuffer.hpp"

namespace Brisk 
{
	class CommandBufferAllocator {
	public:
		virtual void Allocate(std::shared_ptr<CommandBuffer> cmd) = 0;

		static std::shared_ptr<CommandBufferAllocator> Create();
	};
}