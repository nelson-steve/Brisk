#include "ShaderVulkan.hpp"

#include "UtilitiesVulkan.hpp"
#include "GpuAdapterVulkan.hpp"

namespace Brisk
{
	void ShaderVulkan::Init(std::pair<std::string, Pipeline::ShaderStage> shaderInfo)
	{
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

	void ShaderVulkan::Allocate(std::shared_ptr<Pipeline> pipeline)
	{
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

	void ShaderVulkan::UpdateResources()
	{
		vkUpdateDescriptorSets(m_device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
	}

	void ShaderVulkan::Bind(VkCommandBuffer cmdBuffer)
	{
		vkCmdBindDescriptorSets(
			cmdBuffer,						 // Command buffer to bind the descriptor set to
			VK_PIPELINE_BIND_POINT_GRAPHICS, // We are binding it to a graphics pipeline
			pipelineLayout,					 // Pipeline layout used by the pipeline
			0,								 // First set index (usually 0)
			1,								 // Number of descriptor sets to bind
			&m_DescriptorSet,				 // Pointer to the descriptor set array
			0,								 // Number of dynamic offsets (1 in this case)
			nullptr							 // Pointer to the dynamic offsets (can be NULL)
		);
	}

	void ShaderVulkan::SetPipeline(std::shared_ptr<Pipeline> pipeline)
	{
	}

	void ShaderVulkan::SetAlbedoTexture(std::shared_ptr<Texture> texture)
	{
		VkWriteDescriptorSet descriptorWrite;
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.dstSet = m_DescriptorSet;
		descriptorWrite.dstBinding = 0;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &std::static_pointer_cast<TextureVulkan>(texture)->GetDescriptor();
		m_DescriptorWrites.push_back(descriptorWrite);
	}

	void ShaderVulkan::SetNormalTexture(std::shared_ptr<Texture> texture)
	{
		VkWriteDescriptorSet descriptorWrite;
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.dstSet = m_DescriptorSet;
		descriptorWrite.dstBinding = 1;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &std::static_pointer_cast<TextureVulkan>(texture)->GetDescriptor();
		m_DescriptorWrites.push_back(descriptorWrite);
	}

	void ShaderVulkan::SetMetallicTexture(std::shared_ptr<Texture> texture)
	{
		VkWriteDescriptorSet descriptorWrite;
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.dstSet = m_DescriptorSet;
		descriptorWrite.dstBinding = 2;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &std::static_pointer_cast<TextureVulkan>(texture)->GetDescriptor();
		m_DescriptorWrites.push_back(descriptorWrite);
	}

	void ShaderVulkan::SetOcclusionTexture(std::shared_ptr<Texture> texture)
	{
		VkWriteDescriptorSet descriptorWrite;
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.dstSet = m_DescriptorSet;
		descriptorWrite.dstBinding = 3;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &std::static_pointer_cast<TextureVulkan>(texture)->GetDescriptor();
		m_DescriptorWrites.push_back(descriptorWrite);
	}

	void ShaderVulkan::SetEmissiveTexture(std::shared_ptr<Texture> texture)
	{
		VkWriteDescriptorSet descriptorWrite;
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.dstSet = m_DescriptorSet;
		descriptorWrite.dstBinding = 4;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &std::static_pointer_cast<TextureVulkan>(texture)->GetDescriptor();
		m_DescriptorWrites.push_back(descriptorWrite);
	}

	void ShaderVulkan::SetMVPBuffer()
	{
	}
}