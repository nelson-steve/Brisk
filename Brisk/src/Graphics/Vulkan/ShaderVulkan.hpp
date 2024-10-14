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
		void AddDescriptor(const std::shared_ptr<Descriptor> descriptor);
	private:
		VkShaderModule m_Module;
		VkPipelineShaderStageCreateInfo m_ShaderStage;

		std::vector<std::shared_ptr<Descriptor>> m_Descriptors;
	};
}