// INCLUDES
#include "Application.hpp"
#include "Scene.hpp"
#include "Core/Core.hpp"
#include "Events/Event.hpp"
#include <Graphics/Vulkan/GpuAdapterVulkan.hpp>
#include "Component.hpp"
//------------------------
namespace Brisk
{
	void Application::CreateApplication() {
		m_Window = Window::Create(1920, 1080);
		m_Window->SetEventCallBack(BIND_EVENT_FN(Application::OnEvent));

		m_Adapter = GpuAdapter::Create();
		m_Adapter->Init();

		m_Renderer = Renderer::Create();
		m_Renderer->Init();

		m_SceneManager = std::make_unique<SceneManager>();
		m_SceneManager->Init();

		m_EditorCamera = std::make_shared<Camera>((GLFWwindow*)m_Window->GetWindowHandle());

		m_AssetManager = std::make_shared<AssetManager>();

		std::vector<std::string> paths = {
			/* 0 */"../Data/Models/Cube/Cube.gltf",
			/* 1 */"../Data/Models/revolver/revolver.gltf",
			/* 2 */"../Data/Models/gltf_models/Sponza/glTF/Sponza.gltf",
			/* 3 */"../Data/Models/damaged_helmet/DamagedHelmet.gltf",
			/* 4 */"../Data/Models/revolver/revolver.gltf",
			/* 5 */"../Data/Models/cerberus/cerberus.gltf",
			/* 6 */"../Data/Models/gltf_models/BoomBox/glTF/BoomBox.gltf",
		};
		auto start = std::chrono::high_resolution_clock::now();

		std::shared_ptr<MeshAsset> asset1 = m_AssetManager->LoadAsset<MeshAsset>(paths[3], false);

		m_SceneManager->pActiveScene->CreateEntity("name").AddComponent<MeshComponent>().p_Mesh = asset1;
	}

	void Application::OnEvent(Event &event) {
		EventDispatcher dispatcher(event);

		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));
		dispatcher.Dispatch<MouseMovedEvent>(BIND_EVENT_FN(Application::OnMouseMoved));
		dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(Application::OnMouseScrolled));
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

	bool Application::OnMouseMoved(MouseMovedEvent& e) {
		m_EditorCamera->MouseMoved();
		m_EditorCamera->SetMouseOffset(e.GetMouseX(), e.GetMouseY());
		return false;
	}

	bool Application::OnMouseScrolled(MouseScrolledEvent& e) {
		m_EditorCamera->MouseMoved();
		m_EditorCamera->OnMouseScroll(e.GetYOffset());
		return false;
	}

	void Application::Close() {
	}
}
