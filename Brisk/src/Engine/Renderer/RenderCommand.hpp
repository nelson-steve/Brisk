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
	public:
		static RendererAPI* s_RendererAPI;
	};
}