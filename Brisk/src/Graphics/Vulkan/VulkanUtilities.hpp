#pragma once

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.hpp>

namespace Brisk {
	class VulkanUtilities {
	public:
		static std::vector<const char*> GetRequiredExtensions();
		static bool CheckValidationLayerSupport(const std::vector<const char*> validationLayers);
		static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo);
		static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo, VkDebugUtilsMessengerEXT debugMessenger);
		static uint32_t FindMemoryType(VkPhysicalDevice device, uint32_t typeFilter, VkMemoryPropertyFlags properties);

		static const VkShaderModule CreateShaderModule(VkDevice device, const std::string& path);
		static std::vector<char>* ReadShaderFile(const std::string& fileName);
	};
}