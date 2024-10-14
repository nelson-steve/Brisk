#include "Application.hpp"


namespace Brisk 
{
	std::shared_ptr<Window> Application::CreateApplication() {
		m_Window = Window::Create(1280, 720);

		m_Adapter = GpuAdapter::Create();
		m_Adapter->Init();

		m_Renderer = Renderer::Create();

		m_Renderer->Init(); 

		return nullptr;
	}

	void Application::Update(float deltaTime) {

	}

	void Application::Close() {
		
	}
}