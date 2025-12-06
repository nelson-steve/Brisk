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
			VkDescriptorType p_Type;
			VkShaderStageFlags p_Stages;
		};
	public:
		virtual void Init(const GraphicsPipelineSpecs& specs) override;
		virtual void Init(const ComputePipelineSpecs& specs) override;
		virtual void Init(const RayTracingPipelineSpecs& specs) override;
		virtual void Release() override;

		void HotReload();
		virtual void UpdateResources(const std::string& name, std::vector<std::shared_ptr<Texture>> textures, std::shared_ptr<Buffer> buffer, std::shared_ptr<TLAS> tlas) override;
		virtual void Bind(std::shared_ptr<CommandBuffer> cmd) override;
		virtual void BindPushConstant(std::shared_ptr<CommandBuffer> cmd, uint32_t size, void* data, uint32_t offset, Core::ShaderStageFlags stages) override;

		const VkPipeline GetPipeline() const { return m_Pipeline; }
		const VkPipelineLayout GetLayout() const { return m_PipelineLayout; }

	private:
		void BindInternal(std::shared_ptr<CommandBuffer> cmd, VkDescriptorSet set, uint32_t setIndex);
	private:
		VkPipelineBindPoint bindPoint;
		VkPipelineLayout m_PipelineLayout;
		VkPipeline m_Pipeline;

		std::vector<ShaderResource> m_ShaderResources;
		std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;
	};
}