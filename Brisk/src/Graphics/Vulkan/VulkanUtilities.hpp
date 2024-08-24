#pragma once

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.hpp>

namespace Brisk {
	class VulkanUtilities {
	public:
		static std::vector<const char*> GetRequiredExtensions();
		static bool CheckValidationLayerSupport(const std::vector<const char*> validationLayers);
		static void PopulateDebugMessengerCreateInfo();
		static VkResult CreateDebugUtilsMessengerEXT();

		static uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	};
}