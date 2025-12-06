#include "pch.hpp"
#include "SwapchainVulkan.hpp"
#include "Defines.h"
#include "Engine/Engine.hpp"
#include "UtilitiesVulkan.hpp"
#include "Engine/Application.hpp"
#include "FenceVulkan.hpp"
#include "SemaphoreVulkan.hpp"
#include "CommandBufferVulkan.hpp"
#include "TextureVulkan.hpp"

namespace Brisk 
{
	void SwapchainVulkan::Release() {
		for (auto imageView : m_SwapchainImageViews) {
			vkDestroyImageView(Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), imageView, nullptr);
		}
		vkDestroySwapchainKHR(Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), m_Swapchain, nullptr);
	}

	void SwapchainVulkan::Create(Mode mode) {
		// TODO: Dont use hardcoded values
		VkFormat m_format = VK_FORMAT_B8G8R8A8_UNORM;
		VkColorSpaceKHR m_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

		int imageCount = static_cast<uint32_t>(mode);

		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetPhysicalDevice(), std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetSurface()->GetRef(), &surfaceCapabilities);
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetPhysicalDevice(), Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetSurface()->GetRef(), &formatCount, nullptr);
		std::vector<VkSurfaceFormatKHR> supportedFormats;
		if (formatCount != 0) {
			supportedFormats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetPhysicalDevice(), std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetSurface()->GetRef(), &formatCount, supportedFormats.data());
		}
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetPhysicalDevice(), Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetSurface()->GetRef(), &presentModeCount, nullptr);

		std::vector<VkPresentModeKHR> presentModes;
		if (presentModeCount != 0) {
			presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetPhysicalDevice(), Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetSurface()->GetRef(), &presentModeCount, presentModes.data());
		}

		bool format_found = false;
		for (const auto& availableFormat : supportedFormats) {
			if (availableFormat.format == m_format && availableFormat.colorSpace == m_color_space) {
				m_surface_format = availableFormat;
				format_found = true;
			}
		}
		if (!format_found)
		{
			BRISK_CORE_WARN("Format specified is not supported using default format and color space");
			m_surface_format.format = VK_FORMAT_B8G8R8A8_SRGB;
			m_surface_format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		}

		if (imageCount > surfaceCapabilities.maxImageCount && surfaceCapabilities.maxImageCount > 0)
			imageCount = surfaceCapabilities.maxImageCount;
		VkSwapchainCreateInfoKHR swapChainCreateInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
		swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapChainCreateInfo.surface = std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetSurface()->GetRef();
		swapChainCreateInfo.minImageCount = imageCount;
		swapChainCreateInfo.imageFormat = m_surface_format.format;
		swapChainCreateInfo.imageColorSpace = m_surface_format.colorSpace;
		swapChainCreateInfo.imageExtent = surfaceCapabilities.currentExtent;
		swapChainCreateInfo.imageArrayLayers = 1;
		swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		m_extent = surfaceCapabilities.currentExtent;

		std::cout << "Swapchain image count: " << imageCount << std::endl;
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetPhysicalDevice(), &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetPhysicalDevice(), &queueFamilyCount, queueFamilies.data());

		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

		swapChainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
		swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		bool vSync = false;
		swapChainCreateInfo.presentMode = vSync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;
		swapChainCreateInfo.clipped = VK_TRUE;
		swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
		VK_LOG(vkCreateSwapchainKHR(Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &swapChainCreateInfo, nullptr, &m_Swapchain));

		vkGetSwapchainImagesKHR(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), m_Swapchain, &m_ImageCount, nullptr);
		m_SwapchainImages.resize(m_ImageCount);
		vkGetSwapchainImagesKHR(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), m_Swapchain, &m_ImageCount, m_SwapchainImages.data());

		m_SwapchainImageViews.resize(m_SwapchainImages.size());
		
		for (size_t i = 0; i < m_SwapchainImages.size(); i++) {
			VkImageViewCreateInfo image_views_create_info{};
			image_views_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			image_views_create_info.image = m_SwapchainImages[i];
			image_views_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			image_views_create_info.format = m_format;
			image_views_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_views_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_views_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_views_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_views_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			image_views_create_info.subresourceRange.baseMipLevel = 0;
			image_views_create_info.subresourceRange.levelCount = 1;
			image_views_create_info.subresourceRange.baseArrayLayer = 0;
			image_views_create_info.subresourceRange.layerCount = 1;
		
			if (vkCreateImageView(Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &image_views_create_info, nullptr, &m_SwapchainImageViews[i]) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create Swapchain Image Views!");
			}
		}
	}

	bool SwapchainVulkan::AcquireNextImage(uint64_t timeout, std::shared_ptr<Semaphore> semaphore, std::shared_ptr<Fence> fence, uint32_t* pImageIndex) {
		VkResult result = vkAcquireNextImageKHR(
			Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(),
			m_Swapchain,
			timeout,
			semaphore ? std::static_pointer_cast<SemaphoreVulkan>(semaphore)->Get() : VK_NULL_HANDLE,
			fence ? std::static_pointer_cast<FenceVulkan>(fence)->Get() : VK_NULL_HANDLE,
			pImageIndex);
		switch (result) {
		case VK_SUCCESS:
			return true;
		case VK_SUBOPTIMAL_KHR:
			return false;
			BRISK_CORE_ERROR("resized");
		case VK_ERROR_OUT_OF_DATE_KHR:
			return false;
			BRISK_CORE_ERROR("swapchain out of date");
		case VK_TIMEOUT:
			BRISK_CORE_ERROR("swapchain timeout");
			break;
		case VK_NOT_READY:
			BRISK_CORE_ERROR("not ready");
			break;
		case VK_ERROR_DEVICE_LOST:
			BRISK_CORE_ERROR("swapchain device lost");
			break;
		default:
			// Unexpected error
			throw std::runtime_error("Failed to acquire next image: " + std::to_string(result));
		}
		return false;
	}

	void SwapchainVulkan::TransitionCurrentImage(std::shared_ptr<CommandBuffer> cmd, Texture::ImageBarrierParams params, int imageIndex) {
		VkPipelineStageFlags srcFlag = UtilitiesVulkan::PipelineStageToVkPipelineStageFlags(params.srcStage);
		VkPipelineStageFlags dstFlag = UtilitiesVulkan::PipelineStageToVkPipelineStageFlags(params.dstStage);
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = UtilitiesVulkan::ImageLayoutToVkImageLayout(params.oldLayout);
		barrier.newLayout = UtilitiesVulkan::ImageLayoutToVkImageLayout(params.newLayout);
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = m_SwapchainImages[imageIndex];
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.srcAccessMask = UtilitiesVulkan::AccessTypeToVkAccessFlags(params.srcAccess);
		barrier.dstAccessMask = UtilitiesVulkan::AccessTypeToVkAccessFlags(params.dstAccess);
		vkCmdPipelineBarrier(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), srcFlag, dstFlag, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	}

	void SwapchainVulkan::Blit(std::shared_ptr<CommandBuffer> cmd, std::shared_ptr<Texture> image, int imageIndex) {
		VkImageBlit blit{};
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = 0;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { (int)image->GetWidth(), (int)image->GetHeight(), 1 };

		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = 0;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { (int)m_extent.width, (int)m_extent.height, 1 };

		// Blit from lighting output to swapchain image
		vkCmdBlitImage(
			std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(),
			std::static_pointer_cast<TextureVulkan>(image)->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			m_SwapchainImages[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_NEAREST // or VK_FILTER_LINEAR if you want smooth scaling
		);

	}
}