#include "pch.hpp"
#include "GpuTImingVulkan.hpp"
#include "GpuAdapterVulkan.hpp"
#include "CommandBufferVulkan.hpp"
#include "UtilitiesVulkan.hpp"

namespace Brisk
{
	void GpuTimingVulkan::Init(uint32_t frames) {
		VkQueryPoolCreateInfo queryPoolInfo{};
		queryPoolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
		queryPoolInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
		queryPoolInfo.queryCount = 64;
		queryPoolInfo.flags = 0;

		VkQueryPool timestampQueryPool;
		for (int i = 0; i < frames; i++) {
			vkCreateQueryPool(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), &queryPoolInfo, nullptr, &timestampQueryPool);
			m_TimestampQueryPools.push_back(timestampQueryPool);
		}

		VkPhysicalDeviceProperties props{};
		vkGetPhysicalDeviceProperties(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetPhysicalDevice(), &props);

		m_TimeStampPeriod = props.limits.timestampPeriod;
	}

	void GpuTimingVulkan::TimeStamp(std::shared_ptr<CommandBuffer> cmd, Core::PipelineStage stage, uint32_t frameIndex, uint32_t index) {
		VkPipelineStageFlagBits vkStage = VK_PIPELINE_STAGE_NONE_KHR;

		if ((stage & Core::PipelineStage::TopOfPipe) == Core::PipelineStage::TopOfPipe)
			vkStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		else if ((stage & Core::PipelineStage::BottomOfPipe) == Core::PipelineStage::BottomOfPipe)
			vkStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		else if ((stage & Core::PipelineStage::TransferStage) == Core::PipelineStage::TransferStage)
			vkStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		else if ((stage & Core::PipelineStage::ComputeShader) == Core::PipelineStage::ComputeShader)
			vkStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		else if ((stage & Core::PipelineStage::FragmentShader) == Core::PipelineStage::FragmentShader)
			vkStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		else if ((stage & Core::PipelineStage::VertexShader) == Core::PipelineStage::VertexShader)
			vkStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
		else if ((stage & Core::PipelineStage::ColorAttachment) == Core::PipelineStage::ColorAttachment)
			vkStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		else if ((stage & Core::PipelineStage::EarlyFragmentTest) == Core::PipelineStage::EarlyFragmentTest)
			vkStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		else if ((stage & Core::PipelineStage::LateFragmentTest) == Core::PipelineStage::LateFragmentTest)
			vkStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		else if ((stage & Core::PipelineStage::AllGraphics) == Core::PipelineStage::AllGraphics)
			vkStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
		else if ((stage & Core::PipelineStage::AllCommands) == Core::PipelineStage::AllCommands)
			vkStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		else if ((stage & Core::PipelineStage::RayTracing) == Core::PipelineStage::RayTracing)
			vkStage = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;

		vkCmdWriteTimestamp(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), vkStage, m_TimestampQueryPools[frameIndex], index);
	}

	void GpuTimingVulkan::Reset(std::shared_ptr<CommandBuffer> cmd, uint32_t index) {
		vkCmdResetQueryPool(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), m_TimestampQueryPools[index], 0, QUERY_COUNT);
	}

	float GpuTimingVulkan::QueryTime(uint32_t frameIndex) {
		if (vkGetQueryPoolResults(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), m_TimestampQueryPools[frameIndex], 0, QUERY_COUNT, sizeof(uint64_t) * QUERY_COUNT, m_Results, sizeof(m_Results[0]), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create Vulkan framebuffer");
		}
	}

	void GpuTimingVulkan::GetTime(uint32_t beginIndex, uint32_t endIndex, float& currentValue) {
		currentValue = currentValue * 0.95 + ((double(m_Results[endIndex])  * m_TimeStampPeriod * 1e-6) - (double(m_Results[beginIndex])  * m_TimeStampPeriod * 1e-6)) * 0.05;
	}
}