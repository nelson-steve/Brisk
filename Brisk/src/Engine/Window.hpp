#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Brisk {
	class Window {
	public:
		Window();
		void DestroyWindow();

		bool WindowShouldClose();
		inline void WindowResized(bool resized) { m_HasWindowResized = resized; }
		inline bool IsWindowResized() const { return m_HasWindowResized; }
		void PollEvents();

		inline void SetWidth(uint32_t width) { m_Width = width; }
		inline void SetHeight(uint32_t height) { m_Height = height; }

		GLFWwindow* window() const { return m_Window; }
	private:
		GLFWwindow* m_Window = nullptr;
		uint32_t m_Width = 1280;
		uint32_t m_Height = 720;
		bool m_HasWindowResized = false;
	};

	/// <summary>
	/// Class to create windows for different platforms
	/// </summary>
	static class WindowCreator {
	public:
		/// <summary>
		/// Create window for Windows
		/// </summary>
		/// <param name="width"></param>
		/// <param name="height"></param>
		/// <returns></returns>
		[[nodiscard]] static Window* CreateWindowsWindow(int width, int height) {
			Window* w = new Window();
			return w;
		}
	};
}