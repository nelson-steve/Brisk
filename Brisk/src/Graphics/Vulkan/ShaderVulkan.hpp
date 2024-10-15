#pragma once

#include "Engine/Renderer/Descriptor.hpp"
#include "Engine/Renderer/Shader.hpp"

#include <Volk/volk.h>

#include <vector>

namespace Brisk
{
	class ShaderVulkan : public Shader {
	public:
		void Init(std::pair<std::string, Pipeline::ShaderStage>);

		const VkPipelineShaderStageCreateInfo GetShaderStage() const { return m_ShaderStage; }
	private:
		VkShaderModule m_Module;
		VkPipelineShaderStageCreateInfo m_ShaderStage;
	};
}