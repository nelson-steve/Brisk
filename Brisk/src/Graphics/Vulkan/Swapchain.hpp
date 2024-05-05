#pragma once

#include "GraphicsDeviceVulkan.hpp"
#include "Engine/Window.hpp"

namespace Brisk {
	class Swapchain {
	public:
		Swapchain(Window* window);

		void Create();
		void Release();
		//void Resize();

		Swapchain() = delete;
		Swapchain(const Swapchain&) = delete;
		Swapchain(Swapchain&&) = delete;
		Swapchain& operator=(const Swapchain&) = delete;
		Swapchain& operator=(const Swapchain&&) = delete;

		VkSurfaceKHR GetSurface() const { return m_Surface; }
		VkSwapchainKHR GetSwapchain() const { return m_Swapchain; }
		std::vector<VkImage> GetSwapchainImages() const { return m_swapchain_images; }
		std::vector<VkImageView> GetSwapchainImageViews() const { return m_swapchain_image_views; }
		uint32_t GetExtentWidth() const { return m_extent.width; }
		uint32_t GetExtentHeight() const { return m_extent.height; }
		VkExtent2D GetExtent() const { return m_extent; }
		uint32_t GetImageCount() const { return m_image_count; }
		VkPresentModeKHR GetPresentMode() const { return m_present_mode; }
	private:
		VkSwapchainKHR m_Swapchain;
		VkSurfaceKHR m_Surface;
		Window* m_Window;
		std::vector<VkSemaphore> m_render_complete_semaphores;
		std::vector<VkSemaphore> m_present_complete_semaphores;
		std::vector<VkImage> m_swapchain_images;
		std::vector<VkImageView> m_swapchain_image_views;
		VkSurfaceFormatKHR m_surface_format;
		uint32_t m_image_count;
		VkExtent2D m_extent;
		VkPresentModeKHR m_present_mode;
	};
}