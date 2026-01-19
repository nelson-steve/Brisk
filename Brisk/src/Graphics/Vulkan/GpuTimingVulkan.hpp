#pragma once

#include "Engine/Renderer/GpuTiming.hpp"

#include <Volk/volk.h>

#include <memory>
#include <vector>

namespace Brisk
{
	class GpuTimingVulkan : public GpuTiming {
	public:
		virtual void Init(uint32_t frames) override;

		virtual void Reset(std::shared_ptr<CommandBuffer> cmd, uint32_t index) override;
		virtual void TimeStamp(std::shared_ptr<CommandBuffer> cmd, Core::PipelineStage stage, uint32_t frameIndex, uint32_t index) override;
		virtual float QueryTime(uint32_t frameIndex) override;
		virtual void GetTime(uint32_t beginIndex, uint32_t endIndex, float& currentValue) override;

		std::shared_ptr<GpuTiming> Create();
	private:
		std::vector<VkQueryPool> m_TimestampQueryPools;
		float m_TimeStampPeriod;
		uint64_t m_Results[64];
		uint32_t m_QueryCount = 0;
	};
}