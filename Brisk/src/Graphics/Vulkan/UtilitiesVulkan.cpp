#include "UtilitiesVulkan.hpp"
#include "GpuAdapterVulkan.hpp"
#include "Engine/Window.hpp"
#include "Core/Log.hpp"

#include <glfw3.h>

#include <vector>
#include <fstream>
#include <string>
#include <iostream>

namespace Brisk 
{
	bool UtilitiesVulkan::CheckValidationLayerSupport(const std::vector<const char*> validationLayers) {
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

	std::vector<const char*> UtilitiesVulkan::GetRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
#if _DEBUG
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
		return extensions;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	void UtilitiesVulkan::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugCreateInfo) {
		debugCreateInfo = {};
		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugCreateInfo.pfnUserCallback = DebugCallback;
	}

	VkResult UtilitiesVulkan::CreateDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo, VkDebugUtilsMessengerEXT debugMessenger) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, &debugCreateInfo, nullptr, &debugMessenger);
		}
		else {
			std::cout << "Debug Utils Messenger extension not present" << std::endl;
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	uint32_t UtilitiesVulkan::FindMemoryType(VkPhysicalDevice device, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(device, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	void UtilitiesVulkan::InsertImageMemoryBarrier(VkCommandBuffer cmdbuffer, VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
		VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
		VkImageSubresourceRange subresourceRange)
	{
		VkImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.srcAccessMask = srcAccessMask;
		imageMemoryBarrier.dstAccessMask = dstAccessMask;
		imageMemoryBarrier.oldLayout = oldImageLayout;
		imageMemoryBarrier.newLayout = newImageLayout;
		imageMemoryBarrier.image = image;
		imageMemoryBarrier.subresourceRange = subresourceRange;

		vkCmdPipelineBarrier(
			cmdbuffer,
			srcStageMask,
			dstStageMask,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier);
	}

	std::vector<char>* UtilitiesVulkan::ReadShaderFile(const std::string& fileName) {
		std::vector<char>* shaderFileBuffer;
		std::ifstream file(fileName, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			BRISK_CORE_ERROR("Failed to open file: ", fileName);
		}

		size_t fileSize = (size_t)file.tellg();
		shaderFileBuffer = new std::vector<char>(fileSize);

		file.seekg(0);
		file.read(shaderFileBuffer->data(), fileSize);

		file.close();

		return shaderFileBuffer;
	}

	const VkShaderModule UtilitiesVulkan::CreateShaderModule(VkDevice device, const std::string& path) {
		const std::vector<char>* shaderCode = ReadShaderFile(path);

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = shaderCode->size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode->data());

		VkShaderModule  module;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &module) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}

		// Cleanup the data that's not needed anymore
		delete shaderCode;
		return module;
	}

	VkFormat UtilitiesVulkan::FormatToVulkanType(Core::Format format) {
		switch (format) {
			case Core::Format::FORMAT_UNDEFINED: return VK_FORMAT_UNDEFINED;
			case Core::Format::FORMAT_R4G4_UNORM_PACK8: return VK_FORMAT_R4G4_UNORM_PACK8;
			case Core::Format::FORMAT_R4G4B4A4_UNORM_PACK16: return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
			case Core::Format::FORMAT_B4G4R4A4_UNORM_PACK16: return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
			case Core::Format::FORMAT_R5G6B5_UNORM_PACK16: return VK_FORMAT_R5G6B5_UNORM_PACK16;
			case Core::Format::FORMAT_B5G6R5_UNORM_PACK16: return VK_FORMAT_B5G6R5_UNORM_PACK16;
			case Core::Format::FORMAT_R5G5B5A1_UNORM_PACK16: return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
			case Core::Format::FORMAT_B5G5R5A1_UNORM_PACK16: return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
			case Core::Format::FORMAT_A1R5G5B5_UNORM_PACK16: return VK_FORMAT_A1R5G5B5_UNORM_PACK16;
			case Core::Format::FORMAT_R8_UNORM: return VK_FORMAT_R8_UNORM;
			case Core::Format::FORMAT_R8_SNORM: return VK_FORMAT_R8_SNORM;
			case Core::Format::FORMAT_R8_USCALED: return VK_FORMAT_R8_USCALED;
			case Core::Format::FORMAT_R8_SSCALED: return VK_FORMAT_R8_SSCALED;
			case Core::Format::FORMAT_R8_UINT: return VK_FORMAT_R8_UINT;
			case Core::Format::FORMAT_R8_SINT: return VK_FORMAT_R8_SINT;
			case Core::Format::FORMAT_R8_SRGB: return VK_FORMAT_R8_SRGB;
			case Core::Format::FORMAT_R8G8_UNORM: return VK_FORMAT_R8G8_UNORM;
			case Core::Format::FORMAT_R8G8_SNORM: return VK_FORMAT_R8G8_SNORM;
			case Core::Format::FORMAT_R8G8_USCALED: return VK_FORMAT_R8G8_USCALED;
			case Core::Format::FORMAT_R8G8_SSCALED: return VK_FORMAT_R8G8_SSCALED;
			case Core::Format::FORMAT_R8G8_UINT: return VK_FORMAT_R8G8_UINT;
			case Core::Format::FORMAT_R8G8_SINT: return VK_FORMAT_R8G8_SINT;
			case Core::Format::FORMAT_R8G8_SRGB: return VK_FORMAT_R8G8_SRGB;
			case Core::Format::FORMAT_R8G8B8_UNORM: return VK_FORMAT_R8G8B8_UNORM;
			case Core::Format::FORMAT_R8G8B8_SNORM: return VK_FORMAT_R8G8B8_SNORM;
			case Core::Format::FORMAT_R8G8B8_USCALED: return VK_FORMAT_R8G8B8_USCALED;
			case Core::Format::FORMAT_R8G8B8_SSCALED: return VK_FORMAT_R8G8B8_SSCALED;
			case Core::Format::FORMAT_R8G8B8_UINT: return VK_FORMAT_R8G8B8_UINT;
			case Core::Format::FORMAT_R8G8B8_SINT: return VK_FORMAT_R8G8B8_SINT;
			case Core::Format::FORMAT_R8G8B8_SRGB: return VK_FORMAT_R8G8B8_SRGB;
			case Core::Format::FORMAT_B8G8R8_UNORM: return VK_FORMAT_B8G8R8_UNORM;
			case Core::Format::FORMAT_B8G8R8_SNORM: return VK_FORMAT_B8G8R8_SNORM;
			case Core::Format::FORMAT_B8G8R8_USCALED: return VK_FORMAT_B8G8R8_USCALED;
			case Core::Format::FORMAT_B8G8R8_SSCALED: return VK_FORMAT_B8G8R8_SSCALED;
			case Core::Format::FORMAT_B8G8R8_UINT: return VK_FORMAT_B8G8R8_UINT;
			case Core::Format::FORMAT_B8G8R8_SINT: return VK_FORMAT_B8G8R8_SINT;
			case Core::Format::FORMAT_B8G8R8_SRGB: return VK_FORMAT_B8G8R8_SRGB;
			case Core::Format::FORMAT_R8G8B8A8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
			case Core::Format::FORMAT_R8G8B8A8_SNORM: return VK_FORMAT_R8G8B8A8_SNORM;
			case Core::Format::FORMAT_R8G8B8A8_USCALED: return VK_FORMAT_R8G8B8A8_USCALED;
			case Core::Format::FORMAT_R8G8B8A8_SSCALED: return VK_FORMAT_R8G8B8A8_SSCALED;
			case Core::Format::FORMAT_R8G8B8A8_UINT: return VK_FORMAT_R8G8B8A8_UINT;
			case Core::Format::FORMAT_R8G8B8A8_SINT: return VK_FORMAT_R8G8B8A8_SINT;
			case Core::Format::FORMAT_R8G8B8A8_SRGB: return VK_FORMAT_R8G8B8A8_SRGB;
			case Core::Format::FORMAT_B8G8R8A8_UNORM: return VK_FORMAT_B8G8R8A8_UNORM;
			case Core::Format::FORMAT_B8G8R8A8_SNORM: return VK_FORMAT_B8G8R8A8_SNORM;
			case Core::Format::FORMAT_B8G8R8A8_USCALED: return VK_FORMAT_B8G8R8A8_USCALED;
			case Core::Format::FORMAT_B8G8R8A8_SSCALED: return VK_FORMAT_B8G8R8A8_SSCALED;
			case Core::Format::FORMAT_B8G8R8A8_UINT: return VK_FORMAT_B8G8R8A8_UINT;
			case Core::Format::FORMAT_B8G8R8A8_SINT: return VK_FORMAT_B8G8R8A8_SINT;
			case Core::Format::FORMAT_B8G8R8A8_SRGB: return VK_FORMAT_B8G8R8A8_SRGB;
				// Add more cases as needed for all the remaining formats...
			default: return VK_FORMAT_UNDEFINED;
		}
	}

	VkPolygonMode UtilitiesVulkan::PolygonToVulkanType(Pipeline::PolygonMode polygonMode) {
		switch (polygonMode)
		{
			case Pipeline::PolygonMode::POLYGON_MODE_FILL: return VK_POLYGON_MODE_FILL;
			case Pipeline::PolygonMode::POLYGON_MODE_LINE: return VK_POLYGON_MODE_LINE;
			case Pipeline::PolygonMode::POLYGON_MODE_POINT: return VK_POLYGON_MODE_POINT;
			default: return VK_POLYGON_MODE_FILL;
		}
	}

	VkFrontFace UtilitiesVulkan::FrontFaceToVulkanType(Pipeline::FrontFace frontFace) {
		switch (frontFace) {
			case Pipeline::FrontFace::COUTNER_CLOCKWISE: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
			case Pipeline::FrontFace::CLOCKWISE: return VK_FRONT_FACE_CLOCKWISE;
			default: return VK_FRONT_FACE_COUNTER_CLOCKWISE;  // Default to counter-clockwise
		}
	}

	VkCompareOp UtilitiesVulkan::CompareOpToVulkanType(Pipeline::CompareOp compareOp) {
		switch (compareOp) {
			case Pipeline::CompareOp::COMPARE_OP_NEVER: return VK_COMPARE_OP_NEVER;
			case Pipeline::CompareOp::COMPARE_OP_LESS: return VK_COMPARE_OP_LESS;
			case Pipeline::CompareOp::COMPARE_OP_EQUAL: return VK_COMPARE_OP_EQUAL;
			case Pipeline::CompareOp::COMPARE_OP_LESS_OR_EQUAL: return VK_COMPARE_OP_LESS_OR_EQUAL;
			case Pipeline::CompareOp::COMPARE_OP_GREATER: return VK_COMPARE_OP_GREATER;
			case Pipeline::CompareOp::COMPARE_OP_NOT_EQUAL: return VK_COMPARE_OP_NOT_EQUAL;
			case Pipeline::CompareOp::COMPARE_OP_GREATER_OR_EQUAL: return VK_COMPARE_OP_GREATER_OR_EQUAL;
			case Pipeline::CompareOp::COMPARE_OP_ALWAYS: return VK_COMPARE_OP_ALWAYS;
			default: return VK_COMPARE_OP_NEVER;  // Fallback to a safe default value
		}
	}
}