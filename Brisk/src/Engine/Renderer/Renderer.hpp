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

#define MAX_LIGHTS 1024
#define MAX_LIGHTS_PER_CLUSTER 128
#define NUM_CLUSTERS 16 * 9 * 24
#define NUM_LIGHTS 1024

namespace Brisk 
{
	struct LightData {
		glm::vec4 position; // xyz = pos, w = radius
		glm::vec4 color;    // xyz = color, w = intensity
	};

	struct LightOffset {
		alignas(8) glm::uvec2 Offset;
	};

	struct alignas(16) TileAABB {
		glm::vec4 minPoint;
		glm::vec4 maxPoint;
		uint32_t count;
		uint32_t _padding[3];
		alignas(16) uint32_t lightIndices[MAX_LIGHTS_PER_CLUSTER];
	};

	struct ClusterInfo {
		alignas(16) glm::mat4 View;
		alignas(16) glm::mat4 InverseProj;
		alignas(16) glm::uvec4 TileSizes;
		alignas(8)  glm::uvec2 ScreenDimensions;
		alignas(4)  float zNear;
		alignas(4)  float zFar;
		alignas(4)  uint32_t numLights;
		alignas(4)  uint32_t _padding0;
	};

	struct alignas(16) LightsMVP {
		glm::mat4 ViewProjection;
		glm::mat4 Model;
	};

	struct alignas(16) MVP {
		glm::mat4 ProjectionView;
		glm::vec3 CamPos;
	};

	class Renderer {
	public:
		void Init();
		void Release();
		void RenderScene(float deltaTime);

		void AddGlobalTexture(std::vector<std::shared_ptr<Texture>> textures) {
			m_GlobalTextures.insert(m_GlobalTextures.end(), textures.begin(), textures.end());
			m_GBufferPipeline->UpdateResources("u_GlobalTextures", textures, nullptr);
		}

		static std::shared_ptr<Swapchain> GetSwapchain() { return m_Swapchain; }

		static std::unique_ptr<Renderer> Create();
	private:
		void RenderEntity(const MeshComponent& mesh, int alphaMode, bool push = false);
		void Render(MeshAsset* mesh, std::vector<Entity> entities, bool pushMaterialIndex = false, bool pushModelMatrix = false);

	private:
		static std::shared_ptr<Swapchain> m_Swapchain;

		// Synchronization objects
		std::shared_ptr<Semaphore> ClusteredTaskSemaphore;
		std::shared_ptr<Semaphore> ImageAvailableSemaphore;
		std::shared_ptr<Semaphore> RenderFinishedSemaphore;
		std::shared_ptr<Fence> m_Fence;
		std::shared_ptr<Queue> m_GraphicsQueue;
		std::shared_ptr<Queue> m_ComputeQueue;

		// Attachments
		std::shared_ptr<Texture> m_Pos;
		std::shared_ptr<Texture> m_Normal;
		std::shared_ptr<Texture> m_Albedo;
		std::shared_ptr<Texture> m_Material;
		std::shared_ptr<Texture> m_Emissive;
		std::shared_ptr<Texture> m_DepthPre;
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

		std::shared_ptr<Pipeline> m_AABBGeneratorPipeline;
		std::shared_ptr<Pipeline> m_AssignLightsToClustersPipeline;

		std::shared_ptr<Buffer> m_MVPBuffer;

		std::shared_ptr<Buffer> m_ClusterInfoUBO;
		std::shared_ptr<Buffer> m_ClusterTilesSSBO;
		std::shared_ptr<Buffer> m_GlobalIndexCountSSBO;

		std::shared_ptr<Buffer> m_CameraData;
		std::shared_ptr<Buffer> m_LightsList;
		std::shared_ptr<Buffer> m_ClusterLightIndexList;
		std::shared_ptr<Buffer> m_ClusterLightOffsetList;
		std::shared_ptr<Buffer> m_AtomicCounters;

		std::shared_ptr<CommandBuffer> m_CmdBuffer;
		std::shared_ptr<CommandBuffer> m_ClusteredCmdBuffer;
		RenderCommand m_RenderCommand;
		uint32_t m_ImageIndex;

		std::unordered_map<MeshAsset*, std::vector<Entity>> m_RenderGroups;

		std::vector<std::shared_ptr<Texture>> m_GlobalTextures;
	};
}