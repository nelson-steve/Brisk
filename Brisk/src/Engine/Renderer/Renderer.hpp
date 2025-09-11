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
#include "Engine/Component.hpp"
#include "CSMRenderPass.hpp"
//------------------------
#include <memory>
//---------------

#define MAX_LIGHTS 10000
#define MAX_LIGHTS_PER_CLUSTER 128
#define NUM_CLUSTERS 16 * 9 * 24

namespace Brisk 
{
	struct Probe {
		glm::vec3 Position;
	};

	struct PointLight {
		glm::vec4 position; // xyz = pos, w = radius
		glm::vec4 color;    // xyz = color, w = intensity
	};

	struct LightOffset {
		alignas(8) glm::uvec2 Offset;
	};

	struct TileAABB {
		alignas(16) glm::vec4 minPoint;
		alignas(16) glm::vec4 maxPoint;
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

	struct MVP {
		alignas(16) glm::mat4 ProjView;
		alignas(16) glm::mat4 View;
		alignas(16) glm::vec3 CamPos;
	};

	struct ShadowData {
		alignas(16) glm::mat4 lightSpaceMatrices[4];
		alignas(16) glm::vec4 cascadeSplits;;
	};

	class Renderer {
	public:
		void Init();
		void Release();
		void RenderScene(float deltaTime);

		void AddGlobalTexture(std::vector<std::shared_ptr<Texture>> textures) {
			m_GlobalTextures.insert(m_GlobalTextures.end(), textures.begin(), textures.end());
			m_GBufferPipeline->UpdateResources("GlobalTextures", textures, nullptr);
		}

		static std::shared_ptr<Swapchain> GetSwapchain() { return m_Swapchain; }

		static std::unique_ptr<Renderer> Create();
	public:
		static std::shared_ptr<Swapchain> m_Swapchain;

		// Synchronization objects
		std::shared_ptr<Semaphore> AABBGenerateSemaphore;
		std::shared_ptr<Semaphore> AssignLightsSemaphore;
		std::shared_ptr<Semaphore> ImageAvailableSemaphore;
		std::shared_ptr<Semaphore> RenderFinishedSemaphore;
		std::shared_ptr<Semaphore> VoxelizationFinishedSemaphore;

		std::shared_ptr<Fence> m_ClusterFence;
		std::shared_ptr<Fence> m_GraphicsFence;

		std::shared_ptr<Queue> m_GraphicsQueue0;
		std::shared_ptr<Queue> m_GraphicsQueue1;

		std::shared_ptr<Queue> m_ComputeQueue0;
		std::shared_ptr<Queue> m_ComputeQueue1;

		// Attachments
		std::shared_ptr<Texture> m_Pos;
		std::shared_ptr<Texture> m_Normal;
		std::shared_ptr<Texture> m_Albedo;
		std::shared_ptr<Texture> m_Material;
		std::shared_ptr<Texture> m_Emissive;
		std::shared_ptr<Texture> m_DepthPre;
		std::shared_ptr<Texture> m_ShadowMapLOD0;
		std::shared_ptr<Texture> m_ShadowMapLOD1;
		std::shared_ptr<Texture> m_ShadowMapLOD2;
		std::shared_ptr<Texture> m_ShadowMapLOD3;
		std::shared_ptr<Texture> m_LightingOutput;

		std::shared_ptr<Texture> m_IrradiannceImage;

		uint64_t m_ImGuiIdScene;
		uint64_t m_ImGuiIdShadowMap0;
		uint64_t m_ImGuiIdShadowMap1;
		uint64_t m_ImGuiIdShadowMap2;
		uint64_t m_ImGuiIdShadowMap3;

		// RenderPasses
		std::shared_ptr<RenderPass> m_DepthPrePass;
		std::shared_ptr<CSMRenderPass> m_CSMShadowMapPass;
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
		std::shared_ptr<Buffer> m_ShadowDataBuffer;

		std::shared_ptr<Buffer> m_ClusterInfoUBO;
		std::shared_ptr<Buffer> m_ClusterTilesSSBO;
		std::shared_ptr<Buffer> m_GlobalIndexCountSSBO;

		std::shared_ptr<Buffer> m_LightsList;
		std::shared_ptr<Buffer> m_ClusterLightIndexList;
		std::shared_ptr<Buffer> m_ClusterLightOffsetList;
		
		std::shared_ptr<Buffer> m_ProbesBuffer;

		std::shared_ptr<Buffer> m_AtomicCounters;

		std::vector<glm::mat4> m_SunMatrices;
		std::vector<Probe> m_Probes;

		std::shared_ptr<CommandBuffer> m_CmdBuffer;
		std::shared_ptr<CommandBuffer> m_ClusteredCmdBuffer;
		RenderCommand m_RenderCommand;
		uint32_t m_ImageIndex;

		std::unordered_map<MeshAsset*, std::vector<Entity>> m_RenderGroups;

		std::vector<std::shared_ptr<Texture>> m_GlobalTextures;
	};
}