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
	private:
		void CreateRenderPass();
		VkShaderStageFlagBits BriskTypeToVulkanType(ShaderType type);
	private:
		VkShaderModule vertShader;
		VkShaderModule fragShader;
		VkPipelineLayout m_PipelineLayout;
		VkPipeline m_GraphicsPipeline;
		VkRenderPass m_RenderPass;
	};
}