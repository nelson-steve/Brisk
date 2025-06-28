#pragma once

// INCLUDES
#include "Queue.hpp"
#include "Buffer.hpp"
#include "Pipeline.hpp"
#include "Swapchain.hpp"
#include "Engine/Scene.hpp"
#include "CommandBuffer.hpp"
#include "RenderCommand.hpp"
#include "RenderGraph.hpp"
#include "Engine/Renderer/CommandBufferAllocator.hpp"
#include "Engine/Renderer/RenderPass.hpp"
#include "Editor/Editor.hpp"
#include "Engine/Component.hpp"
//------------------------
#include <memory>
//---------------

namespace Brisk 
{
	class Renderer {
	public:
		void Init();
		void RenderScene(float deltaTime);

		static std::shared_ptr<Swapchain> GetSwapchain() { return m_Swapchain; }

		static std::unique_ptr<Renderer> Create();
	private:
		void RenderEntity(const MeshComponent& mesh);

	private:
		static std::shared_ptr<Swapchain> m_Swapchain;

		// Synchronization objects
		std::shared_ptr<Semaphore> ImageAvailableSemaphore;
		std::shared_ptr<Semaphore> RenderFinishedSemaphore;
		std::shared_ptr<Fence> m_Fence;
		std::shared_ptr<Queue> m_GraphicsQueue;
		//-

		// Attachments
		std::shared_ptr<Texture> g_Pos;
		std::shared_ptr<Texture> g_Normal;
		std::shared_ptr<Texture> g_Albedo;
		std::shared_ptr<Texture> g_Material;
		std::shared_ptr<Texture> g_Depth;
		std::shared_ptr<Texture> g_lightingOutput;

		std::shared_ptr<Editor> m_Editor;

		// RenderPasses
		std::shared_ptr<RenderPass> m_DepthPrePass;
		std::shared_ptr<RenderPass> m_GeometryBufferPass;
		std::shared_ptr<RenderPass> m_LightingPass;
		std::shared_ptr<RenderPass> m_UIPass;

		// Pipelines
		std::shared_ptr<Pipeline> m_DepthPrePassPipeline;
		std::shared_ptr<Pipeline> m_GBufferPipeline;
		std::shared_ptr<Pipeline> m_LightingPipeline;

		std::shared_ptr<Buffer> m_UniformBuffer;
		std::shared_ptr<CommandBuffer> m_CmdBuffer;
		RenderCommand m_RenderCommand;
		uint32_t m_ImageIndex;
	};
}