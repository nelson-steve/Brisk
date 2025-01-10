#pragma once

#include "Engine/Renderer/ShaderModule.hpp"

#include <Volk/volk.h>

namespace Brisk
{
	class ShaderModuleVulkan : public ShaderModule {
	public:
		virtual void Init(std::pair<std::string, int> shaderInfo) override;

		VkShaderModule GetModule() { return m_Module; }
		VkPipelineShaderStageCreateInfo GetShaderStageInfo() { return m_ShaderStage; }

	private:
		VkShaderModule m_Module;
		VkPipelineShaderStageCreateInfo m_ShaderStage;
	};
}