#pragma once

#include "PhysicalDevice.hpp"
#include "Engine/WindowBase.hpp"

#include <Volk/volk.h>

#include <vector>

namespace Brisk 
{
	/// <summary>
	/// Forward declarations
	/// </summary>
	class Swapchain;

	class GraphicsDeviceVulkan {
	public:
		/// <summary>
		/// Create Vulkan Device
		/// </summary>
		void Create();

		/// <summary>
		/// Release all Vulkan resources cleanly
		/// </summary>
		void Release();

		Swapchain* CreateSwapchain(WindowBase* window);

		/// <summary>
		/// Getters for Vulkan handles
		/// </summary>
		static VkInstance GetInstance() { return s_Instance; }
		static std::vector<const char*> GetRequiredExtenstions() { return s_RequiredExtensions; }
		static std::vector<const char*> GetValidationLayers() { return s_ValidationLayers; }
	private:
		/// <summary>
		/// Vulkan handles
		/// </summary>
		static VkInstance s_Instance;

		/// <summary>
		/// Vulkan helper variables
		/// </summary>
		static std::vector<const char*> s_Extensions;
		static std::vector<const char*> s_Layers;
		static std::vector<const char*> s_RequiredExtensions;
		static std::vector<const char*> s_ValidationLayers;
		static VkDebugUtilsMessengerCreateInfoEXT s_DebugCreateInfo;
		static VkDebugUtilsMessengerEXT s_DebugMessenger;
		static bool m_ValidationLayersFound;

		/// <summary>
		/// Friend class declaration
		/// </summary>
		friend class VulkanUtilities;
	};
}