#pragma once

// INCLUDES
#include "Engine/Renderer/CommandBufferAllocator.hpp"
//--------------------------------------------------
#include <Volk/volk.h>
//--------------------

namespace Brisk 
{
	class CommandBufferAllocatorVulkan : public CommandBufferAllocator {
	public:
		virtual void Init() override;
		virtual void Allocate(std::shared_ptr<CommandBuffer> cmd) override;

		VkCommandPool GetAllocator() const { return m_CommandPool; }
	private:
		VkCommandPool m_CommandPool;
	};
}