#include "Window.hpp"
#include "Engine/Engine.hpp"
#include "Core/Log.hpp"
#include "Platform/Windows/WindowsWindow.hpp"

namespace Brisk 
{
	[[nodiscard]] std::unique_ptr<Window> Window::Create(uint32_t width, uint32_t height) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
		return std::make_unique<WindowsWindow>(width, height);
#else
		BRISK_APP_ERROR("ERROR: Only Windows is supported currently");
#endif
	}
}