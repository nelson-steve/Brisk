#pragma once

#include "Engine/Renderer/Pipeline.hpp"

#include <Volk/volk.h>

#include <string>
#include <vector>

namespace Brisk
{
	class PipelineVulkan : public Pipeline {
	public:
		virtual void Init(const PipelineSpecs& specs) override;

		virtual void Bind(std::shared_ptr<CommandBuffer> cmd) override;

		void Destroy();

		const VkPipeline GetPipeline() const { return m_Pipeline; }
		const VkPipelineLayout GetLayout() const { return m_PipelineLayout; }
	private:
		std::vector<VkShaderModule> m_Modules;
		VkPipelineLayout m_PipelineLayout;
		VkPipeline m_Pipeline;
	};
}