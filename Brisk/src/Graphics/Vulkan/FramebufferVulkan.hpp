#pragma once

#include <Volk/volk.h>

#include <vector>

namespace Brisk {
	class FramebufferVulkan {
	public:
		void Create();
		void Release();

		const std::vector<VkFramebuffer>& GetSwapChainFramebuffers() const 
		{ 
			return m_SwapchainFramebuffers; 
		}

	private:
		std::vector<VkFramebuffer> m_SwapchainFramebuffers;
	};
}