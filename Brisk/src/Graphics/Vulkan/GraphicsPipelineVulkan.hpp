#pragma once

#include <Volk/volk.h>

#include <string>
#include <vector>

namespace Brisk
{
	class GraphicsPipelineVulkan {
	public:
		GraphicsPipelineVulkan();
		void CreatePipeline(VkRenderPass renderpass);
		void Destroy();

		const VkPipeline GetPipeline() const { return m_Pipeline; }
		const VkPipelineLayout GetLayout() const { return m_PipelineLayout; }
	private:
		std::vector<VkShaderModule> m_Modules;
		VkPipelineLayout m_PipelineLayout;
		VkPipeline m_Pipeline;
	};
}