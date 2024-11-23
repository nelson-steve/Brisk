#pragma once

#include "Engine/Renderer/CommandBuffer.hpp"

#include <Volk/volk.h>

namespace Brisk {
	class CommandBufferVulkan : public CommandBuffer {
	public:
		virtual void Bind() override;
		virtual void UnBind() override;

		void Allocate(VkCommandPool pool);
		void Reset() override;
		//void Begin(VkCommandBufferUsageFlags usageFlags = 0);
		//void End();

		const VkCommandBuffer& Get() { return m_CommandBuffer; }
	private:
		VkCommandPool m_ParentPool;
		VkCommandBuffer m_CommandBuffer;
	};
}