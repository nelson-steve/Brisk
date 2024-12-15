#include "Application.hpp"
#include "Scene.hpp"
#include "Core/Core.hpp"

namespace Brisk 
{
	void Application::CreateApplication() {
		m_Window = Window::Create(1280, 720);
		m_EditorCamera = std::make_shared<Camera>(m_Window->GetWindowHandle());

		m_Window->SetEventCallBack(BIND_EVENT_FN(Application::OnEvent));

		m_Adapter = GpuAdapter::Create();
		m_Adapter->Init();

		m_Renderer = Renderer::Create();
		std::shared_ptr<Scene> scene = std::make_shared<Scene>();
		
		m_Renderer->Init(scene);
	}

	void Application::OnEvent(Event& event) {
		EventDispatcher dispatcher(event);

		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));
	}

	void Application::Update(float deltaTime) {
		m_Renderer->RenderScene(deltaTime);
		m_EditorCamera->OnUpdate(deltaTime);
		m_Window->ProcessEvents();
	}

	bool Application::OnWindowClose(WindowCloseEvent& e) {
		return false;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e) {
		return false;
	}

	void Application::Close() {
		
	}
}
