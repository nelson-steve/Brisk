#include "Window.hpp"
#include "Engine/Engine.hpp"
#include "Core/Log.hpp"

namespace Brisk {
	void FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		win->WindowResized(true);
	}

	[[nodiscard]] Window* Window::Create(int width, int height) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
		Window* window = new Window();
		return window;
#else
		ERROR_FATAL("ERROR: Only Windows is supported currently");
#endif
	}

	Window::Window() {
		int result = glfwInit();
		if (result != GLFW_TRUE) {
			//ERROR_FATAL("Failed to intitialize GLFW");
		}
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		m_Window = glfwCreateWindow(m_Width, m_Height, Engine::s_EngineInfo.EngineName.c_str(), nullptr, nullptr);
		if (m_Window == nullptr) {
			//ERROR_FATAL("Could not create window of size width/height: " + std::to_string(m_Width) +"/" + std::to_string(m_Height));
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