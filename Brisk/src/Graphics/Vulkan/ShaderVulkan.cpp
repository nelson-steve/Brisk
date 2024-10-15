#include "ShaderVulkan.hpp"

#include "UtilitiesVulkan.hpp"
#include "GpuAdapterVulkan.hpp"

namespace Brisk
{


	void ShaderVulkan::Init(std::pair<std::string, Pipeline::ShaderStage> shaderInfo) {
		m_Module = UtilitiesVulkan::CreateShaderModule(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), shaderInfo.first);
		m_ShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		if (shaderInfo.second == Pipeline::ShaderStage::VERTEX)
			m_ShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
		else if (shaderInfo.second == Pipeline::ShaderStage::FRAGMENT)
			m_ShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		else
			std::cout << "Wrong shader type";
		m_ShaderStage.module = m_Module;
		m_ShaderStage.pName = "main";
	}
}