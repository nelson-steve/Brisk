#include "GraphicsDeviceVulkan.hpp"
#include "Engine/Engine.hpp"
#include "Defines.h"
#include "VulkanUtilities.hpp"

#include <Volk/volk.h>

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

	/// <summary>
	/// Static memebers declarations
	/// </summary>
	VkInstance GraphicsDeviceVulkan::s_Instance;
	VkSurfaceKHR GraphicsDeviceVulkan::m_Surface;

	std::vector<const char*> GraphicsDeviceVulkan::s_Extensions;
	std::vector<const char*> GraphicsDeviceVulkan::s_Layers;
	VkDebugUtilsMessengerCreateInfoEXT GraphicsDeviceVulkan::s_DebugCreateInfo;
	VkDebugUtilsMessengerEXT GraphicsDeviceVulkan::s_DebugMessenger;
	bool GraphicsDeviceVulkan::m_ValidationLayersFound;

	void GraphicsDeviceVulkan::Create(){
		volkInitialize();

		VkApplicationInfo appInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
		appInfo.pApplicationName = "Demo";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = Engine::s_EngineInfo.EngineName.c_str();
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		s_Extensions = VulkanUtilities::GetRequiredExtensions();
		m_ValidationLayersFound = false;
#if _DEBUG
		m_ValidationLayersFound = VulkanUtilities::CheckValidationLayerSupport(validationLayers);
		if (!m_ValidationLayersFound) {
			BRISK_APP_ERROR("Validation layers not found");
		}
#endif
		VkInstanceCreateInfo createInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(s_Extensions.size());
		createInfo.ppEnabledExtensionNames = s_Extensions.data();
#if _DEBUG
		createInfo.enabledLayerCount =
			m_ValidationLayersFound ? static_cast<uint32_t>(validationLayers.size()) : 0;
		createInfo.ppEnabledLayerNames =
			m_ValidationLayersFound ? validationLayers.data() : nullptr;

		VulkanUtilities::PopulateDebugMessengerCreateInfo();
		createInfo.pNext = &s_DebugCreateInfo;
#endif
		VK_LOG(vkCreateInstance(&createInfo, nullptr, &s_Instance), 
			"Failed to create Vulkan instance");

		volkLoadInstance(s_Instance);

#if _DEBUG
		VkResult result = VulkanUtilities::CreateDebugUtilsMessengerEXT();
		if (result == VK_ERROR_EXTENSION_NOT_PRESENT) {
			BRISK_APP_ERROR("Debug Utils extension not present");
		}
		else if(result != VK_SUCCESS) {
			BRISK_APP_ERROR("Failed to create debug messenger");
		}
#endif
	}

	void GraphicsDeviceVulkan::Release() {
		vkDestroyDebugUtilsMessengerEXT(s_Instance, s_DebugMessenger, nullptr);

		vkDestroyInstance(s_Instance, nullptr);
	}
}