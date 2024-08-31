#pragma once

#include <Volk/volk.h>

#include <vector>

namespace Brisk {
	struct UBO {
		//glm::mat4 model;
		//glm::mat4 view;
		//glm::mat4 proj;
	};

	class UniformBufferVulkan {
		void Create(uint32_t count);
		void Release();
	private:
		VkDeviceSize m_BufferSize;
		std::vector<VkBuffer> m_Buffers;
		std::vector<VkDeviceMemory> m_BuffersMemory;
		std::vector<void*> m_BuffersMapped;
	};
}