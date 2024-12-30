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

	void ShaderVulkan::Allocate(std::shared_ptr<Pipeline> pipeline) {
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_descriptor_pools.scene;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &m_descriptorSetLayouts.model;
		if (vkAllocateDescriptorSets(m_device, &allocInfo, &m_DescriptorSet) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
	}

	void ShaderVulkan::UpdateResources() {

	}

	void ShaderVulkan::Bind() {

	}

	void ShaderVulkan::SetPipeline(std::shared_ptr<Pipeline> pipeline) {

	}

	void ShaderVulkan::SetAlbedoTexture(std::shared_ptr<Texture> texture) {

	}

	void ShaderVulkan::SetNormalTexture(std::shared_ptr<Texture> texture) {

	}

	void ShaderVulkan::SetMetallicTexture(std::shared_ptr<Texture> texture) {

	}

	void ShaderVulkan::SetOcclusionTexture(std::shared_ptr<Texture> texture) {

	}

	void ShaderVulkan::SetEmissiveTexture(std::shared_ptr<Texture> texture) {

	}

	void ShaderVulkan::SetMVPBuffer() {

	}
}