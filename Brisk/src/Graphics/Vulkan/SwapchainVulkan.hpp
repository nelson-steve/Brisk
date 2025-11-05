#pragma once

#include "Engine/Renderer/Swapchain.hpp"
#include "Graphics/Vulkan/GpuAdapterVulkan.hpp"

namespace Brisk 
{
	class SwapchainVulkan : public Swapchain {
	public:
		virtual void Create(Mode mode) override;
		virtual void Release() override;

		virtual void Resize() override {
			std::cout << "swapchain resized";
			Release();
			Create(Mode::DOUBLE_BUFFERING);
		};

		virtual bool AcquireNextImage(uint64_t timeout, std::shared_ptr<Semaphore> semaphore, std::shared_ptr<Fence> fence, uint32_t* pImageIndex) override;
		virtual void TransitionCurrentImage(std::shared_ptr<CommandBuffer> cmd, Texture::ImageBarrierParams params, int imageIndex) override;
		virtual void Blit(std::shared_ptr<CommandBuffer> cmd, std::shared_ptr<Texture> image, int imageIndex) override;

		VkSwapchainKHR GetSwapchain() const { return m_Swapchain; }
		std::vector<VkImage> GetSwapchainImages() const { return m_SwapchainImages; }
		std::vector<VkImageView> GetSwapchainImageViews() const { return m_SwapchainImageViews; }
		virtual uint32_t GetExtentWidth() const override { return m_extent.width; }
		virtual uint32_t GetExtentHeight() const override { return m_extent.height; }
		VkSurfaceFormatKHR GetFormat() const { return m_surface_format; }
		VkFormat GetDepthFormat() const { return m_DepthFormat; }
		VkExtent2D GetExtent() const { return m_extent; }
		VkPresentModeKHR GetPresentMode() const { return m_present_mode; }
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

		friend class SwapchainFactory;
	};
}