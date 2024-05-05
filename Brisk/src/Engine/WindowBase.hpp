#pragma once

#include <glfw3.h>

#include <iostream>

namespace Brisk {
	/// <summary>
	/// Base Window class
	/// </summary>
	class WindowBase {
	private:
		/// <summary>
		/// Static function to create and retrieve the newly created window
		/// </summary>
		/// <param name="width"></param>
		/// <param name="height"></param>
		/// <returns></returns>
		static WindowBase* Create(int width, int height);		
	public:
		virtual void* GetWindowHandle() = 0;
		/// <summary>
		/// Cleanly destroys the window
		/// </summary>
		virtual void DestroyWindow() = 0;

		/// <summary>
		/// Queries if the window has been closed
		/// </summary>
		/// <returns></returns>
		
		virtual bool WindowShouldClose() = 0;
		/// <summary>
		/// Queries if the window has been resized
		/// </summary>
		/// <param name="resized"></param>
		virtual inline bool IsWindowResized() = 0;
		
		/// <summary>
		/// Processes all events fired
		/// </summary>
		virtual void ProcessEvents() = 0;

		/// <summary>
		/// Setter: Width of window
		/// </summary>
		/// <param name="width"></param>
		virtual inline void SetWidth(uint32_t width) = 0;

		/// <summary>
		/// Setter Width of window
		/// </summary>
		/// <param name="height"></param>
		virtual inline void SetHeight(uint32_t height) = 0;

		/// <summary>
		/// Stores the status of window being resized
		/// </summary>
		/// <param name="resized"></param>
		virtual inline void WindowResized(bool resized) = 0;
	protected:
		uint32_t m_Width = 1280;
		uint32_t m_Height = 720;
		bool m_HasWindowResized = false;

		friend class WindowCreator;
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
		[[nodiscard]] static WindowBase* CreateWindowsWindow(int width, int height) {
			WindowBase* window = WindowBase::Create(width, height);
			return window;
		}
	};
}