#pragma once

#include "Semaphore.hpp"
#include "Graphics/Swapchain.hpp"
#include "CommandBuffer.hpp"

#include <vector>
#include <memory>

namespace Brisk 
{
	class Queue {
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
			int imageIndex;
		};		

	public:
		void Submit(SubmitInfo submitInfo, std::shared_ptr<Fence> fence);
		void Present(Queue::PresentInfo info);

		static std::shared_ptr<Queue> Create();
	};
}