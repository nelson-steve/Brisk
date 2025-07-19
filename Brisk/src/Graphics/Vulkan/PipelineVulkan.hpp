#pragma once

#include "Engine/Renderer/Pipeline.hpp"

#include <volk.h>
#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace Brisk
{
	/*struct ShaderResource {
		std::string p_Name;
		uint32_t p_Set;
		uint32_t p_Binding;
		VkDescriptorType p_Type;
		VkShaderStageFlags p_Stages;
	};*/

	class PipelineVulkan : public Pipeline {
	public:
		virtual void Init(const GraphicsPipelineSpecs& specs) override;
		virtual void Init(const ComputePipelineSpecs& specs) override;

		virtual void Bind(std::shared_ptr<CommandBuffer> cmd) override;

		virtual void BindPushConstant(std::shared_ptr<CommandBuffer> cmd, uint32_t size, void* data, uint32_t offset, bool vertexShader) override;

		virtual void Release() override;

		const VkPipeline GetPipeline() const { return m_Pipeline; }
		const VkPipelineLayout GetLayout() const { return m_PipelineLayout; }
	private:
		std::vector<VkShaderModule> m_Modules;
		VkPipelineLayout m_PipelineLayout;
		VkPipeline m_Pipeline;

		std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;
	};
}