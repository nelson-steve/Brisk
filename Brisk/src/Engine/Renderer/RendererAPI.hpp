#pragma once

// INCLUDES
#include "CommandBuffer.hpp"
#include "Buffer.hpp"
//------------------
#include <memory>
#include <vector>
//---------------

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
		virtual void SetViewport(std::shared_ptr<CommandBuffer> cmd, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t minDepth, uint32_t maxDepth) = 0;
		virtual void SetScissor(std::shared_ptr<CommandBuffer> cmd, uint32_t offsetX, uint32_t offsetY, uint32_t extentX, uint32_t extentY) = 0;
		virtual void Draw(std::shared_ptr<CommandBuffer> cmd, uint32_t vertexCount, uint32_t firstVertex) = 0;
		virtual void DrawIndexed(std::shared_ptr<CommandBuffer> cmd, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) = 0;
		virtual void BindIndexBuffer(std::shared_ptr<CommandBuffer> cmd, std::shared_ptr<Buffer> buffer, uint32_t firstBinding) = 0;
		virtual void BindVertexBuffer(std::shared_ptr<CommandBuffer> cmd, std::vector<std::shared_ptr<Buffer>> buffers, uint32_t firstBinding) = 0;

		//inline static API GetAPI() { return s_API; }
		static RendererAPI* Create();
	private:
		//static API s_API;
	};

	class ComputeAPI
	{
	public:
		enum class API
		{
			None = 0, Vulkan = 1
		};
	public:
		virtual ~ComputeAPI() = default;
		virtual void CmdDispatch(std::shared_ptr<CommandBuffer> cmd, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;

		//inline static API GetAPI() { return s_API; }
		static ComputeAPI* Create();
	private:
		//static API s_API;
	};
}