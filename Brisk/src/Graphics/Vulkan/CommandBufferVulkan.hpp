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

		virtual void SetParentAllocator(std::shared_ptr<CommandBufferAllocator> allocator) override;

		void Allocate(VkCommandPool pool);
		void Reset() override;

		const VkCommandBuffer& Get() { return m_CommandBuffer; }
	private:
		VkCommandPool m_ParentPool;
		VkCommandBuffer m_CommandBuffer;
	};
}