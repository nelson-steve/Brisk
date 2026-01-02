#pragma once

// INCLUDES
#include "Queue.hpp"
#include "Buffer.hpp"
#include "Pipeline.hpp"
#include "Swapchain.hpp"
#include "Engine/Scene.hpp"
#include "CommandBuffer.hpp"
#include "RenderCommand.hpp"
#include "Engine/Renderer/CommandBufferAllocator.hpp"
#include "Engine/Renderer/RenderPass.hpp"
#include "Engine/Component.hpp"
#include "CSMRenderPass.hpp"
//------------------------
#include "BLAS.hpp"
#include "TLAS.hpp"
#include "SBT.hpp"
//---------------

#define MAX_LIGHTS 2048
#define MAX_LIGHTS_PER_CLUSTER 128
#define NUM_CLUSTERS 16 * 9 * 24

#define SIZE_1KB 1024 
#define SIZE_10KB 10240 
#define SIZE_100KB 102400 
#define SIZE_1MB 1048576 
#define SIZE_10MB 10485760 
#define SIZE_100MB 104857600

#define FRAMES_IN_FLIGHT 2

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
		alignas(16) glm::uvec4 ScreenDimensions; // x.y -> screen dimension, z.w -> near far
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

	struct RayTracingProps {
		alignas(16) glm::mat4 ViewInv;
		alignas(16) glm::mat4 ProjInv;
		alignas(16) glm::vec3 LightPos;
		alignas(16) glm::vec3 CamPos;
		alignas(16) glm::uvec2 dimension; // width, height
		uint32_t frame;
		uint32_t _pad;
	};

	struct ShadowData {
		alignas(16) glm::mat4 lightSpaceMatrices[4];
		alignas(16) glm::vec4 cascadeSplits;;
	};

	struct ScratchAllocator {
		uint64_t offset = 0;
		uint64_t size = SIZE_100MB * 10;
		uint64_t alignment = 128;

		uint64_t Allocate(uint64_t bytes) {
			bytes = (bytes + alignment - 1) & ~(alignment - 1);
			if (offset + bytes > size)
				throw std::runtime_error("Scratch buffer overflow!");
			uint64_t allocOffset = offset;
			offset += bytes;
			return allocOffset;
		}

		void Reset() { offset = 0; } // reset per frame

		std::shared_ptr<Buffer> m_ScratchBuffer;
	};


	class Renderer {
	public:
		void Init();
		void Release();
		void RenderScene(float deltaTime);
		void RenderRT(float deltaTime);
		void UpdateTransforms();
		void SetSubmitTransferWork(bool value) { m_SubmitTransferWork = value; }
		void AddGlobalTexture(std::vector<std::shared_ptr<Texture>> textures) {
			m_GBufferPipeline->UpdateResources("GlobalTextures", textures, nullptr, {});
		}
		void RebuildAccelerationStructures();

		static std::shared_ptr<Swapchain> GetSwapchain() { return m_Swapchain; }

		void RecreateSwapchain();

		static std::unique_ptr<Renderer> Create();
	public:
		static std::shared_ptr<Swapchain> m_Swapchain;

		// Synchronization objects
		std::array<std::shared_ptr<Semaphore>, FRAMES_IN_FLIGHT> AABBGenerateSemaphore;
		std::array<std::shared_ptr<Semaphore>, FRAMES_IN_FLIGHT> AssignLightsSemaphore;
		std::array<std::shared_ptr<Semaphore>, FRAMES_IN_FLIGHT> ImageAvailableSemaphore;
		std::array<std::shared_ptr<Semaphore>, FRAMES_IN_FLIGHT> RenderFinishedSemaphore;
		std::array<std::shared_ptr<Semaphore>, FRAMES_IN_FLIGHT> TransferFinishedSemaphore;
		std::array<std::shared_ptr<Fence>, FRAMES_IN_FLIGHT> m_ClusterFence;
		std::array<std::shared_ptr<Fence>, FRAMES_IN_FLIGHT> m_GraphicsFence;

		std::array<std::shared_ptr<Semaphore>, FRAMES_IN_FLIGHT> RayTracingFinishedSemaphore;
		std::array<std::shared_ptr<Fence>, FRAMES_IN_FLIGHT> m_RayTracingFence;

		std::shared_ptr<Queue> m_GraphicsQueue0;
		std::shared_ptr<Queue> m_GraphicsQueue1;

		std::shared_ptr<Queue> m_TransferQueue0;
		std::shared_ptr<Queue> m_TransferQueue1;

		std::shared_ptr<Queue> m_ComputeQueue0;
		std::shared_ptr<Queue> m_ComputeQueue1;
		// Synchronization objects - End

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
		std::shared_ptr<Texture> m_TonemapOutput;
		std::shared_ptr<Texture> m_AccumulationImage;
		//std::shared_ptr<Texture> m_RayTracingOutput;
		// Attachments - End

		// RenderPasses
		std::shared_ptr<RenderPass> m_DepthPrePass;
		std::shared_ptr<CSMRenderPass> m_CSMShadowMapPass;
		std::shared_ptr<RenderPass> m_GeometryBufferPass;
		std::shared_ptr<RenderPass> m_LightingPass;
		std::shared_ptr<RenderPass> m_TonemappingPass;
		std::shared_ptr<RenderPass> m_UIPass;
		std::shared_ptr<RenderPass> m_ClusteredDebugPass;
		// RenderPasses - End

		// Pipelines
		std::shared_ptr<Pipeline> m_DepthPrePassPipeline;
		std::shared_ptr<Pipeline> m_ShadowMapPipeline;
		std::shared_ptr<Pipeline> m_GBufferPipeline;
		std::shared_ptr<Pipeline> m_GBufferDoubleSidedPipeline;
		std::shared_ptr<Pipeline> m_GBufferAlphaBlendPipeline;
		std::shared_ptr<Pipeline> m_LightingPipeline;
		std::shared_ptr<Pipeline> m_TonemappingPipeline;

		std::shared_ptr<Pipeline> m_AABBGeneratorPipeline;
		std::shared_ptr<Pipeline> m_AssignLightsToClustersPipeline;

		std::shared_ptr<Pipeline> m_RayTracing;
		// Pipelines - End

		std::shared_ptr<BLAS> m_BLAS;
		std::shared_ptr<TLAS> m_TLAS;

		// Buffer
		std::shared_ptr<Buffer> m_MVPBuffer;
		std::shared_ptr<Buffer> m_RayTracingPropsBuffer;
		std::shared_ptr<Buffer> m_ShadowDataBuffer;

		std::shared_ptr<Buffer> m_ClusterInfoUBO;
		std::shared_ptr<Buffer> m_ClusterTilesSSBO;
		std::shared_ptr<Buffer> m_GlobalIndexCountSSBO;
		std::shared_ptr<Buffer> m_ClustersVertexBuffer;
		std::shared_ptr<Buffer> m_ClustersIndexBuffer;

		std::shared_ptr<Buffer> m_LightsList;
		std::shared_ptr<Buffer> m_ClusterLightIndexList;
		std::shared_ptr<Buffer> m_ClusterLightOffsetList;

		std::shared_ptr<Buffer> m_AtomicCounters;

		std::shared_ptr<Buffer> m_VertexBuffer;
		std::shared_ptr<Buffer> m_IndexBuffer;
		std::shared_ptr<Buffer> m_DrawsBuffer;
		std::shared_ptr<Buffer> m_MeshesBuffer;
		std::shared_ptr<Buffer> m_MeshletsBuffer;
		std::shared_ptr<Buffer> m_MeshletDataBuffer;
		std::shared_ptr<Buffer> m_MaterialStorageBuffer;
		std::shared_ptr<Buffer> m_TransformsBuffer;
		// Buffer - End

		std::shared_ptr<SBT> m_SBT;

		ScratchAllocator m_ScratchAllocator;

		std::vector<glm::mat4> m_SunMatrices;

		uint64_t m_ImGuiIdScene;
		uint64_t m_ImGuiIdShadowMap0;
		uint64_t m_ImGuiIdShadowMap1;
		uint64_t m_ImGuiIdShadowMap2;
		uint64_t m_ImGuiIdShadowMap3;

		std::array<std::shared_ptr<CommandBuffer>, FRAMES_IN_FLIGHT> m_CmdBuffer;
		std::array<std::shared_ptr<CommandBuffer>, FRAMES_IN_FLIGHT> m_RayTracingCmdBuffer;
		std::shared_ptr<CommandBuffer> m_TransferCmdBuffer;
		std::array<std::shared_ptr<CommandBuffer>, FRAMES_IN_FLIGHT> m_ClusteredCmdBuffer;
		RenderCommand m_RenderCommand;
		uint32_t m_ImageIndex;
		uint32_t m_CurrentFrame = 0;
		bool m_SubmitTransferWork = false;

		std::atomic<bool> m_WindowResized = false;
	};
}