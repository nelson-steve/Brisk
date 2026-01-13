#pragma once

#include "Engine/Renderer/CommandBuffer.hpp"
#include "Engine/Renderer/RHI.hpp"

#include <memory>

namespace Brisk
{
	class GpuTiming {
	public:
		virtual void Init(uint32_t frames) = 0;
		virtual void Reset(std::shared_ptr<CommandBuffer> cmd, uint32_t index) = 0;
		virtual void TimeStamp(std::shared_ptr<CommandBuffer> cmd, Core::PipelineStage stage, uint32_t frameIndex, uint32_t index) = 0;
		virtual float QueryTime(uint32_t frameIndex) = 0;
		virtual void GetTime(uint32_t beginIndex, uint32_t endIndex, float& currentValue) = 0;

		static std::shared_ptr<GpuTiming> Create();
	};
}