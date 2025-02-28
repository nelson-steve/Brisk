#include "ShaderVulkan.hpp"

#include "UtilitiesVulkan.hpp"
#include "GpuAdapterVulkan.hpp"
#include "TextureVulkan.hpp"
#include "CommandBufferVulkan.hpp"
#include "DescriptorLayoutVulkan.hpp"

namespace Brisk
{
	void ShaderVulkan::Init(std::shared_ptr<Pipeline> pipeline)
	{
		{
			for (const auto& l : pipeline->m_GraphicsSpecs.pDescriptorLayouts) {
				VkDescriptorSet set;
				VkDescriptorSetLayout layout{};
				VkDescriptorSetAllocateInfo allocInfo{};
				allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				layout = std::static_pointer_cast<DescriptorLayoutVulkan>(l)->GetLayout();

				allocInfo.descriptorPool = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDescriptorPool();
				allocInfo.descriptorSetCount = 1;
				allocInfo.pSetLayouts = &layout;
				if (l->IsGlobal()) {
					if (vkAllocateDescriptorSets(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), &allocInfo, &set) != VK_SUCCESS)
					{
						throw std::runtime_error("failed to allocate descriptor sets!");
					}
					m_GlobalDescriptorSets.push_back(set);
				}
				else {
					if (vkAllocateDescriptorSets(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), &allocInfo, &set) != VK_SUCCESS)
					{
						throw std::runtime_error("failed to allocate descriptor sets!");
					}
					m_LocalDescriptorSets.push_back(set);
				}
			}
		}
	}

	void ShaderVulkan::UpdateResources()
	{
		vkUpdateDescriptorSets(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), static_cast<uint32_t>(m_DescriptorWrites.size()), m_DescriptorWrites.data(), 0, nullptr);
	}

	void ShaderVulkan::AddTextures(std::vector<std::shared_ptr<Texture>> textures) {
		std::vector<VkWriteDescriptorSet> writes;
		writes.resize(textures.size());
		for (int i = 0; i < textures.size(); i++) {
			VkWriteDescriptorSet& writeDescriptorSet3 = writes[i];
			writeDescriptorSet3.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSet3.dstSet = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_BindlessDescriptorSet;
			writeDescriptorSet3.dstBinding = 10;
			writeDescriptorSet3.dstArrayElement = i;
			writeDescriptorSet3.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writeDescriptorSet3.descriptorCount = 1;
			writeDescriptorSet3.pImageInfo = std::static_pointer_cast<TextureVulkan>(textures[i])->GetDescriptor();
		}

		vkUpdateDescriptorSets(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), writes.size(), writes.data(), 0, nullptr);
	}

	void ShaderVulkan::Allocate(std::shared_ptr<CommandBuffer> cmdBuffer, std::shared_ptr<Pipeline> pipeline) {
		//
	}

	void ShaderVulkan::Bind(std::shared_ptr<CommandBuffer> cmdBuffer, std::shared_ptr<Pipeline> pipeline)
	{
		std::vector<VkDescriptorSet> sets;
		sets.push_back(m_GlobalDescriptorSets[0]);
		sets.push_back(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_BindlessDescriptorSet);

		vkCmdBindDescriptorSets(
			std::static_pointer_cast<CommandBufferVulkan>(cmdBuffer)->Get(),						 // Command buffer to bind the descriptor set to
			VK_PIPELINE_BIND_POINT_GRAPHICS, // We are binding it to a graphics pipeline
			std::static_pointer_cast<PipelineVulkan>(pipeline)->GetLayout(),					 // Pipeline layout used by the pipeline
			0,								 // First set index (usually 0)
			sets.size(),								 // Number of descriptor sets to bind
			sets.data(),				 // Pointer to the descriptor set array
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