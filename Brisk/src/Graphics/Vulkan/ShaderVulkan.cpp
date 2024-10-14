#include "ShaderVulkan.hpp"

#include "UtilitiesVulkan.hpp"

namespace Brisk
{
	void ShaderVulkan::Init(std::pair<std::string, Pipeline::ShaderStage> shaderInfo) {
		m_Module = UtilitiesVulkan::CreateShaderModule(s_GPUDevice->GetDevice(), "Shaders/Vulkan/Compiled/TriangleVS.spv");
		m_ShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		m_ShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
		m_ShaderStage.module = m_Module;
		m_ShaderStage.pName = "main";
	}

	void ShaderVulkan::AddDescriptor(const std::shared_ptr<Descriptor> descriptor) {
		m_Descriptors.push_back(descriptor);
	}
}