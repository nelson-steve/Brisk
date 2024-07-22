#pragma once

#include <Volk/volk.h>

#include <vector>

namespace Brisk {
	class FramebufferVulkan {
	public:
		void Create();
		void Release();
	private:
		std::vector<VkFramebuffer> m_SwapchainFramebuffers;
	};
}