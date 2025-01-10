#pragma once

#include "RendererAPI.hpp"
#include "Buffer.hpp"

#include <memory>

namespace Brisk 
{
	class RenderCommand {
	public:
		inline static void SetViewport(std::shared_ptr<CommandBuffer> cmd, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t minDepth, uint32_t maxDepth) {
			s_RendererAPI->SetViewport(cmd, x, y, width, height, minDepth, maxDepth);
		}
		inline static void SetScissor(std::shared_ptr<CommandBuffer> cmd, uint32_t offsetX, uint32_t offsetY, uint32_t extentX, uint32_t extentY) {
			s_RendererAPI->SetScissor(cmd, offsetX, offsetY, extentX, extentY);
		}
		inline static void Draw(std::shared_ptr<CommandBuffer> cmd, uint32_t vertexCount, uint32_t firstVertex){
			s_RendererAPI->Draw(cmd, vertexCount, firstVertex);
		}
		inline static void DrawIndexed(std::shared_ptr<CommandBuffer> cmd, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance){
			s_RendererAPI->DrawIndexed(cmd, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
		}
		inline static void BindIndexBuffer(std::shared_ptr<CommandBuffer> cmd, std::shared_ptr<Buffer> buffer, uint32_t firstBinding) {
			s_RendererAPI->BindIndexBuffer(cmd, buffer, firstBinding);
		}
		inline static void BindVertexBuffer(std::shared_ptr<CommandBuffer> cmd, std::vector<std::shared_ptr<Buffer>> buffers, uint32_t firstBinding) {
			s_RendererAPI->BindVertexBuffer(cmd, buffers, firstBinding);
		}
	public:
		static RendererAPI* s_RendererAPI;
	};
}