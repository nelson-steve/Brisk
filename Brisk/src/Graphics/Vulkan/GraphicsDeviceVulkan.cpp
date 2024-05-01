#include "GraphicsDeviceVulkan.hpp"
#include "Engine/Engine.hpp"
#include "Defines.h"
#include "VulkanUtilities.hpp"

namespace Brisk 
{
	static std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
	};

	static void CheckAvailableExtensions() {
		VkResult result;

		/*
		 * From the link above:
		 * If `pProperties` is NULL, then the number of extensions properties
		 * available is returned in `pPropertyCount`.
		 *
		 * Basically, gets the number of extensions.
		 */
		uint32_t count = 0;
		result = vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
		if (result != VK_SUCCESS) {
			// Throw an exception or log the error
		}

		std::vector<VkExtensionProperties> extensionProperties(count);

		// Get the extensions
		result = vkEnumerateInstanceExtensionProperties(nullptr, &count, extensionProperties.data());
		if (result != VK_SUCCESS) {
			assert(false);
		}

		//std::set<std::string> extensions;
		std::cout << "Availble Instance Extension:" << std::endl;
		for (auto& extension : extensionProperties) {
			std::cout << extension.extensionName << std::endl;
		}

		std::vector<VkLayerProperties> layers;

		count = 0;
		result = vkEnumerateInstanceLayerProperties(&count, nullptr);
		if (result != VK_SUCCESS) {
			assert(false);
		}

		layers.resize(count);
		result = vkEnumerateInstanceLayerProperties(&count, layers.data());
		if (result != VK_SUCCESS) {
			assert(false);
		}
		std::cout << "Availble Instance Layers:" << std::endl;
		for (auto& layer : layers) {
			std::cout << layer.layerName << std::endl;
		}

	}

	VkInstance GraphicsDeviceVulkan::s_Instance;
	VkSurfaceKHR GraphicsDeviceVulkan::m_Surface;

	std::vector<const char*> GraphicsDeviceVulkan::s_Extensions;
	std::vector<const char*> GraphicsDeviceVulkan::s_Layers;
	VkDebugUtilsMessengerCreateInfoEXT GraphicsDeviceVulkan::s_DebugCreateInfo;
	VkDebugReportCallbackCreateInfoEXT GraphicsDeviceVulkan::s_ReportCreateInfo;
	VkDebugUtilsMessengerEXT GraphicsDeviceVulkan::s_DebugMessenger;
	VkDebugReportCallbackEXT GraphicsDeviceVulkan::s_ReportCallback;

	bool GraphicsDeviceVulkan::m_ValidationLayersFound;

	void GraphicsDeviceVulkan::Create(){
		VkApplicationInfo appInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
		appInfo.pApplicationName = "Demo";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = Engine::s_EngineInfo.EngineName.c_str();
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VulkanUtilities::GetRequiredExtensions();
#if _DEBUG
		if (!VulkanUtilities::CheckValidationLayerSupport(validationLayers)) {
			m_ValidationLayersFound = false;
			BRISK_APP_ERROR("Validation layers not found");
		}
		VulkanUtilities::PopulateDebugMessengerCreateInfo();
		VulkanUtilities::PopulateReportMessengerCreateInfo();
#endif
		VkInstanceCreateInfo createInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
		createInfo.pApplicationInfo = &appInfo;
#if _DEBUG
		createInfo.enabledLayerCount =
			m_ValidationLayersFound ? static_cast<uint32_t>(validationLayers.size()) : 0;
		createInfo.ppEnabledLayerNames =
			m_ValidationLayersFound ? validationLayers.data() : nullptr;
#endif
		createInfo.enabledExtensionCount = static_cast<uint32_t>(s_Extensions.size());
		createInfo.ppEnabledExtensionNames = s_Extensions.data();
		createInfo.pNext = &s_DebugCreateInfo;
		s_DebugCreateInfo.pNext = &s_ReportCreateInfo;
		
		VK_LOG(vkCreateInstance(&createInfo, nullptr, &s_Instance), 
			"Failed to create Vulkan instance");

		CheckAvailableExtensions();
#if _DEBUG
		//VkResult result = VulkanUtilities::CreateDebugUtilsMessengerEXT();
		//if (result == VK_ERROR_EXTENSION_NOT_PRESENT) {
		//	VK_LOG(VulkanUtilities::CreateReportMessengerEXT(),
		//		"Failed to setup debug messenger");
		//}
#endif
	}

	void GraphicsDeviceVulkan::Release() {
		vkDestroyInstance(s_Instance, nullptr);
	}
}