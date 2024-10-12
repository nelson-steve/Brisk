#pragma once

#include <Volk/volk.h>
#define GLFW_INCLUDE_VULKAN

#include <vector>
#include <string>

namespace Brisk 
{
	class UtilitiesVulkan {
	public:
		static std::vector<const char*> GetRequiredExtensions();
		static bool CheckValidationLayerSupport(const std::vector<const char*> validationLayers);
		static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugCreateInfo);
		static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo, VkDebugUtilsMessengerEXT debugMessenger);
		static uint32_t FindMemoryType(VkPhysicalDevice device, uint32_t typeFilter, VkMemoryPropertyFlags properties);
		static void InsertImageMemoryBarrier(VkCommandBuffer cmdbuffer, VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
			VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
			VkImageSubresourceRange subresourceRange);

		static const VkShaderModule CreateShaderModule(VkDevice device, const std::string& path);
		static std::vector<char>* ReadShaderFile(const std::string& fileName);
	};
}