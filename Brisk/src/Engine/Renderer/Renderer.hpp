#pragma once

#include "Pipeline.hpp"
#include "CommandBuffer.hpp"
#include "Graphics/Swapchain.hpp"
#include "Buffer.hpp"
#include "Engine/Scene.hpp"

#include <memory>

namespace Brisk 
{
	class BufferVulkan;

	class Renderer {
	public:
		void Init(const std::shared_ptr<Scene> scene);
		void RenderScene(float deltaTime);

		static std::unique_ptr<Renderer> Create();
	private:
		std::shared_ptr<Swapchain> m_Swapchain;
		std::shared_ptr<Buffer> m_VertexBuffer;
		std::shared_ptr<Buffer> m_UniformBuffer;
	};
}