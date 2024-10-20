#pragma once

#include "Engine/Renderer/CommandBuffer.hpp"

#include <Volk/volk.h>

namespace Brisk {
	class CommandBufferVulkan : public CommandBuffer {
	public:
		void Bind();
		void UnBind();

		void Allocate(VkCommandPool pool);
		//void Begin(VkCommandBufferUsageFlags usageFlags = 0);
		//void End();

		const VkCommandBuffer& Get() { return m_CommandBuffer; }
	private:
		VkCommandPool m_ParentPool;
		VkCommandBuffer m_CommandBuffer;
	};
}