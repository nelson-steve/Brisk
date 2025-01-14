#include "ShaderVulkan.hpp"

#include "UtilitiesVulkan.hpp"
#include "GpuAdapterVulkan.hpp"
#include "TextureVulkan.hpp"
#include "CommandBufferVulkan.hpp"
#include "DescriptorLayoutVulkan.hpp"

namespace Brisk
{
	void ShaderVulkan::Init(std::shared_ptr<Pipeline> pipeline, std::string layoutName)
	{
		VkDescriptorSetLayout layout{};
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		if (pipeline->m_Specs.pDescriptorLayouts.find(layoutName) != pipeline->m_Specs.pDescriptorLayouts.end()) {
			for (const auto& pair : pipeline->m_Specs.pDescriptorLayouts) {
				layout = std::static_pointer_cast<DescriptorLayoutVulkan>(pair.second)->GetLayout();
				break;
			}
		}

		allocInfo.descriptorPool = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDescriptorPool();
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &layout;
		if (vkAllocateDescriptorSets(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), &allocInfo, &m_DescriptorSet) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
	}

	void ShaderVulkan::UpdateResources()
	{
		vkUpdateDescriptorSets(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), m_DescriptorWrites.size(), m_DescriptorWrites.data(), 0, nullptr);
	}

	void ShaderVulkan::Allocate(std::shared_ptr<CommandBuffer> cmdBuffer, std::shared_ptr<Pipeline> pipeline) {
		//
	}

	void ShaderVulkan::Bind(std::shared_ptr<CommandBuffer> cmdBuffer, std::shared_ptr<Pipeline> pipeline)
	{
		vkCmdBindDescriptorSets(
			std::static_pointer_cast<CommandBufferVulkan>(cmdBuffer)->Get(),						 // Command buffer to bind the descriptor set to
			VK_PIPELINE_BIND_POINT_GRAPHICS, // We are binding it to a graphics pipeline
			std::static_pointer_cast<PipelineVulkan>(pipeline)->GetLayout(),					 // Pipeline layout used by the pipeline
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
		descriptorWrite.pImageInfo = std::static_pointer_cast<TextureVulkan>(texture)->GetDescriptor();
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
		descriptorWrite.pImageInfo = std::static_pointer_cast<TextureVulkan>(texture)->GetDescriptor();
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
		descriptorWrite.pImageInfo = std::static_pointer_cast<TextureVulkan>(texture)->GetDescriptor();
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
		descriptorWrite.pImageInfo = std::static_pointer_cast<TextureVulkan>(texture)->GetDescriptor();
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
		descriptorWrite.pImageInfo = std::static_pointer_cast<TextureVulkan>(texture)->GetDescriptor();
		m_DescriptorWrites.push_back(descriptorWrite);
	}

	void ShaderVulkan::SetMVPBuffer(std::shared_ptr<Buffer> buffer)
	{
		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.dstSet = m_DescriptorSet;
		descriptorWrite.dstBinding = 0;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = std::static_pointer_cast<BufferVulkan>(buffer)->GetDescriptor();
		m_DescriptorWrites.push_back(descriptorWrite);
	}
}