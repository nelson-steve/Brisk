#pragma once
// INCLUDES
#include "Core/Core.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Buffer.hpp"
#include "Engine/Renderer/Descriptor.hpp"
#include "Engine/Renderer/Fence.hpp"
#include "Engine/Renderer/Semaphore.hpp"
#include "Engine/Renderer/Swapchain.hpp"
//---------------
#include <memory>
//---------------

namespace Brisk
{
	// Predefined descriptor set values
	constexpr uint32_t SET_FRAME_GLOBAL = 0; // Camera, lights, materials
	constexpr uint32_t SET_BINDLESS_TEXTURES = 1; // Global bindless textures
	constexpr uint32_t SET_PER_MESH = 2; //  Deferred textures
	constexpr uint32_t SET_CLUSTERED_LIGHTING = 3; // Clustered lighting
	//

	class GpuAdapter {
		DEFINE_BASE_CLASS_CONSTRUCTOR(GpuAdapter)
	public:
		enum class QueueType {
			Graphics,
			Compute,
			Transfer
		};
	public:
		struct SubmitInfo {
			std::vector<std::shared_ptr<Semaphore>> pWaitSemaphores;
			std::vector<std::shared_ptr<Semaphore>> pSignalSemaphores;
			std::vector<std::shared_ptr<CommandBuffer>> pCmdBuffers;
			std::vector<Core::PipelineStage> pWaitStages;
		};

		struct PresentInfo {
			std::vector<std::shared_ptr<Semaphore>> pWaitSemaphores;
			std::vector<std::shared_ptr<Swapchain>> pSwapchains;
			int pImageIndex;
		};
		virtual void WaitIdle() = 0;

		virtual void SubmitGraphics(SubmitInfo submitInfo, std::shared_ptr<Fence> fence) = 0;
		virtual void SubmitTransfer(SubmitInfo submitInfo, std::shared_ptr<Fence> fence) = 0;
		virtual void SubmitCompute(SubmitInfo submitInfo, std::shared_ptr<Fence> fence) = 0;

		virtual void Present(PresentInfo info) = 0;

		virtual void Init() = 0;
		virtual void Release() = 0;
		virtual void ReleasePools() = 0;

		static std::shared_ptr<GpuAdapter> Create();
	};
}