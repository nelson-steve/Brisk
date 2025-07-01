// INCLUDES
#include "WindowsWindow.hpp"
#include "Core/Log.hpp"
#include "Engine/Engine.hpp"
#include "Editor/Editor.hpp"
#include "Engine/Events/ApplicationEvent.hpp"
#include "Engine/Events/MouseEvent.hpp"
#include "Engine/Events/KeyEvent.hpp"
//----------------------------------
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
//----------------

namespace Brisk 
{
	WindowsWindow::WindowsWindow(int width, int height) {
		m_Width = width;
		m_Height = height;
		int result = glfwInit();
		if (result != GLFW_TRUE) {
			BRISK_APP_ERROR("Failed to intitialize GLFW");
		}
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		//glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
		//glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
		m_Window = glfwCreateWindow((int)m_Width, (int)m_Height, Engine::s_EngineSettings.EngineName.c_str(), nullptr, nullptr);
		if (m_Window == nullptr) {
			BRISK_APP_ERROR("Could not create window of size width/height: " + std::to_string(m_Width) + "/" + std::to_string(m_Height));
		}

		glfwSetWindowUserPointer(m_Window, &m_Data);

		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				data.Width = width;
				data.Height = height;

				WindowResizeEvent event(width, height);
				//Brisk_Core_TRACE("{0}, {1}", width, height);
				data.EventCallBack(event);
			});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				WindowCloseEvent event;
				data.EventCallBack(event);
				std::cout << "Window close";
			});

		glfwSetWindowFocusCallback(m_Window, [](GLFWwindow* window, int focus)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				data.Focus = focus;

				switch (focus)
				{
					case 0:
					{
						WindowFocusEvent event(focus);
						data.EventCallBack(event);
					}
					case 1:
					{
						WindowLoseFocusEvent event(focus);
						data.EventCallBack(event);
					}
				}
			});

		//glfwSetWindowPosCallback(m_Window, [](GLFWwindow* window, int xPos, int yPos)
		//	{
		//		WindowsWindow& data = *(WindowsWindow*)glfwGetWindowUserPointer(window);
		//
		//		WindowPosChangeEvent event;
		//		data.EventCallBack(event);
		//	});

		//glfwSetWindowRefreshCallBack(m_Window, [](GLFWwindow* window)
		//	{
		//		WindowsWindow& data = *(WindowsWindow*)glfwGetWindowUserPointer(window);
		//
		//		WindowRefreshEvent event;
		//		data.EventCallBack(event);
		//	});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				BRISK_CORE_INFO("Mouse button {}", button);
				switch (action)
				{
					case GLFW_PRESS:
					{
						MouseButtonPressedEvent event(button);
						data.EventCallBack(event);
						break;
					}
					case GLFW_RELEASE:
					{
						MouseButtonReleasedEvent event(button);
						data.EventCallBack(event);
						break;
					}
				}

			});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				MouseScrolledEvent event((float)xOffset, (float)yOffset);
				data.EventCallBack(event);
			});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				MouseMovedEvent event((float)xPos, (float)yPos);
				data.EventCallBack(event);
			});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				switch (action)
				{
					case GLFW_PRESS:
					{
						KeyPressedEvent event(key, 0);
						data.EventCallBack(event);
						break;
					}
					case GLFW_RELEASE:
					{
						KeyReleasedEvent event(key);
						data.EventCallBack(event);
						break;
					}
					case GLFW_REPEAT:
					{
						KeyPressedEvent event(key, 1);
						data.EventCallBack(event);
						break;
					}
				}
			});
	}

	void* WindowsWindow::GetHWNDWindowHandle() {
		 return glfwGetWin32Window(m_Window);
	}

	void WindowsWindow::DestroyWindow() {
		glfwDestroyWindow(m_Window);
	}

	bool WindowsWindow::WindowShouldClose()
	{
		return glfwWindowShouldClose(m_Window);
	}
	void WindowsWindow::ProcessEvents()
	{
		glfwPollEvents();
	}
}