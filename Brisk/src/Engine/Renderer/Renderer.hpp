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
	struct alignas(16) LightsMVP {
		glm::mat4 ViewProjection;
		glm::mat4 Model;
	};

	class Renderer {
	public:
		void Init();
		void Release();
		void RenderScene(float deltaTime);

		static std::shared_ptr<Swapchain> GetSwapchain() { return m_Swapchain; }

		static std::unique_ptr<Renderer> Create();
	private:
		void RenderEntity(const MeshComponent& mesh, int alphaMode, bool push = false);
		void Render(bool pushMaterialIndex = false, bool pushModelMatrix = false);

	private:
		static std::shared_ptr<Swapchain> m_Swapchain;

		// Synchronization objects
		std::shared_ptr<Semaphore> ImageAvailableSemaphore;
		std::shared_ptr<Semaphore> RenderFinishedSemaphore;
		std::shared_ptr<Fence> m_Fence;
		std::shared_ptr<Queue> m_GraphicsQueue;

		// Attachments
		std::shared_ptr<Texture> m_Pos;
		std::shared_ptr<Texture> m_Normal;
		std::shared_ptr<Texture> m_Albedo;
		std::shared_ptr<Texture> m_Material;
		std::shared_ptr<Texture> m_Emissive;
		std::shared_ptr<Texture> m_DepthPre;
		//std::shared_ptr<Texture> m_Depth;
		std::shared_ptr<Texture> m_ShadowMap;
		std::shared_ptr<Texture> m_LightingOutput;

		std::shared_ptr<Editor> m_Editor;

		// RenderPasses
		std::shared_ptr<RenderPass> m_DepthPrePass;
		std::shared_ptr<RenderPass> m_ShadowMapPass;
		std::shared_ptr<RenderPass> m_GeometryBufferPass;
		std::shared_ptr<RenderPass> m_LightingPass;
		std::shared_ptr<RenderPass> m_UIPass;

		// Pipelines
		std::shared_ptr<Pipeline> m_DepthPrePassPipeline;
		std::shared_ptr<Pipeline> m_ShadowMapPipeline;
		std::shared_ptr<Pipeline> m_GBufferPipeline;
		std::shared_ptr<Pipeline> m_GBufferDoubleSidedPipeline;
		std::shared_ptr<Pipeline> m_GBufferAlphaBlendPipeline;
		std::shared_ptr<Pipeline> m_LightingPipeline;

		std::shared_ptr<Buffer> m_LightsUBO;
		std::shared_ptr<CommandBuffer> m_CmdBuffer;
		RenderCommand m_RenderCommand;
		uint32_t m_ImageIndex;

		std::unordered_map<MeshAsset*, std::vector<Entity>> m_RenderGroups;
	};
}