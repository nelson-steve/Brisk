#include "ShaderModuleVulkan.hpp"
#include "UtilitiesVulkan.hpp"
#include "GpuAdapterVulkan.hpp"

namespace Brisk
{
	void ShaderModuleVulkan::Init(std::string path, Pipeline::ShaderStage type) {
		m_Module = UtilitiesVulkan::CreateShaderModule(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), path);
		m_ShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		if (type == Pipeline::ShaderStage::VERTEX)
			m_ShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
		else if (type == Pipeline::ShaderStage::FRAGMENT)
			m_ShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		else
			std::cout << "Wrong shader type";
		m_ShaderStage.module = m_Module;
		m_ShaderStage.pName = "main";
	}
}