#include "Window.hpp"
#include "Engine/Engine.hpp"
#include "Core/Log.hpp"

namespace Brisk {
	void FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto w = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		w->WindowResized(true);
	}

	Window::Window() {
		int result = glfwInit();
		if (result != GLFW_TRUE) {
			ERROR_FATAL("Failed to intitialize GLFW");
		}
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		m_Window = glfwCreateWindow(m_Width, m_Height, Engine::s_EngineInfo.EngineName.c_str(), nullptr, nullptr);
		if (m_Window == nullptr) {
			ERROR_FATAL("Could not create window");
		}

		glfwSetWindowUserPointer(m_Window, this);
		glfwSetFramebufferSizeCallback(m_Window, FramebufferResizeCallback);
	}

	void Window::DestroyWindow() {
		glfwDestroyWindow(m_Window);
	}

	bool Window::WindowShouldClose()
	{
		return glfwWindowShouldClose(m_Window);
	}
	void Window::PollEvents()
	{
		glfwPollEvents();
	}
}