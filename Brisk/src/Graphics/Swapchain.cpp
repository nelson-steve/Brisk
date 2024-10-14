#include "Swapchain.hpp"

namespace Brisk 
{
	Swapchain::Swapchain(std::shared_ptr<Window> window, uint32_t width, uint32_t height)
		:m_Extent{ width, height }, m_Window(window)
	{
	}
}