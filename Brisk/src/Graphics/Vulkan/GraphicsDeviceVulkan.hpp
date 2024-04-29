#pragma once

#include <vulkan/vulkan.hpp>

namespace Brisk {
	static class GraphicsDeviceVulkan {
	public:
		static void Create();
		static void Release();
		static void GetRequiredExtensions();
	private:
		static VkInstance s_Instance;
		static std::vector<const char*> s_Extensions;
		static std::vector<const char*> s_Layers;
	};
}