#include "Swapchain.hpp"

namespace Brisk {
	Swapchain::Swapchain(WindowBase* window, uint32_t width, uint32_t height)
		:m_Extent{ width, height }, m_Window(window)
	{
	}
}