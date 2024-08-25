#include "UniformBufferVulkan.hpp"
#include "Engine/Engine.hpp"

namespace Brisk {
	void UniformBufferVulkan::Create(uint32_t count) {
		m_Buffers.resize(count);
		m_BuffersMemory.resize(count);
		m_BuffersMapped.resize(count);

		for (size_t i = 0; i < count; i++) {
			VkBufferCreateInfo createInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			createInfo.pNext = nullptr;
			m_BufferSize = sizeof(UBO);
			createInfo.size = m_BufferSize;
			createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

			if (vkCreateBuffer(Engine::s_PhysicalDevice->GetDevice(), &createInfo, nullptr, &m_Buffers[i]) != VK_SUCCESS) {
				//throw std::runtime_error("failed to create vertex buffer!");
			}

			createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);

			vkMapMemory(Engine::s_PhysicalDevice->GetDevice(), uniformBuffersMemory[i], 0, bufferSize, 0, &m_BuffersMapped[i]);
		}
	}
}