#pragma once

// INCLUDES
#include "Queue.hpp"
#include "Buffer.hpp"
#include "Pipeline.hpp"
#include "Swapchain.hpp"
#include "Engine/Scene.hpp"
#include "Engine/Model.hpp"
#include "CommandBuffer.hpp"
#include "RenderCommand.hpp"
#include "RenderGraph.hpp"
#include "Engine/Renderer/CommandBufferAllocator.hpp"
//------------------------
#include <memory>
//---------------

namespace Brisk 
{
	class Renderer {
	public:
		void Init();
		void PreRenderScene();
		void RenderScene(float deltaTime);
		void DrawNode(const std::shared_ptr<Mesh> model, std::vector<std::shared_ptr<Shader>> materials, GLTF_Node* node);

		static std::shared_ptr<Swapchain> GetSwapchain() { return m_Swapchain; }

		static std::unique_ptr<Renderer> Create();
	private:
		void RenderEntity(Entity e);
		void SetupEntity(Entity e);

	private:
		static std::shared_ptr<Swapchain> m_Swapchain;

		std::shared_ptr<RenderGraph> m_RenderGraph;

		// Synchronization objects
		std::shared_ptr<Semaphore> ImageAvailableSemaphore;
		std::shared_ptr<Semaphore> RenderFinishedSemaphore;
		std::shared_ptr<Fence> m_Fence;
		std::shared_ptr<Queue> m_Queue;
		//-

		std::shared_ptr<CommandBufferAllocator> m_MainCmdBufferAllocator;
		std::shared_ptr<Pipeline> m_Pipeline;
		std::shared_ptr<Buffer> m_UniformBuffer;
		std::shared_ptr<CommandBuffer> m_MainCmdBuffer;
		RenderCommand m_RenderCommand;
		uint32_t m_ImageIndex;
	};
}