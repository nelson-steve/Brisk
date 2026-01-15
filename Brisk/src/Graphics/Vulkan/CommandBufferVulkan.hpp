#pragma once
// INCLUDES
#include "Engine/Renderer/CommandBuffer.hpp"
#include <Engine/Renderer/CommandBufferAllocator.hpp>
//------------------------------------------
#include <volk.h>
//--------------------

namespace Brisk 
{
	class CommandBufferVulkan : public CommandBuffer {
	public:
		virtual void Bind(bool singleUse = 0) override;
		virtual void UnBind() override;

		void SetParentAllocator(VkCommandPool allocator);
		VkCommandPool GetParentAllocator() { return m_ParentPool; }

		virtual void Allocate(PoolType type) override;
		void Reset() override;

		VkCommandBuffer& Get() { return m_CommandBuffer; }
	private:
		VkCommandPool m_ParentPool;
		VkCommandBuffer m_CommandBuffer;
	};
}