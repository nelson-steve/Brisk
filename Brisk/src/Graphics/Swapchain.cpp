#include "Swapchain.hpp"

namespace Brisk {
	Swapchain::Swapchain(WindowBase* window, uint32_t width, uint32_t height) {
		m_Extent.Width = width;
		m_Extent.Height = height;
		m_Window = window;
	}
}