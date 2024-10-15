#include "ShaderVulkan.hpp"

#include "UtilitiesVulkan.hpp"
#include "GpuAdapterVulkan.hpp"

namespace Brisk
{
	void ShaderVulkan::Init(std::pair<std::string, Pipeline::ShaderStage> shaderInfo) {
		m_Module = UtilitiesVulkan::CreateShaderModule(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), "Shaders/Vulkan/Compiled/TriangleVS.spv");
		m_ShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		m_ShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
		m_ShaderStage.module = m_Module;
		m_ShaderStage.pName = "main";
	}
}