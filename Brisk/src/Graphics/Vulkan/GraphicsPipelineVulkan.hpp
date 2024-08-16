#pragma once

#include "Graphics/ShaderManager.hpp"

#include <Volk/volk.h>

#include <string>
#include <vector>

namespace Brisk 
{
	struct Vec3 {
		float X; float Y; float Z;
	};

	struct Vertex {
		Vec3 Position;
		Vec3 Color;
	};

	class GraphicsPipelineVulkan {
	public:
		void Create(std::vector<ShaderModule> modules);
		void Release();

		const VkPipeline GetPipeline() const { return m_GraphicsPipeline; }
		//const VkRenderPass GetRenderPass() const { return m_RenderPass; }
	private:
		//void CreateRenderPass();
		VkShaderStageFlagBits BriskTypeToVulkanType(ShaderType type);
	private:
		std::vector<ShaderModule> m_Modules;
		VkPipelineLayout m_PipelineLayout;
		VkPipeline m_GraphicsPipeline;
		//VkRenderPass m_RenderPass;
	};
}