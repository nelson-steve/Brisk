#pragma once

#include "Engine/Renderer/RHI.hpp"
#include "Engine/Renderer/Pipeline.hpp"
#include "Core/Log.hpp"

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

		static VkPolygonMode PolygonToVulkanType(Pipeline::PolygonMode polygonMode);
		static VkFrontFace FrontFaceToVulkanType(Pipeline::FrontFace frontFace);
		static VkCompareOp CompareOpToVulkanType(Pipeline::CompareOp compareOp);

		static VkFormat FormatToVkFormat(Core::Format format) {
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
				case Core::Format::FORMAT_A8B8G8R8_UNORM_PACK32: return VK_FORMAT_A8B8G8R8_UNORM_PACK32;
				case Core::Format::FORMAT_A8B8G8R8_SNORM_PACK32: return VK_FORMAT_A8B8G8R8_SNORM_PACK32;
				case Core::Format::FORMAT_A8B8G8R8_USCALED_PACK32: return VK_FORMAT_A8B8G8R8_USCALED_PACK32;
				case Core::Format::FORMAT_A8B8G8R8_SSCALED_PACK32: return VK_FORMAT_A8B8G8R8_SSCALED_PACK32;
				case Core::Format::FORMAT_A8B8G8R8_UINT_PACK32: return VK_FORMAT_A8B8G8R8_UINT_PACK32;
				case Core::Format::FORMAT_A8B8G8R8_SINT_PACK32: return VK_FORMAT_A8B8G8R8_SINT_PACK32;
				case Core::Format::FORMAT_A8B8G8R8_SRGB_PACK32: return VK_FORMAT_A8B8G8R8_SRGB_PACK32;
				case Core::Format::FORMAT_A2R10G10B10_UNORM_PACK32: return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
				case Core::Format::FORMAT_A2R10G10B10_SNORM_PACK32: return VK_FORMAT_A2R10G10B10_SNORM_PACK32;
				case Core::Format::FORMAT_A2R10G10B10_USCALED_PACK32: return VK_FORMAT_A2R10G10B10_USCALED_PACK32;
				case Core::Format::FORMAT_A2R10G10B10_SSCALED_PACK32: return VK_FORMAT_A2R10G10B10_SSCALED_PACK32;
				case Core::Format::FORMAT_A2R10G10B10_UINT_PACK32: return VK_FORMAT_A2R10G10B10_UINT_PACK32;
				case Core::Format::FORMAT_A2R10G10B10_SINT_PACK32: return VK_FORMAT_A2R10G10B10_SINT_PACK32;
				case Core::Format::FORMAT_A2B10G10R10_UNORM_PACK32: return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
				case Core::Format::FORMAT_A2B10G10R10_SNORM_PACK32: return VK_FORMAT_A2B10G10R10_SNORM_PACK32;
				case Core::Format::FORMAT_A2B10G10R10_USCALED_PACK32: return VK_FORMAT_A2B10G10R10_USCALED_PACK32;
				case Core::Format::FORMAT_A2B10G10R10_SSCALED_PACK32: return VK_FORMAT_A2B10G10R10_SSCALED_PACK32;
				case Core::Format::FORMAT_A2B10G10R10_UINT_PACK32: return VK_FORMAT_A2B10G10R10_UINT_PACK32;
				case Core::Format::FORMAT_A2B10G10R10_SINT_PACK32: return VK_FORMAT_A2B10G10R10_SINT_PACK32;
				case Core::Format::FORMAT_R16_UNORM: return VK_FORMAT_R16_UNORM;
				case Core::Format::FORMAT_R16_SNORM: return VK_FORMAT_R16_SNORM;
				case Core::Format::FORMAT_R16_USCALED: return VK_FORMAT_R16_USCALED;
				case Core::Format::FORMAT_R16_SSCALED: return VK_FORMAT_R16_SSCALED;
				case Core::Format::FORMAT_R16_UINT: return VK_FORMAT_R16_UINT;
				case Core::Format::FORMAT_R16_SINT: return VK_FORMAT_R16_SINT;
				case Core::Format::FORMAT_R16_SFLOAT: return VK_FORMAT_R16_SFLOAT;
				case Core::Format::FORMAT_R16G16_UNORM: return VK_FORMAT_R16G16_UNORM;
				case Core::Format::FORMAT_R16G16_SNORM: return VK_FORMAT_R16G16_SNORM;
				case Core::Format::FORMAT_R16G16_USCALED: return VK_FORMAT_R16G16_USCALED;
				case Core::Format::FORMAT_R16G16_SSCALED: return VK_FORMAT_R16G16_SSCALED;
				case Core::Format::FORMAT_R16G16_UINT: return VK_FORMAT_R16G16_UINT;
				case Core::Format::FORMAT_R16G16_SINT: return VK_FORMAT_R16G16_SINT;
				case Core::Format::FORMAT_R16G16_SFLOAT: return VK_FORMAT_R16G16_SFLOAT;
				case Core::Format::FORMAT_R16G16B16_UNORM: return VK_FORMAT_R16G16B16_UNORM;
				case Core::Format::FORMAT_R16G16B16_SNORM: return VK_FORMAT_R16G16B16_SNORM;
				case Core::Format::FORMAT_R16G16B16_USCALED: return VK_FORMAT_R16G16B16_USCALED;
				case Core::Format::FORMAT_R16G16B16_SSCALED: return VK_FORMAT_R16G16B16_SSCALED;
				case Core::Format::FORMAT_R16G16B16_UINT: return VK_FORMAT_R16G16B16_UINT;
				case Core::Format::FORMAT_R16G16B16_SINT: return VK_FORMAT_R16G16B16_SINT;
				case Core::Format::FORMAT_R16G16B16_SFLOAT: return VK_FORMAT_R16G16B16_SFLOAT;
				case Core::Format::FORMAT_R16G16B16A16_UNORM: return VK_FORMAT_R16G16B16A16_UNORM;
				case Core::Format::FORMAT_R16G16B16A16_SNORM: return VK_FORMAT_R16G16B16A16_SNORM;
				case Core::Format::FORMAT_R16G16B16A16_USCALED: return VK_FORMAT_R16G16B16A16_USCALED;
				case Core::Format::FORMAT_R16G16B16A16_SSCALED: return VK_FORMAT_R16G16B16A16_SSCALED;
				case Core::Format::FORMAT_R16G16B16A16_UINT: return VK_FORMAT_R16G16B16A16_UINT;
				case Core::Format::FORMAT_R16G16B16A16_SINT: return VK_FORMAT_R16G16B16A16_SINT;
				case Core::Format::FORMAT_R16G16B16A16_SFLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
				case Core::Format::FORMAT_R32_UINT: return VK_FORMAT_R32_UINT;
				case Core::Format::FORMAT_R32_SINT: return VK_FORMAT_R32_SINT;
				case Core::Format::FORMAT_R32_SFLOAT: return VK_FORMAT_R32_SFLOAT;
				case Core::Format::FORMAT_R32G32_UINT: return VK_FORMAT_R32G32_UINT;
				case Core::Format::FORMAT_R32G32_SINT: return VK_FORMAT_R32G32_SINT;
				case Core::Format::FORMAT_R32G32_SFLOAT: return VK_FORMAT_R32G32_SFLOAT;
				case Core::Format::FORMAT_R32G32B32_UINT: return VK_FORMAT_R32G32B32_UINT;
				case Core::Format::FORMAT_R32G32B32_SINT: return VK_FORMAT_R32G32B32_SINT;
				case Core::Format::FORMAT_R32G32B32_SFLOAT: return VK_FORMAT_R32G32B32_SFLOAT;
				case Core::Format::FORMAT_R32G32B32A32_UINT: return VK_FORMAT_R32G32B32A32_UINT;
				case Core::Format::FORMAT_R32G32B32A32_SINT: return VK_FORMAT_R32G32B32A32_SINT;
				case Core::Format::FORMAT_R32G32B32A32_SFLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
				case Core::Format::FORMAT_R64_UINT: return VK_FORMAT_R64_UINT;
				case Core::Format::FORMAT_R64_SINT: return VK_FORMAT_R64_SINT;
				case Core::Format::FORMAT_R64_SFLOAT: return VK_FORMAT_R64_SFLOAT;
				case Core::Format::FORMAT_R64G64_UINT: return VK_FORMAT_R64G64_UINT;
				case Core::Format::FORMAT_R64G64_SINT: return VK_FORMAT_R64G64_SINT;
				case Core::Format::FORMAT_R64G64_SFLOAT: return VK_FORMAT_R64G64_SFLOAT;
				case Core::Format::FORMAT_R64G64B64_UINT: return VK_FORMAT_R64G64B64_UINT;
				case Core::Format::FORMAT_R64G64B64_SINT: return VK_FORMAT_R64G64B64_SINT;
				case Core::Format::FORMAT_R64G64B64_SFLOAT: return VK_FORMAT_R64G64B64_SFLOAT;
				case Core::Format::FORMAT_R64G64B64A64_UINT: return VK_FORMAT_R64G64B64A64_UINT;
				case Core::Format::FORMAT_R64G64B64A64_SINT: return VK_FORMAT_R64G64B64A64_SINT;
				case Core::Format::FORMAT_R64G64B64A64_SFLOAT: return VK_FORMAT_R64G64B64A64_SFLOAT;
				case Core::Format::FORMAT_B10G11R11_UFLOAT_PACK32: return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
				case Core::Format::FORMAT_E5B9G9R9_UFLOAT_PACK32: return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
				case Core::Format::FORMAT_D16_UNORM: return VK_FORMAT_D16_UNORM;
				case Core::Format::FORMAT_X8_D24_UNORM_PACK32: return VK_FORMAT_X8_D24_UNORM_PACK32;
				case Core::Format::FORMAT_D32_SFLOAT: return VK_FORMAT_D32_SFLOAT;
				case Core::Format::FORMAT_S8_UINT: return VK_FORMAT_S8_UINT;
				case Core::Format::FORMAT_D16_UNORM_S8_UINT: return VK_FORMAT_D16_UNORM_S8_UINT;
				case Core::Format::FORMAT_D24_UNORM_S8_UINT: return VK_FORMAT_D24_UNORM_S8_UINT;
				case Core::Format::FORMAT_D32_SFLOAT_S8_UINT: return VK_FORMAT_D32_SFLOAT_S8_UINT;
				case Core::Format::FORMAT_BC1_RGB_UNORM_BLOCK: return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
				case Core::Format::FORMAT_BC1_RGB_SRGB_BLOCK: return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
				case Core::Format::FORMAT_BC1_RGBA_UNORM_BLOCK: return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
				case Core::Format::FORMAT_BC1_RGBA_SRGB_BLOCK: return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
				case Core::Format::FORMAT_BC2_UNORM_BLOCK: return VK_FORMAT_BC2_UNORM_BLOCK;
				case Core::Format::FORMAT_BC2_SRGB_BLOCK: return VK_FORMAT_BC2_SRGB_BLOCK;
				case Core::Format::FORMAT_BC3_UNORM_BLOCK: return VK_FORMAT_BC3_UNORM_BLOCK;
				case Core::Format::FORMAT_BC3_SRGB_BLOCK: return VK_FORMAT_BC3_SRGB_BLOCK;
				case Core::Format::FORMAT_BC4_UNORM_BLOCK: return VK_FORMAT_BC4_UNORM_BLOCK;
				case Core::Format::FORMAT_BC4_SNORM_BLOCK: return VK_FORMAT_BC4_SNORM_BLOCK;
				case Core::Format::FORMAT_BC5_UNORM_BLOCK: return VK_FORMAT_BC5_UNORM_BLOCK;
				case Core::Format::FORMAT_BC5_SNORM_BLOCK: return VK_FORMAT_BC5_SNORM_BLOCK;
				case Core::Format::FORMAT_BC6H_UFLOAT_BLOCK: return VK_FORMAT_BC6H_UFLOAT_BLOCK;
				case Core::Format::FORMAT_BC6H_SFLOAT_BLOCK: return VK_FORMAT_BC6H_SFLOAT_BLOCK;
				case Core::Format::FORMAT_BC7_UNORM_BLOCK: return VK_FORMAT_BC7_UNORM_BLOCK;
				case Core::Format::FORMAT_BC7_SRGB_BLOCK: return VK_FORMAT_BC7_SRGB_BLOCK;
				case Core::Format::FORMAT_ETC2_R8G8B8_UNORM_BLOCK: return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
				case Core::Format::FORMAT_ETC2_R8G8B8_SRGB_BLOCK: return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
				case Core::Format::FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK: return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
				case Core::Format::FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK: return VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;
				case Core::Format::FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK: return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
				case Core::Format::FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK: return VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
				case Core::Format::FORMAT_EAC_R11_UNORM_BLOCK: return VK_FORMAT_EAC_R11_UNORM_BLOCK;
				case Core::Format::FORMAT_EAC_R11_SNORM_BLOCK: return VK_FORMAT_EAC_R11_SNORM_BLOCK;
				case Core::Format::FORMAT_EAC_R11G11_UNORM_BLOCK: return VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
				case Core::Format::FORMAT_EAC_R11G11_SNORM_BLOCK: return VK_FORMAT_EAC_R11G11_SNORM_BLOCK;
				case Core::Format::FORMAT_ASTC_4x4_UNORM_BLOCK: return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
				case Core::Format::FORMAT_ASTC_4x4_SRGB_BLOCK: return VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
				case Core::Format::FORMAT_ASTC_5x4_UNORM_BLOCK: return VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
				case Core::Format::FORMAT_ASTC_5x4_SRGB_BLOCK: return VK_FORMAT_ASTC_5x4_SRGB_BLOCK;
				case Core::Format::FORMAT_ASTC_5x5_UNORM_BLOCK: return VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
				case Core::Format::FORMAT_ASTC_5x5_SRGB_BLOCK: return VK_FORMAT_ASTC_5x5_SRGB_BLOCK;
				case Core::Format::FORMAT_ASTC_6x5_UNORM_BLOCK: return VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
				case Core::Format::FORMAT_ASTC_6x5_SRGB_BLOCK: return VK_FORMAT_ASTC_6x5_SRGB_BLOCK;
				case Core::Format::FORMAT_ASTC_6x6_UNORM_BLOCK: return VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
				case Core::Format::FORMAT_ASTC_6x6_SRGB_BLOCK: return VK_FORMAT_ASTC_6x6_SRGB_BLOCK;
				case Core::Format::FORMAT_ASTC_8x5_UNORM_BLOCK: return VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
				case Core::Format::FORMAT_ASTC_8x5_SRGB_BLOCK: return VK_FORMAT_ASTC_8x5_SRGB_BLOCK;
				case Core::Format::FORMAT_ASTC_8x6_UNORM_BLOCK: return VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
				case Core::Format::FORMAT_ASTC_8x6_SRGB_BLOCK: return VK_FORMAT_ASTC_8x6_SRGB_BLOCK;
				case Core::Format::FORMAT_ASTC_8x8_UNORM_BLOCK: return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
				case Core::Format::FORMAT_ASTC_8x8_SRGB_BLOCK: return VK_FORMAT_ASTC_8x8_SRGB_BLOCK;
				case Core::Format::FORMAT_ASTC_10x5_UNORM_BLOCK: return VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
				case Core::Format::FORMAT_ASTC_10x5_SRGB_BLOCK: return VK_FORMAT_ASTC_10x5_SRGB_BLOCK;
				case Core::Format::FORMAT_ASTC_10x6_UNORM_BLOCK: return VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
				case Core::Format::FORMAT_ASTC_10x6_SRGB_BLOCK: return VK_FORMAT_ASTC_10x6_SRGB_BLOCK;
				case Core::Format::FORMAT_ASTC_10x8_UNORM_BLOCK: return VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
				case Core::Format::FORMAT_ASTC_10x8_SRGB_BLOCK: return VK_FORMAT_ASTC_10x8_SRGB_BLOCK;
				case Core::Format::FORMAT_ASTC_10x10_UNORM_BLOCK: return VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
				case Core::Format::FORMAT_ASTC_10x10_SRGB_BLOCK: return VK_FORMAT_ASTC_10x10_SRGB_BLOCK;
				case Core::Format::FORMAT_ASTC_12x10_UNORM_BLOCK: return VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
				case Core::Format::FORMAT_ASTC_12x10_SRGB_BLOCK: return VK_FORMAT_ASTC_12x10_SRGB_BLOCK;
				case Core::Format::FORMAT_ASTC_12x12_UNORM_BLOCK: return VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
				case Core::Format::FORMAT_ASTC_12x12_SRGB_BLOCK: return VK_FORMAT_ASTC_12x12_SRGB_BLOCK;
			}
			assert(false);
		}

		static VkBufferUsageFlags BufferUsageToVkFormat(Core::BufferUsage usageFlags) {
			VkBufferUsageFlags flags = 0;

			if (HasFlag(usageFlags, Core::BufferUsage::TransferSrc))                flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			if (HasFlag(usageFlags, Core::BufferUsage::TransferDst))                flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			if (HasFlag(usageFlags, Core::BufferUsage::UniformTexelBuffer))         flags |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
			if (HasFlag(usageFlags, Core::BufferUsage::StorageTexelBuffer))         flags |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
			if (HasFlag(usageFlags, Core::BufferUsage::UniformBuffer))              flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			if (HasFlag(usageFlags, Core::BufferUsage::StorageBuffer))              flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			if (HasFlag(usageFlags, Core::BufferUsage::IndexBuffer))                flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			if (HasFlag(usageFlags, Core::BufferUsage::VertexBuffer))               flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			if (HasFlag(usageFlags, Core::BufferUsage::IndirectBuffer))             flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
			if (HasFlag(usageFlags, Core::BufferUsage::ShaderDeviceAddress))        flags |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
			if (HasFlag(usageFlags, Core::BufferUsage::VideoDecodeSrcKHR))          flags |= VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR;
			if (HasFlag(usageFlags, Core::BufferUsage::VideoDecodeDstKHR))          flags |= VK_BUFFER_USAGE_VIDEO_DECODE_DST_BIT_KHR;
			if (HasFlag(usageFlags, Core::BufferUsage::TransformFeedbackBufferEXT)) flags |= VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT;
			if (HasFlag(usageFlags, Core::BufferUsage::TransformFeedbackCounterBufferEXT)) flags |= VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT;
			if (HasFlag(usageFlags, Core::BufferUsage::ConditionalRenderingEXT))    flags |= VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT;

			return flags;
		}

		static VkMemoryPropertyFlags ToVkMemoryPropertyFlags(Core::MemoryProperty memFlags) {
			VkMemoryPropertyFlags flags = 0;

			if (HasFlag(memFlags, Core::MemoryProperty::DeviceLocal))        flags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			if (HasFlag(memFlags, Core::MemoryProperty::HostVisible))        flags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			if (HasFlag(memFlags, Core::MemoryProperty::HostCoherent))       flags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			if (HasFlag(memFlags, Core::MemoryProperty::HostCached))         flags |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
			if (HasFlag(memFlags, Core::MemoryProperty::LazilyAllocated))    flags |= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
			if (HasFlag(memFlags, Core::MemoryProperty::Protected))          flags |= VK_MEMORY_PROPERTY_PROTECTED_BIT;
			// The following two are vendor-specific extensions and not part of core Vulkan
			if (HasFlag(memFlags, Core::MemoryProperty::DeviceCoherentAMD))  flags |= VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD;
			if (HasFlag(memFlags, Core::MemoryProperty::DeviceUncachedAMD))  flags |= VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD;
			if (HasFlag(memFlags, Core::MemoryProperty::RdmaCapableNV))      flags |= VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV;

			return flags;
		}


		//static VkBufferUsageFlags BufferUsageToVkFormat(Core::BufferUsage usageFlag) {
		//	switch (usageFlag)
		//	{
		//		case Core::BUFFER_USAGE_TRANSFER_SRC_BIT: return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		//		case Core::BUFFER_USAGE_TRANSFER_DST_BIT: return VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		//		case Core::BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT: return VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
		//		case Core::BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT: return VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
		//		case Core::BUFFER_USAGE_UNIFORM_BUFFER_BIT: return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		//		case Core::BUFFER_USAGE_STORAGE_BUFFER_BIT: return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		//		case Core::BUFFER_USAGE_INDEX_BUFFER_BIT: return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		//		case Core::BUFFER_USAGE_VERTEX_BUFFER_BIT: return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		//		case Core::BUFFER_USAGE_INDIRECT_BUFFER_BIT: return VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
		//		case Core::BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT: return VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		//		case Core::BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR: return VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR;
		//		case Core::BUFFER_USAGE_VIDEO_DECODE_DST_BIT_KHR: return VK_BUFFER_USAGE_VIDEO_DECODE_DST_BIT_KHR;
		//		case Core::BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT: return VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT;
		//		case Core::BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT: return VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT;
		//		case Core::BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT: return VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT;
		//	}
		//	assert(false);
		//}

		//static VkMemoryPropertyFlags MemoryPropertyToVkFormat(Core::MemoryProperty memoryPropertyFlag) {
		//	switch (memoryPropertyFlag)
		//	{
		//		case Core::MEMORY_PROPERTY_DEVICE_LOCAL_BIT: return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		//		case Core::MEMORY_PROPERTY_HOST_VISIBLE_BIT: return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		//		case Core::MEMORY_PROPERTY_HOST_COHERENT_BIT: return VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		//		case Core::MEMORY_PROPERTY_HOST_CACHED_BIT: return VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
		//		case Core::MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT: return VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
		//		case Core::MEMORY_PROPERTY_PROTECTED_BIT: return VK_MEMORY_PROPERTY_PROTECTED_BIT;
		//		case Core::MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD: return VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD;
		//		case Core::MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD: return VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD;
		//		case Core::MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV: return VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV;
		//		case Core::MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM: return VK_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM;
		//	}
		//	assert(false);
		//}

		static VkDescriptorType ResourceToDescriptorType(GPUResource::ResourceType resourceType) {
			switch (resourceType)
			{
				case GPUResource::ResourceType::DESCRIPTOR_TYPE_SAMPLER: return VK_DESCRIPTOR_TYPE_SAMPLER;
				case GPUResource::ResourceType::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				case GPUResource::ResourceType::DESCRIPTOR_TYPE_SAMPLED_IMAGE: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
				case GPUResource::ResourceType::DESCRIPTOR_TYPE_STORAGE_IMAGE: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				case GPUResource::ResourceType::DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
				case GPUResource::ResourceType::DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
				case GPUResource::ResourceType::DESCRIPTOR_TYPE_UNIFORM_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				case GPUResource::ResourceType::DESCRIPTOR_TYPE_STORAGE_BUFFER: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				case GPUResource::ResourceType::DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
				case GPUResource::ResourceType::DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
				case GPUResource::ResourceType::DESCRIPTOR_TYPE_INPUT_ATTACHMENT: return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
				case GPUResource::ResourceType::DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK: return VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK;
				case GPUResource::ResourceType::DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR: return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
				case GPUResource::ResourceType::DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV: return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
				case GPUResource::ResourceType::DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM: return VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM;
				case GPUResource::ResourceType::DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM: return VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM;
				case GPUResource::ResourceType::DESCRIPTOR_TYPE_MUTABLE_EXT: return VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
			}
			assert(false);
		}

		static VkShaderStageFlagBits ShaderStageToVkType(GPUResource::ShaderStageAccess stage) {
			switch (stage)
			{
				case GPUResource::ShaderStageAccess::SHADER_STAGE_VERTEX_BIT: return VK_SHADER_STAGE_VERTEX_BIT;
				case GPUResource::ShaderStageAccess::SHADER_STAGE_TESSELLATION_CONTROL_BIT: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
				case GPUResource::ShaderStageAccess::SHADER_STAGE_TESSELLATION_EVALUATION_BIT: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
				case GPUResource::ShaderStageAccess::SHADER_STAGE_GEOMETRY_BIT: return VK_SHADER_STAGE_GEOMETRY_BIT;
				case GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT: return VK_SHADER_STAGE_FRAGMENT_BIT;
				case GPUResource::ShaderStageAccess::SHADER_STAGE_COMPUTE_BIT: return VK_SHADER_STAGE_COMPUTE_BIT;
				case GPUResource::ShaderStageAccess::SHADER_STAGE_ALL_GRAPHICS: return VK_SHADER_STAGE_ALL_GRAPHICS;
				case GPUResource::ShaderStageAccess::SHADER_STAGE_ALL: return VK_SHADER_STAGE_ALL;
				case GPUResource::ShaderStageAccess::SHADER_STAGE_RAYGEN_BIT_KHR: return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
				case GPUResource::ShaderStageAccess::SHADER_STAGE_ANY_HIT_BIT_KHR: return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
				case GPUResource::ShaderStageAccess::SHADER_STAGE_CLOSEST_HIT_BIT_KHR: return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
				case GPUResource::ShaderStageAccess::SHADER_STAGE_MISS_BIT_KHR: return VK_SHADER_STAGE_MISS_BIT_KHR;
				case GPUResource::ShaderStageAccess::SHADER_STAGE_INTERSECTION_BIT_KHR: return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
				case GPUResource::ShaderStageAccess::SHADER_STAGE_CALLABLE_BIT_KHR: return VK_SHADER_STAGE_CALLABLE_BIT_KHR;
				case GPUResource::ShaderStageAccess::SHADER_STAGE_TASK_BIT_EXT: return VK_SHADER_STAGE_TASK_BIT_EXT;
				case GPUResource::ShaderStageAccess::SHADER_STAGE_MESH_BIT_EXT: return VK_SHADER_STAGE_MESH_BIT_EXT;
				case GPUResource::ShaderStageAccess::SHADER_STAGE_SUBPASS_SHADING_BIT_HUAWEI: return VK_SHADER_STAGE_SUBPASS_SHADING_BIT_HUAWEI;
				case GPUResource::ShaderStageAccess::SHADER_STAGE_CLUSTER_CULLING_BIT_HUAWEI: return VK_SHADER_STAGE_CLUSTER_CULLING_BIT_HUAWEI;
			}
			BRISK_CORE_ERROR("Invalid shader stage parameter");
		}

		static VkImageUsageFlags ImageUsageToVulkanType(Core::TextureUsage usage) {
			VkImageUsageFlags flags = 0;

			if ((usage & Core::TextureUsage::ImageUsageTransferSrc) != Core::TextureUsage::Undefined)            flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			if ((usage & Core::TextureUsage::ImageUsageTransferDst) != Core::TextureUsage::Undefined)            flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			if ((usage & Core::TextureUsage::ImageUsageSampled) != Core::TextureUsage::Undefined)                flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
			if ((usage & Core::TextureUsage::ImageUsageStorage) != Core::TextureUsage::Undefined)                flags |= VK_IMAGE_USAGE_STORAGE_BIT;
			if ((usage & Core::TextureUsage::ImageUsageColorAttachment) != Core::TextureUsage::Undefined)        flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			if ((usage & Core::TextureUsage::ImageUsageDepthStencilAttachment) != Core::TextureUsage::Undefined) flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

			return flags;
		}

		static VkImageLayout ImageLayoutToVkImageLayout(Core::ImageLayout layout) {
			if ((layout & Core::ImageLayout::ColorAttachmentOptimal) == Core::ImageLayout::ColorAttachmentOptimal)
				return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			if ((layout & Core::ImageLayout::DepthStencilAttachmentOptimal) == Core::ImageLayout::DepthStencilAttachmentOptimal)
				return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			if ((layout & Core::ImageLayout::ShaderReadOnlyOptimal) == Core::ImageLayout::ShaderReadOnlyOptimal)
				return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			if ((layout & Core::ImageLayout::TransferSrc) == Core::ImageLayout::TransferSrc)
				return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

			if ((layout & Core::ImageLayout::TransferDst) == Core::ImageLayout::TransferDst)
				return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

			if ((layout & Core::ImageLayout::General) == Core::ImageLayout::General)
				return VK_IMAGE_LAYOUT_GENERAL;

			if ((layout & Core::ImageLayout::DepthStencilReadOnlyOptimal) == Core::ImageLayout::DepthStencilReadOnlyOptimal)
				return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

			if ((layout & Core::ImageLayout::PresentSrc) == Core::ImageLayout::PresentSrc)
				return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			if ((layout & Core::ImageLayout::ComputeShaderWrite) == Core::ImageLayout::ComputeShaderWrite)
				return VK_IMAGE_LAYOUT_GENERAL;

			if ((layout & Core::ImageLayout::AttachmentFeedbackLoopOptimal) == Core::ImageLayout::AttachmentFeedbackLoopOptimal)
				return VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT;

			return VK_IMAGE_LAYOUT_UNDEFINED;
		}


		static VkAccessFlags AccessTypeToVkAccessFlags(Core::AccessType access) {
			VkAccessFlags flags = 0;

			if ((access & Core::AccessType::ShaderRead) == Core::AccessType::ShaderRead)
				flags |= VK_ACCESS_SHADER_READ_BIT;

			if ((access & Core::AccessType::ShaderWrite) == Core::AccessType::ShaderWrite)
				flags |= VK_ACCESS_SHADER_WRITE_BIT;

			if ((access & Core::AccessType::TransferRead) == Core::AccessType::TransferRead)
				flags |= VK_ACCESS_TRANSFER_READ_BIT;

			if ((access & Core::AccessType::TransferWrite) == Core::AccessType::TransferWrite)
				flags |= VK_ACCESS_TRANSFER_WRITE_BIT;

			if ((access & Core::AccessType::ColorAttachmentRead) == Core::AccessType::ColorAttachmentRead)
				flags |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

			if ((access & Core::AccessType::ColorAttachmentWrite) == Core::AccessType::ColorAttachmentWrite)
				flags |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			if ((access & Core::AccessType::DepthStencilRead) == Core::AccessType::DepthStencilRead)
				flags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

			if ((access & Core::AccessType::DepthStencilWrite) == Core::AccessType::DepthStencilWrite)
				flags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			if ((access & Core::AccessType::HostRead) == Core::AccessType::HostRead)
				flags |= VK_ACCESS_HOST_READ_BIT;

			if ((access & Core::AccessType::HostWrite) == Core::AccessType::HostWrite)
				flags |= VK_ACCESS_HOST_WRITE_BIT;

			if ((access & Core::AccessType::MemoryRead) == Core::AccessType::MemoryRead)
				flags |= VK_ACCESS_MEMORY_READ_BIT;

			if ((access & Core::AccessType::MemoryWrite) == Core::AccessType::MemoryWrite)
				flags |= VK_ACCESS_MEMORY_WRITE_BIT;

			if ((access & Core::AccessType::InputAttachmentRead) == Core::AccessType::InputAttachmentRead)
				flags |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;

			if ((access & Core::AccessType::IndirectCommandRead) == Core::AccessType::IndirectCommandRead)
				flags |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;

			if ((access & Core::AccessType::VertexAttributeRead) == Core::AccessType::VertexAttributeRead)
				flags |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;

			if ((access & Core::AccessType::UniformRead) == Core::AccessType::UniformRead)
				flags |= VK_ACCESS_UNIFORM_READ_BIT;

			return flags;
		}


		static VkPipelineStageFlags PipelineStageToVkPipelineStageFlags(Core::PipelineStage stage) {
			VkPipelineStageFlags flags = 0;

			if ((stage & Core::PipelineStage::TopOfPipe) == Core::PipelineStage::TopOfPipe)
				flags |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

			if ((stage & Core::PipelineStage::BottomOfPipe) == Core::PipelineStage::BottomOfPipe)
				flags |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

			if ((stage & Core::PipelineStage::TransferStage) == Core::PipelineStage::TransferStage)
				flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;

			if ((stage & Core::PipelineStage::ComputeShader) == Core::PipelineStage::ComputeShader)
				flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

			if ((stage & Core::PipelineStage::FragmentShader) == Core::PipelineStage::FragmentShader)
				flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

			if ((stage & Core::PipelineStage::VertexShader) == Core::PipelineStage::VertexShader)
				flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;

			if ((stage & Core::PipelineStage::ColorAttachment) == Core::PipelineStage::ColorAttachment)
				flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

			if ((stage & Core::PipelineStage::EarlyFragmentTest) == Core::PipelineStage::EarlyFragmentTest)
				flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

			if ((stage & Core::PipelineStage::LateFragmentTest) == Core::PipelineStage::LateFragmentTest)
				flags |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

			if ((stage & Core::PipelineStage::AllGraphics) == Core::PipelineStage::AllGraphics)
				flags |= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;

			if ((stage & Core::PipelineStage::AllCommands) == Core::PipelineStage::AllCommands)
				flags |= VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

			return flags;
		}
	};
}