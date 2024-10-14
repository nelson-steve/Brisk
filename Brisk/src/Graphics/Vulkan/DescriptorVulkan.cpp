#include "DescriptorVulkan.hpp"

namespace Brisk 
{
	DescriptorVulkan::DescriptorVulkan() {
        std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
        for (int i = 0; i < m_Layouts.size(); i++) {
            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = m_Layouts[i].p_Binding;
            layoutBinding.descriptorCount = m_Layouts[i].p_DescriptorCount;
            layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; //m_Layouts[i].p_Type
            layoutBinding.pImmutableSamplers = nullptr;
            layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

            layoutBindings.push_back(layoutBinding);
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
        layoutInfo.pBindings = layoutBindings.data();

        m_DescriptorLayouts.resize(1);
        if (vkCreateDescriptorSetLayout(s_GPUDevice->GetDevice(), &layoutInfo, nullptr, &m_DescriptorLayouts[0]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
	}

	void DescriptorVulkan::Allocate() {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        //allocInfo.descriptorPool = m_DescriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(m_DescriptorLayouts.size());
        allocInfo.pSetLayouts = m_DescriptorLayouts.data();

        if (vkAllocateDescriptorSets(s_GPUDevice->GetDevice(), &allocInfo, &m_Set) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }
	}

	void DescriptorVulkan::Update() {
	//void DescriptorVulkan::Update(uint32_t destination, uint32_t destinationElement) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_UniformBuffer->Get();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(MVPBuffer);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_Set;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(s_GPUDevice->GetDevice(), 1, &descriptorWrite, 0, nullptr);
	}
}