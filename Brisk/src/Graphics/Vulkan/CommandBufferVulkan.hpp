#pragma once
// INCLUDES
#include "Engine/Renderer/CommandBuffer.hpp"
//------------------------------------------
#include <Volk/volk.h>
#include <Engine/Renderer/CommandBufferAllocator.hpp>
//--------------------

namespace Brisk 
{
	class CommandBufferVulkan : public CommandBuffer {
	public:
		virtual void Bind() override;
		virtual void UnBind() override;

		void SetParentAllocator(VkCommandPool allocator);

		void Allocate(VkCommandPool pool);
		void Reset() override;

		VkCommandBuffer& Get() { return m_CommandBuffer; }
	private:
		VkCommandPool m_ParentPool;
		VkCommandBuffer m_CommandBuffer;
	};
}