#pragma once

#include "Engine/WindowBase.hpp"

namespace Brisk {
	struct SwapchainExtent2D {
		uint32_t Width;
		uint32_t Height;
	};

	class Swapchain {
	public:
		Swapchain(WindowBase* window, uint32_t width, uint32_t height);

		virtual void Create() = 0;
		virtual void Release() = 0;
		//void Resize();

		Swapchain() = delete;
		Swapchain(const Swapchain&) = delete;
		Swapchain(Swapchain&&) = delete;
		Swapchain& operator=(const Swapchain&) = delete;
		Swapchain& operator=(const Swapchain&&) = delete;

		virtual uint32_t GetExtentWidth() const = 0;
		virtual uint32_t GetExtentHeight() const = 0;
	protected:
		WindowBase* m_Window;
		uint32_t m_ImageCount;
	private:
		SwapchainExtent2D m_Extent;
		//VkPresentModeKHR m_present_mode; // TODO: Define your own Present modes
	};
}