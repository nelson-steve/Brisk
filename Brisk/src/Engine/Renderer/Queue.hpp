#pragma once

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

	public:
		void Submit(SubmitInfo submitInfo, std::shared_ptr<Fence> fence);
		void Present();

		static std::shared_ptr<Queue> Create();
	};
}