#include "VulkanUtilities.hpp"
#include "GraphicsDeviceVulkan.hpp"

#include <GLFW/glfw3.h>

#include <iostream>

namespace Brisk 
{
	bool VulkanUtilities::CheckValidationLayerSupport(const std::vector<const char*> validationLayers) {
		uint32_t layer_count;
		vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

		std::vector<VkLayerProperties> availableLayers(layer_count);
		vkEnumerateInstanceLayerProperties(&layer_count, availableLayers.data());
		for (const char* layerName : validationLayers) {
			bool layerFound = false;
			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}
			if (!layerFound) {
				return false;
			}
		}
		return true;
	}

	std::vector<const char*> VulkanUtilities::GetRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
#if _DEBUG
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif
		return extensions;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	int validation_error = 0;
	static VKAPI_ATTR VkBool32 VKAPI_CALL ReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object,
		size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData) {
		std::cerr << "validation layer " << validation_error++ << ": " << pMessage << std::endl;

		return VK_FALSE;
	}

	void VulkanUtilities::PopulateDebugMessengerCreateInfo() {
		GraphicsDeviceVulkan::s_DebugCreateInfo = {};
		GraphicsDeviceVulkan::s_DebugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		GraphicsDeviceVulkan::s_DebugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		GraphicsDeviceVulkan::s_DebugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		GraphicsDeviceVulkan::s_DebugCreateInfo.pfnUserCallback = DebugCallback;
	}

	void VulkanUtilities::PopulateReportMessengerCreateInfo() {
		//VK_DEBUG_REPORT_INFORMATION_BIT_EXT = 0x00000001,
		//	VK_DEBUG_REPORT_WARNING_BIT_EXT = 0x00000002,
		//	VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT = 0x00000004,
		//	VK_DEBUG_REPORT_ERROR_BIT_EXT = 0x00000008,
		//	VK_DEBUG_REPORT_DEBUG_BIT_EXT = 0x00000010,
		GraphicsDeviceVulkan::s_ReportCreateInfo = {};
		GraphicsDeviceVulkan::s_ReportCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		GraphicsDeviceVulkan::s_ReportCreateInfo.flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
			VK_DEBUG_REPORT_WARNING_BIT_EXT |
			VK_DEBUG_REPORT_ERROR_BIT_EXT |
			VK_DEBUG_REPORT_DEBUG_BIT_EXT |
			VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
		GraphicsDeviceVulkan::s_ReportCreateInfo.pfnCallback = ReportCallback;
	}

	VkResult VulkanUtilities::CreateDebugUtilsMessengerEXT() {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(GraphicsDeviceVulkan::s_Instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(GraphicsDeviceVulkan::s_Instance, &GraphicsDeviceVulkan::s_DebugCreateInfo, nullptr, &GraphicsDeviceVulkan::s_DebugMessenger);
		}
		else {
			std::cout << "Debug Utils Messenger extension not present" << std::endl;
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	VkResult VulkanUtilities::CreateReportMessengerEXT() {
		auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(GraphicsDeviceVulkan::s_Instance, "vkCreateDebugReportCallbackEXT");
		if (func != nullptr) {
			return func(GraphicsDeviceVulkan::s_Instance, &GraphicsDeviceVulkan::s_ReportCreateInfo, nullptr, &GraphicsDeviceVulkan::s_ReportCallback);
		}
		else {
			std::cout << "Debug Report extension not present" << std::endl;
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}
}