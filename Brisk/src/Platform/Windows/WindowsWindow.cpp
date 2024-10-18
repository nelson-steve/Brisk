#include "WindowsWindow.hpp"
#include "Core/Log.hpp"
#include "Engine/Engine.hpp"
#include "Editor/Editor.hpp"

#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>

namespace Brisk {
	void FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto win = reinterpret_cast<WindowsWindow*>(glfwGetWindowUserPointer(window));
		win->SetWidth(width);
		win->SetHeight(height);
		win->WindowResized(true);
	}

	
	//float lastX = 0.0f;
	//float lastY = 0.0f;
	//bool firstMouse = true;
	//void mouseCallback(GLFWwindow* window, double xposIn, double yposIn)
	//{
	//	float xpos = static_cast<float>(xposIn);
	//	float ypos = static_cast<float>(yposIn);

	//	if (firstMouse)
	//	{
	//		lastX = xpos;
	//		lastY = ypos;
	//		firstMouse = false;
	//	}

	//	float xoffset = xpos - lastX;
	//	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	//	lastX = xpos;
	//	lastY = ypos;

	//	Engine::s_Camera->SetMouseOffset(xoffset, yoffset);
	//	Engine::s_Camera->MouseMoved();
	//}

	//void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
	//{
	//	Engine::s_Camera->OnMouseScroll(yoffset);
	//}


	WindowsWindow::WindowsWindow(int width, int height) {
		m_Width = width;
		m_Height = height;
		int result = glfwInit();
		if (result != GLFW_TRUE) {
			BRISK_APP_FATAL("Failed to intitialize GLFW");
		}
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		m_Window = glfwCreateWindow(m_Width, m_Height, Engine::s_EngineInfo.EngineName.c_str(), nullptr, nullptr);
		if (m_Window == nullptr) {
			BRISK_APP_FATAL("Could not create window of size width/height: " + std::to_string(m_Width) + "/" + std::to_string(m_Height));
		}

		glfwSetWindowUserPointer(m_Window, this);
		glfwSetFramebufferSizeCallback(m_Window, FramebufferResizeCallback);
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