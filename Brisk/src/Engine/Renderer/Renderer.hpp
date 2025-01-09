#pragma once

#include "Pipeline.hpp"
#include "CommandBuffer.hpp"
#include "Graphics/Swapchain.hpp"
#include "Buffer.hpp"
#include "Engine/Scene.hpp"
#include "Engine/Model.hpp"

#include <memory>

namespace Brisk 
{
	class BufferVulkan;

	class Renderer {
	public:
		void Init();
		void RenderScene(float deltaTime);
		void DrawNode(const std::shared_ptr<Mesh> model, std::vector<std::shared_ptr<Shader>> materials, GLTF_Node* node);

		static std::shared_ptr<Swapchain> GetSwapchain() { return m_Swapchain; }

		static std::unique_ptr<Renderer> Create();
	private:
		static std::shared_ptr<Swapchain> m_Swapchain;
		std::shared_ptr<Buffer> m_UniformBuffer;
	};
}