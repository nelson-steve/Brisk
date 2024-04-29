#pragma once

#include <vulkan/vulkan.hpp>

namespace Brisk {
	static class GraphicsDeviceVulkan {
	public:
		static void Create();
		static std::vector<const char*> GetRequiredExtensions();
	private:
		static VkInstance s_Instance;
	};
}