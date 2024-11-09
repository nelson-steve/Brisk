#pragma once

#include "Graphics/Swapchain.hpp"
#include "Graphics/Vulkan/GpuAdapterVulkan.hpp"

namespace Brisk 
{
	class SwapchainVulkan : public Swapchain {
	public:
		virtual void Create(Mode mode) override;
		virtual void Release() override;

		VkResult AquireNextImage(uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex);
		VkSwapchainKHR GetSwapchain() const { return m_Swapchain; }
		std::vector<VkImage> GetSwapchainImages() const { return m_SwapchainImages; }
		std::vector<VkImageView> GetSwapchainImageViews() const { return m_SwapchainImageViews; }
		VkImageView GetDepthImageView() const { return m_DepthImageView; }
		uint32_t GetExtentWidth() const { return m_extent.width; }
		uint32_t GetExtentHeight() const { return m_extent.height; }
		VkSurfaceFormatKHR GetFormat() const { return m_surface_format; }
		VkFormat GetDepthFormat() const { return m_DepthFormat; }
		VkExtent2D GetExtent() const { return m_extent; }
		VkPresentModeKHR GetPresentMode() const { return m_present_mode; }
	//private:
		SwapchainVulkan(std::shared_ptr<Window> window);
	private:
		VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
		std::vector<VkSemaphore> m_render_complete_semaphores;
		std::vector<VkSemaphore> m_present_complete_semaphores;
		std::vector<VkImage> m_SwapchainImages;
		std::vector<VkImageView> m_SwapchainImageViews;
		VkSurfaceFormatKHR m_surface_format;
		VkFormat m_DepthFormat;
		VkExtent2D m_extent;
		VkPresentModeKHR m_present_mode;

		// depth buffer
		VkImage m_DepthImage;
		VkDeviceMemory m_DepthImageMemory;
		VkImageView m_DepthImageView;

		friend class SwapchainFactory;
	};
}