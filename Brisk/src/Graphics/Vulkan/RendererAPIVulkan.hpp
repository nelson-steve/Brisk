#pragma once

#include "Engine/Renderer/RendererAPI.hpp"
#include "CommandBufferVulkan.hpp"
#include "BufferVulkan.hpp"

#include <volk.h>
#include <vector>
#include <Engine/Engine.hpp>

namespace Brisk
{
	class RendererAPIVulkan : public RendererAPI {
		virtual void SetViewport(std::shared_ptr<CommandBuffer> cmd, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t minDepth, uint32_t maxDepth) override {
			VkViewport viewport{};
			viewport.x = static_cast<float>(x);
			viewport.y = static_cast<float>(y);
			viewport.width = static_cast<float>(width);
			viewport.height = static_cast<float>(height);
			viewport.minDepth = static_cast<float>(minDepth);
			viewport.maxDepth = static_cast<float>(maxDepth);

			vkCmdSetViewport(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), 0, 1, &viewport);
		}

		virtual void SetScissor(std::shared_ptr<CommandBuffer> cmd, uint32_t offsetX, uint32_t offsetY, uint32_t extentX, uint32_t extentY) override {
			VkRect2D scissor{};
			scissor.offset = { 0, 0 };
			scissor.extent.width = extentX;
			scissor.extent.height = extentY;
			vkCmdSetScissor(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), 0, 1, &scissor);
		}

		virtual void Draw(std::shared_ptr<CommandBuffer> cmd, uint32_t vertexCount, uint32_t firstVertex) override {
			vkCmdDraw(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), vertexCount, 1, firstVertex, 0);
		}

		virtual void DrawIndexed(std::shared_ptr<CommandBuffer> cmd, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) override {
			vkCmdDrawIndexed(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
		}

		virtual void BindIndexBuffer(std::shared_ptr<CommandBuffer> cmd, std::shared_ptr<Buffer> buffer, uint32_t firstBinding) override {
			vkCmdBindIndexBuffer(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), std::static_pointer_cast<BufferVulkan>(buffer)->Get(), 0, VK_INDEX_TYPE_UINT32);
		}

		virtual void BindVertexBuffer(std::shared_ptr<CommandBuffer> cmd, std::vector<std::shared_ptr<Buffer>> buffers, uint32_t firstBinding) override {
			std::vector<VkBuffer> vertexBuffers;
			std::vector<VkDeviceSize> offsets;
			for (auto buffer : buffers) {
				vertexBuffers.push_back(std::static_pointer_cast<BufferVulkan>(buffer)->Get());
				offsets.push_back(0);
			}
			vkCmdBindVertexBuffers(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), firstBinding, static_cast<uint32_t>(vertexBuffers.size()), vertexBuffers.data(), offsets.data());
		}
	};
}