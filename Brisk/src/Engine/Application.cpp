// INCLUDES
#include "Application.hpp"
#include "Scene.hpp"
#include "Core/Core.hpp"
#include "MeshAsset.hpp"
#include "Events/Event.hpp"
#include <Graphics/Vulkan/GpuAdapterVulkan.hpp>
#include <random>
#include "Component.hpp"
//------------------------
namespace Brisk
{
	ImGuiLayer* Application::m_ImGuiLayer;
	std::shared_ptr<GpuAdapter> Application::m_Adapter;
	std::shared_ptr<Window> Application::m_Window;
	std::shared_ptr<Renderer> Application::m_Renderer;
	std::shared_ptr<Camera> Application::m_EditorCamera;

	void Application::GenerateRandomLights(uint32_t count, float range) {
		std::random_device rd;
		std::mt19937 rng(rd());

		std::uniform_real_distribution<float> posDist(-range, range);
		std::uniform_real_distribution<float> radiusDist(10.0f, 50.0f); // light radius
		std::uniform_real_distribution<float> colorDist(0.5f, 1.0f);  // bright colors
		std::uniform_real_distribution<float> intensityDist(1.0f, 5.0f); // intensity

		for (uint32_t i = 0; i < count; ++i) {
			glm::vec3 pos = glm::vec3(posDist(rng), posDist(rng), posDist(rng));
			float radius = radiusDist(rng);

			glm::vec3 color = glm::vec3(colorDist(rng), colorDist(rng), colorDist(rng));
			float intensity = intensityDist(rng);

			Entity lightEntity = m_SceneManager->pActiveScene->CreateEntity("Light");
			PointLightComponent& lc = lightEntity.AddComponent<PointLightComponent>();

			lc.Position = glm::vec3(pos);
			lc.Color = glm::vec3(color);
			lc.Intensity = intensity;
			lc.Radius = radius;
		}
	}

	Application::Application(std::string name) {
		m_Window = Window::Create(1920, 1080);
		m_Window->SetEventCallBack(BIND_EVENT_FN(Application::OnEvent));

		m_Adapter = GpuAdapter::Create();
		m_Adapter->Init();

		m_EditorCamera = std::make_shared<Camera>((GLFWwindow*)m_Window->GetWindowHandle());

		m_SceneManager = std::make_unique<SceneManager>();
		m_SceneManager->Init();

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

		Entity lightEntity = m_SceneManager->pActiveScene->CreateEntity("Sun Light");
		DirectionalLightComponent& lc = lightEntity.AddComponent<DirectionalLightComponent>();
		lc.Direction = glm::vec3(0.0f, -1.0f, 0.0f);

		GenerateRandomLights(MAX_LIGHTS, 400);

		m_Renderer = Renderer::Create();
		m_Renderer->Init();

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

		std::shared_ptr<MeshAsset> asset1 = m_AssetManager->LoadAsset<MeshAsset>(paths[2], false);

		Entity entity = m_SceneManager->pActiveScene->CreateEntity("Flight Helmet");
		for (auto& node : asset1->m_Nodes)
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

	void Application::Run() {
		auto currentTime = std::chrono::high_resolution_clock::now();
		while (!ShouldClose()) {
			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			for (Layer* layer : m_LayerStack)
				layer->OnUpdate(frameTime);

			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();

			m_ImGuiLayer->End();

			m_Renderer->RenderScene(frameTime);
			m_EditorCamera->OnUpdate(frameTime);
			m_Window->ProcessEvents();
		}
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

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
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
