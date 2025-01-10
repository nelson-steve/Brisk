#pragma once

// INCLUDES
#include "Engine/Window.hpp"
//--------------------

struct GLFWwindow;

namespace Brisk {

	class WindowsWindow : public Window {
	public:
		WindowsWindow(int width, int height);
		virtual void* GetWindowHandle() override { return m_Window; }
		/// <summary>
		/// Cleanly destroys the window
		/// </summary>
		virtual void DestroyWindow() override;

		/// <summary>
		/// Queries if the window has been closed
		/// </summary>
		/// <returns></returns>
		virtual bool WindowShouldClose() override;
		/// <summary>
		/// Queries if the window has been resized
		/// </summary>
		/// <param name="resized"></param>
		virtual inline bool IsWindowResized() override 
		{
			if (m_HasWindowResized) {
				m_HasWindowResized = false;
				return true;
			}
			return false; 
		}

		/// <summary>
		/// Processes all events fired
		/// </summary>
		virtual void ProcessEvents() override;

		/// <summary>
		/// Setter: Width of window
		/// </summary>
		/// <param name="width"></param>
		virtual inline void SetWidth(uint32_t width) override { m_Width = width; };

		/// <summary>
		/// Setter Width of window
		/// </summary>
		/// <param name="height"></param>
		virtual inline void SetHeight(uint32_t height) override { m_Height = height; };

		/// <summary>
		/// Setter: Width of window
		/// </summary>
		/// <param name="width"></param>
		virtual inline uint32_t GetWidth() const override { return m_Width; }

		/// <summary>
		/// Setter Width of window
		/// </summary>
		/// <param name="height"></param>
		virtual inline uint32_t GetHeight() const override { return m_Height; }

		/// <summary>
		/// Stores the status of window being resized
		/// </summary>
		/// <param name="resized"></param>
		virtual inline void WindowResized(bool resized) override { m_HasWindowResized = resized; }

		/// <summary>
		/// Setting the function that will be process every event
		/// </summary>
		/// <param name="callback"></param>
		virtual inline void SetEventCallBack(const EventCallBackFn& callback) override { m_Data.EventCallBack = callback; }
	private:
		GLFWwindow* m_Window = nullptr;

		struct WindowData
		{
			std::string Title;
			uint32_t Width, Height;
			bool VSync;

			int Focus;

			EventCallBackFn EventCallBack;
		};

		WindowData m_Data;
	};
}