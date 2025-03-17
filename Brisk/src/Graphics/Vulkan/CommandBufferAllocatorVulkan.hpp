#pragma once

#include "Engine/Renderer/CommandBufferAllocator.hpp"

namespace Brisk 
{
	class CommandBufferAllocatorVulkan : public CommandBufferAllocator {
		virtual void Allocate(std::shared_ptr<CommandBuffer> cmd) override;
	};
}