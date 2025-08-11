#pragma once

#include "Engine/Renderer/Pipeline.hpp"

#include <volk.h>
#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace Brisk
{
	class PipelineVulkan : public Pipeline {
		struct ShaderResource {
			std::string p_Name;
			uint32_t p_Set;
			uint32_t p_Binding;
			uint32_t p_Type;
			uint32_t p_Stages;
		};
	public:
		virtual void Init(const GraphicsPipelineSpecs& specs) override;
		virtual void Init(const ComputePipelineSpecs& specs) override;
		virtual void Release() override;

		virtual void UpdateResources(const std::string& name, std::vector<std::shared_ptr<Texture>> textures, std::shared_ptr<Buffer> buffer) override;
		virtual void Bind(std::shared_ptr<CommandBuffer> cmd) override;
		virtual void BindPushConstant(std::shared_ptr<CommandBuffer> cmd, uint32_t size, void* data, uint32_t offset, bool vertexShader) override;

		const VkPipeline GetPipeline() const { return m_Pipeline; }
		const VkPipelineLayout GetLayout() const { return m_PipelineLayout; }

	private:
		void BindInternal(std::shared_ptr<CommandBuffer> cmd, VkDescriptorSet set, uint32_t setIndex);
	private:
		bool m_IsCompute = false;
		std::vector<VkShaderModule> m_Modules;
		VkPipelineLayout m_PipelineLayout;
		VkPipeline m_Pipeline;

		std::vector<ShaderResource> m_ShaderResources;
		std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;
	};
}