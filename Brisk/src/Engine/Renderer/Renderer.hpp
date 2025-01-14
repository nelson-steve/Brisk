#pragma once

// INCLUDES
#include "Pipeline.hpp"
#include "CommandBuffer.hpp"
#include "Graphics/Swapchain.hpp"
#include "Buffer.hpp"
#include "Engine/Scene.hpp"
#include "Engine/Model.hpp"
#include "RenderCommand.hpp"
#include "Queue.hpp"
#include "Graphics/Vulkan/CommandBufferVulkan.hpp"
//------------------------
#include <memory>
//---------------

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

		std::shared_ptr<Pipeline> pipeline;
	private:
		static std::shared_ptr<Swapchain> m_Swapchain;
		std::shared_ptr<Buffer> m_UniformBuffer;


		std::shared_ptr<Semaphore> ImageAvailableSemaphore;
		std::shared_ptr<Semaphore> RenderFinishedSemaphore;
		std::shared_ptr<Fence> fence;
		uint32_t imageIndex;
		std::shared_ptr<CommandBuffer> cmd;
		VkCommandPool m_CommandPool;
		RenderCommand command;

		std::shared_ptr<Queue> queue;
	};
}