#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>

namespace Brisk {
	/// <summary>
	/// Cross platform window class with GLFW
	/// </summary>
	class Window {
	public:
		/// <summary>
		/// Static function to create and retrieve the newly created window
		/// </summary>
		/// <param name="width"></param>
		/// <param name="height"></param>
		/// <returns></returns>
		static Window* Create(int width, int height);
		
		/// <summary>
		/// Cleanly destroys the window
		/// </summary>
		void DestroyWindow();

		/// <summary>
		/// Queries if the window has been closed
		/// </summary>
		/// <returns></returns>
		
		bool WindowShouldClose();
		/// <summary>
		/// Queries if the window has been resized
		/// </summary>
		/// <param name="resized"></param>
		inline bool IsWindowResized() const { return m_HasWindowResized; }
		
		/// <summary>
		/// Processes all events fired
		/// </summary>
		void PollEvents();

		/// <summary>
		/// Setter: Width of window
		/// </summary>
		/// <param name="width"></param>
		inline void SetWidth(uint32_t width) { m_Width = width; }

		/// <summary>
		/// Setter Width of window
		/// </summary>
		/// <param name="height"></param>
		inline void SetHeight(uint32_t height) { m_Height = height; }

		/// <summary>
		/// Getter for window handle
		/// </summary>
		/// <returns></returns>
		GLFWwindow* window() const 
		{
#if _DEBUG
			if (m_Window == nullptr)
				std::cout << "ERROR: Window is null - did you forget to call 'Window::Create()' to retrieve a window";
#endif
			return m_Window; 
		}

		/// <summary>
		/// Stores the status of window being resized
		/// </summary>
		/// <param name="resized"></param>
		inline void WindowResized(bool resized) { m_HasWindowResized = resized; }

	private:
		/// <summary>
		/// Constructor to create the window
		/// </summary>
		Window(int width, int height);
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
			Window* window = Window::Create(width, height);
			return window;
		}
	};
}