#pragma once

#include "Engine/Renderer/Pipeline.hpp"

#include <Volk/volk.h>
#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace Brisk
{
	struct PushConstants {
		uint32_t albedoIndex;
		uint32_t metallicRoughnessIndex;
		uint32_t normalIndex;
		uint32_t emissiveIndex;
		uint32_t occlusionIndex;
	};

	class PipelineVulkan : public Pipeline {
	public:
		virtual void Init(const GraphicsPipelineSpecs& specs) = 0;
		virtual void Init(const ComputePipelineSpecs& specs) = 0;

		virtual void Bind(std::shared_ptr<CommandBuffer> cmd) override;

		virtual void BindPushConstant(std::shared_ptr<CommandBuffer> cmd, uint32_t size, void* data) override;

		void Destroy();

		const VkPipeline GetPipeline() const { return m_Pipeline; }
		const VkPipelineLayout GetLayout() const { return m_PipelineLayout; }
	private:
		std::vector<VkShaderModule> m_Modules;
		VkPipelineLayout m_PipelineLayout;
		VkPipeline m_Pipeline;
	};
}