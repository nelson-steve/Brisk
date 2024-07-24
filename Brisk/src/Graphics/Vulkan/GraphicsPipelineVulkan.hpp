#pragma once

#include "Graphics/ShaderManager.hpp"

#include <Volk/volk.h>

#include <string>
#include <vector>

namespace Brisk 
{
	class GraphicsPipelineVulkan {
	public:
		void Create(std::vector<ShaderModule> modules);
		void Release();

		const VkPipeline GetPipeline() const { return m_GraphicsPipeline; }
		const VkRenderPass GetRenderPass() const { return m_RenderPass; }
	private:
		void CreateRenderPass();
		VkShaderStageFlagBits BriskTypeToVulkanType(ShaderType type);
	private:
		std::vector<ShaderModule> m_Modules;
		VkPipelineLayout m_PipelineLayout;
		VkPipeline m_GraphicsPipeline;
		VkRenderPass m_RenderPass;
	};
}