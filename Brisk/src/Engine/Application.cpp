#include "Application.hpp"
#include "Scene.hpp"


namespace Brisk 
{
	void Application::CreateApplication() {
		m_Window = Window::Create(1280, 720);
		//m_EditorCamera = std::make_shared<Camera>(m_Window->GetWindowHandle());

		m_Adapter = GpuAdapter::Create();
		m_Adapter->Init();

		m_Renderer = Renderer::Create();
		std::shared_ptr<Scene> scene = std::make_shared<Scene>();
		m_Renderer->Init();
	}

	void Application::Update(float deltaTime) {
		m_Renderer->RenderScene(deltaTime);
		m_EditorCamera->OnUpdate(deltaTime);
		m_Window->ProcessEvents();
	}

	void Application::Close() {
		
	}
}