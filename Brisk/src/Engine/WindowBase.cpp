#include "WindowBase.hpp"
#include "Engine/Engine.hpp"
#include "Core/Log.hpp"
#include "Platform/Windows/WindowsWindow.hpp"

namespace Brisk {
	[[nodiscard]] WindowBase* WindowBase::Create(int width, int height) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
		WindowBase* window = new WindowsWindow(width, height);
		return window;
#else
		BRISK_APP_ERROR("ERROR: Only Windows is supported currently");
#endif
	}
}