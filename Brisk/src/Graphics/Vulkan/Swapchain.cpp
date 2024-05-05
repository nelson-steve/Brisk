#include "Swapchain.hpp"
#include "Defines.h"
#include "Engine/Engine.hpp"

#include <glfw3.h>

namespace Brisk {
	Swapchain::Swapchain(Window* window) {
		m_Window = window;
	}

	void Swapchain::Release() {
		//for (auto imageView : m_swapchain_image_views)
		//	vkDestroyImageView(m_device->Device(), imageView, nullptr);
		vkDestroySwapchainKHR(Engine::s_PhysicalDevice->GetDevice(), m_Swapchain, nullptr);
	}

	void Swapchain::Create() {
		if (glfwCreateWindowSurface(Engine::m_GPUDeviceVulkan->GetInstance(), m_Window->window(), nullptr, &m_Surface) != VK_SUCCESS) {
			BRISK_CORE_ERROR("Failed to create window surface!");
		}

		Engine::s_PhysicalDevice = new PhysicalDevice();
		Engine::s_PhysicalDevice->Create(m_Surface);

		// TODO: Dont use hardcoded values
		VkFormat m_format = VK_FORMAT_B8G8R8A8_SRGB;
		VkColorSpaceKHR m_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Engine::s_PhysicalDevice->GetPhysicalDevice(), m_Surface, &surfaceCapabilities);
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(Engine::s_PhysicalDevice->GetPhysicalDevice(), m_Surface, &formatCount, nullptr);
		std::vector<VkSurfaceFormatKHR> supportedFormats;
		if (formatCount != 0) {
			supportedFormats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(Engine::s_PhysicalDevice->GetPhysicalDevice(), m_Surface, &formatCount, supportedFormats.data());
		}
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(Engine::s_PhysicalDevice->GetPhysicalDevice(), m_Surface, &presentModeCount, nullptr);

		std::vector<VkPresentModeKHR> presentModes;
		if (presentModeCount != 0) {
			presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(Engine::s_PhysicalDevice->GetPhysicalDevice(), m_Surface, &presentModeCount, presentModes.data());
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

		VkSwapchainCreateInfoKHR swapChainCreateInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
		swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapChainCreateInfo.surface = m_Surface;
		swapChainCreateInfo.minImageCount = surfaceCapabilities.minImageCount + 1;
		swapChainCreateInfo.imageFormat = m_surface_format.format;
		swapChainCreateInfo.imageColorSpace = m_surface_format.colorSpace;
		swapChainCreateInfo.imageExtent = surfaceCapabilities.currentExtent;
		swapChainCreateInfo.imageArrayLayers = 1;
		swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(Engine::s_PhysicalDevice->GetPhysicalDevice(), &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(Engine::s_PhysicalDevice->GetPhysicalDevice(), &queueFamilyCount, queueFamilies.data());

		uint32_t graphics = -1, presentation = -1;
		VkBool32 presentSupport = false;
		for (int i = 0; i < queueFamilies.size(); i++) {
			vkGetPhysicalDeviceSurfaceSupportKHR(Engine::s_PhysicalDevice->GetPhysicalDevice(), i, m_Surface, &presentSupport);
		}
		VkBool32 graphicsSupport = false;
		for (int i = 0; i < queueFamilies.size(); i++) {
			vkGetPhysicalDeviceSurfaceSupportKHR(Engine::s_PhysicalDevice->GetPhysicalDevice(), i, m_Surface, &graphicsSupport);
		}
		if (!presentSupport)
			BRISK_CORE_ERROR("GPU does not support presentation");
		if(!graphicsSupport)
			BRISK_CORE_ERROR("GPU does not support graphics");

		uint32_t queueFamilyIndices[] = { graphics, presentation };
		if (graphics != presentation) {
			swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapChainCreateInfo.queueFamilyIndexCount = 2;
			swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		swapChainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
		swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapChainCreateInfo.presentMode = m_present_mode;
		swapChainCreateInfo.clipped = VK_TRUE;
		swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
		VK_LOG(vkCreateSwapchainKHR(Engine::s_PhysicalDevice->GetDevice(), &swapChainCreateInfo, nullptr, &m_Swapchain),
			"Failed to create swapchain!");

		return;

		//vkGetSwapchainImagesKHR(m_device->Device(), m_swapchain, &m_image_count, nullptr);
		//m_swapchain_images.resize(m_image_count);
		//vkGetSwapchainImagesKHR(m_device->Device(), m_swapchain, &m_image_count, m_swapchain_images.data());

		// Create Image Views
		//m_swapchain_image_views.resize(m_swapchain_images.size());
		//
		//for (size_t i = 0; i < m_swapchain_images.size(); i++) {
		//	VkImageViewCreateInfo image_views_create_info{};
		//	image_views_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		//	image_views_create_info.image = m_swapchain_images[i];
		//	image_views_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		//	image_views_create_info.format = m_format;
		//	image_views_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		//	image_views_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		//	image_views_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		//	image_views_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		//	image_views_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		//	image_views_create_info.subresourceRange.baseMipLevel = 0;
		//	image_views_create_info.subresourceRange.levelCount = 1;
		//	image_views_create_info.subresourceRange.baseArrayLayer = 0;
		//	image_views_create_info.subresourceRange.layerCount = 1;
		//
		//	if (vkCreateImageView(m_device->Device(), &image_views_create_info, nullptr, &m_swapchain_image_views[i]) != VK_SUCCESS) {
		//		throw std::runtime_error("Failed to create Swapchain Image Views!");
		//	}
		//}
	}
}