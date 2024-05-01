#pragma once

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.hpp>

#include <vector>

namespace Brisk 
{
	static class GraphicsDeviceVulkan {
	public:
		/// <summary>
		/// Create Vulkan Device
		/// </summary>
		static void Create();

		/// <summary>
		/// Release all Vulkan resources cleanly
		/// </summary>
		static void Release();
	private:
		/// <summary>
		/// Vulkan handles
		/// </summary>
		static VkInstance s_Instance;
		static VkSurfaceKHR m_Surface;

		/// <summary>
		/// Vulkan helper variables
		/// </summary>
		static std::vector<const char*> s_Extensions;
		static std::vector<const char*> s_Layers;
		static VkDebugUtilsMessengerCreateInfoEXT s_DebugCreateInfo;
		static VkDebugUtilsMessengerEXT s_DebugMessenger;
		static bool m_ValidationLayersFound;

		/// <summary>
		/// Friend class declaration
		/// </summary>
		friend class VulkanUtilities;
	};
}