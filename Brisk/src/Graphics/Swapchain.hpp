#pragma once

#include "Engine/WindowBase.hpp"

namespace Brisk 
{
	struct SwapchainExtent2D {
		uint32_t Width;
		uint32_t Height;
	};

	class Swapchain {
	public:
		enum Mode {
			DOUBLE_BUFFERING = 2U,
			TRIPLE_BUFFERING = 3U,
		};
	public:
		Swapchain(WindowBase* window, uint32_t width, uint32_t height);

		virtual void Create(Mode mode = Mode::DOUBLE_BUFFERING) = 0;
		virtual void Release() = 0;
		//void Resize();

		Swapchain() = delete;
		Swapchain(const Swapchain&) = delete;
		Swapchain(Swapchain&&) = delete;
		Swapchain& operator=(const Swapchain&) = delete;
		Swapchain& operator=(const Swapchain&&) = delete;

		uint32_t GetImageCount() const { return m_ImageCount; }

		virtual uint32_t GetExtentWidth() const = 0;
		virtual uint32_t GetExtentHeight() const = 0;
	protected:
		WindowBase* m_Window;
		uint32_t m_ImageCount;
	private:
		Mode m_SwapchainMode;
		SwapchainExtent2D m_Extent;
	};
}