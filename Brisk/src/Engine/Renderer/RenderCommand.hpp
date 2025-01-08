#pragma once
#include <memory>
#include "RendererAPI.hpp"

namespace Brisk 
{
	class RenderCommand {
	public:
		inline static void SetViewport() {
			//s_RendererAPI->SetViewport();
		}
		inline static void SetScissor() {
			//s_RendererAPI->SetScrissor();
		}
		inline static void Draw(std::shared_ptr<CommandBuffer> cmd, uint32_t vertexCount, uint32_t firstVertex){
			s_RendererAPI->Draw(cmd, vertexCount, firstVertex);
		}
		inline static void DrawIndexed(std::shared_ptr<CommandBuffer> cmd, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance){
			s_RendererAPI->DrawIndexed(cmd, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
		}
	public:
		static RendererAPI* s_RendererAPI;
	};
}