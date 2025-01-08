#pragma once

#include "Engine/Renderer/RendererAPI.hpp"
#include "CommandBufferVulkan.hpp"

#include <Volk/volk.h>
#include <vector>
#include <Engine/Engine.hpp>

namespace Brisk
{
	class RendererAPIVulkan : public RendererAPI {
		virtual void SetViewport(unsigned int x, unsigned int y, unsigned int width, unsigned int height) override {
		}

		virtual void Draw(std::shared_ptr<CommandBuffer> cmd, uint32_t vertexCount, uint32_t firstVertex) override {
			vkCmdDraw(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), vertexCount, 1, 0, 0);
		}

		virtual void DrawIndexed(std::shared_ptr<CommandBuffer> cmd, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) override {
			vkCmdDrawIndexed(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
		}
	};
}