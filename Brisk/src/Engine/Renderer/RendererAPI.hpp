#pragma once

#include "CommandBuffer.hpp"

#include <memory>

namespace Brisk
{
	class RendererAPI
	{
	public:
		enum class API
		{
			None = 0, Vulkan = 1
		};
	public:
		virtual ~RendererAPI() = default;
		virtual void SetViewport(unsigned int x, unsigned int y, unsigned int width, unsigned int height) = 0;
		virtual void Draw(std::shared_ptr<CommandBuffer> cmd, uint32_t vertexCount, uint32_t firstVertex) = 0;
		virtual void DrawIndexed(std::shared_ptr<CommandBuffer> cmd, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) = 0;

		inline static API GetAPI() { return s_API; }
		static RendererAPI* Create();
	private:
		static API s_API;
	};
}