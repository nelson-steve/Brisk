// INCLUDES
#include "Application.hpp"
#include "Scene.hpp"
#include "Core/Core.hpp"
#include "Events/Event.hpp"
//------------------------
namespace Brisk
{
	void Application::CreateApplication() {
		m_Window = Window::Create(1280, 720);

		m_Window->SetEventCallBack(BIND_EVENT_FN(Application::OnEvent));

		m_Adapter = GpuAdapter::Create();
		m_Adapter->Init();

		m_Renderer = Renderer::Create();
		m_Renderer->Init();

		m_SceneManager = std::make_unique<SceneManager>();
		m_EditorCamera = std::make_shared<Camera>((GLFWwindow*)m_Window->GetWindowHandle());
		m_SceneManager->Init();
	}

	void Application::OnEvent(Event &event) {
		EventDispatcher dispatcher(event);

		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));
	}

	void Application::Update(float deltaTime) {
		m_Renderer->RenderScene(deltaTime);
		m_EditorCamera->OnUpdate(deltaTime);
		m_Window->ProcessEvents();
	}

	bool Application::OnWindowClose(WindowCloseEvent &e) {
		return false;
	}

	bool Application::OnWindowResize(WindowResizeEvent &e) {
		return false;
	}

	void Application::Close() {
	}
}
