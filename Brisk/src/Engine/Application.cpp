#include "Application.hpp"

namespace Brisk 
{
	std::shared_ptr<Window> Application::CreateApplication() {
		std::unique_ptr<Renderer> m_Renderer = Renderer::Create();
		std::unique_ptr<Window> m_Window = Window::Create(1280, 720);

		m_Renderer->Init();
	}

	void Application::Update(float deltaTime) {

	}

	void Application::Close() {
		
	}
}