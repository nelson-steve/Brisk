#pragma once

#include "Graphics/Swapchain.hpp"
#include "Graphics/Vulkan/GraphicsDeviceVulkan.hpp"
#include "FramebufferVulkan.hpp"

namespace Brisk {
	class SwapchainVulkan : public Swapchain {
	public:
		SwapchainVulkan(WindowBase* window);

		virtual void Create() override;
		virtual void Release() override;
		//void Resize();

		void CreateFramebuffer();

		const FramebufferVulkan* GetFramebuffer() const { return m_Framebuffer; }

		VkSurfaceKHR GetSurface() const { return m_Surface; }
		VkSwapchainKHR GetSwapchain() const { return m_Swapchain; }
		std::vector<VkImage> GetSwapchainImages() const { return m_SwapchainImages; }
		std::vector<VkImageView> GetSwapchainImageViews() const { return m_SwapchainImageViews; }
		uint32_t GetExtentWidth() const { return m_extent.width; }
		uint32_t GetExtentHeight() const { return m_extent.height; }
		VkExtent2D GetExtent() const { return m_extent; }
		uint32_t GetImageCount() const { return m_ImageCount; }
		VkPresentModeKHR GetPresentMode() const { return m_present_mode; }
	private:
		FramebufferVulkan* m_Framebuffer;
		VkSwapchainKHR m_Swapchain;
		VkSurfaceKHR m_Surface;
		std::vector<VkSemaphore> m_render_complete_semaphores;
		std::vector<VkSemaphore> m_present_complete_semaphores;
		std::vector<VkImage> m_SwapchainImages;
		std::vector<VkImageView> m_SwapchainImageViews;
		VkSurfaceFormatKHR m_surface_format;
		VkExtent2D m_extent;
		VkPresentModeKHR m_present_mode;
	};
}