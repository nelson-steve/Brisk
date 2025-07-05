// INCLUDES
#include "Application.hpp"
#include "Scene.hpp"
#include "Core/Core.hpp"
#include "MeshAsset.hpp"
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
			/* 4 */"../Data/Models/gltf_models/FlightHelmet/glTF/FlightHelmet.gltf",
			/* 5 */"../Data/Models/revolver/revolver.gltf",
			/* 6 */"../Data/Models/cerberus/cerberus.gltf",
			/* 7 */"../Data/Models/gltf_models/BoomBox/glTF/BoomBox.gltf",
		};
		auto start = std::chrono::high_resolution_clock::now();

		std::shared_ptr<MeshAsset> asset1 = m_AssetManager->LoadAsset<MeshAsset>(paths[4], false);

		Entity entity = m_SceneManager->pActiveScene->CreateEntity("Flight Helmet");
		for (auto& node: asset1->m_Nodes)
			AddMeshToScene(node, entity, asset1);
	}

	void Application::AddMeshToScene(MeshAsset::Node* node, std::optional<Entity> parent, std::shared_ptr<MeshAsset> meshAsset) {
		Entity entity = m_SceneManager->pActiveScene->CreateEntity(node->name);

		if (node->meshIndex != UINT32_MAX) {
			MeshComponent& mc = entity.AddComponent<MeshComponent>();
			mc.p_Mesh = meshAsset;
			mc.p_SubMeshIndex = node->meshIndex;
			mc.p_Name = node->name;
		}

		if (parent.has_value()) {
			entity.AddComponent<ParentComponent>().parent = parent.value();
			if(parent.value().HasComponent<ChildrenComponent>())
				parent.value().GetComponent<ChildrenComponent>().children.push_back(entity);
			else
				parent.value().AddComponent<ChildrenComponent>().children.push_back(entity);
		}

		for (auto n : node->children)
			AddMeshToScene(n, entity, meshAsset);
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
		m_Adapter->WaitIdle();
		m_Adapter->ReleasePools();
		m_EditorCamera->Release();
		m_Window->DestroyWindow();
		m_AssetManager->ClearAssets();
		m_Renderer->Release();
		m_Adapter->Release();
	}
}
