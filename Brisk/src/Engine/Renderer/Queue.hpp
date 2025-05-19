#pragma once

// INCLUDES
#include "Swapchain.hpp"
#include "CommandBuffer.hpp"
#include "Semaphore.hpp"
#include "Fence.hpp"
//-----------------
#include <vector>
#include <memory>
//---------------

namespace Brisk 
{
	class Queue {
		DEFINE_BASE_CLASS_CONSTRUCTOR(Queue)
	public:
		enum class QueueType {
			Graphics,
			Transfer
		};
	public:
		enum WaitStage {
			PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		};

		struct SubmitInfo {
			std::vector<std::shared_ptr<Semaphore>> pWaitSemaphores;
			std::vector<std::shared_ptr<Semaphore>> pSignalSemaphores;
			std::vector<std::shared_ptr<CommandBuffer>> pCmdBuffers;
			std::vector<WaitStage> pWaitStages;
		};

		struct PresentInfo {
			std::vector<std::shared_ptr<Semaphore>> pWaitSemaphores;
			std::vector<std::shared_ptr<Swapchain>> pSwapchains;
			int pImageIndex;
		};

	public:
		virtual void Submit(SubmitInfo submitInfo, std::shared_ptr<Fence> fence) = 0;
		virtual void Present(Queue::PresentInfo info) = 0;

		virtual void Init(Queue::QueueType type) = 0;

		static std::shared_ptr<Queue> Create();
	};
}