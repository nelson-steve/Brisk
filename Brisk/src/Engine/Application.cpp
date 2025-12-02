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
	JobSystem Application::m_JobSystem;

	void GenerateRandomLights(uint32_t count, float range, float radiusMin, float radiusMax, float colorMin, float colorMax, float intensityMin, float intensityMax) {
		std::random_device rd;
		std::mt19937 rng(rd());

		std::uniform_real_distribution<float> posDist(-range, range);
		std::uniform_real_distribution<float> radiusDist(radiusMin, radiusMax); // light radius
		std::uniform_real_distribution<float> colorDist(colorMin, colorMax);  // bright colors
		std::uniform_real_distribution<float> intensityDist(intensityMin, intensityMax); // intensity

		for (uint32_t i = 0; i < count; ++i) {
			glm::vec3 pos = glm::vec3(posDist(rng), posDist(rng), posDist(rng));
			float radius = radiusDist(rng);

			glm::vec3 color = glm::vec3(colorDist(rng), colorDist(rng), colorDist(rng));
			float intensity = intensityDist(rng);

			Entity lightEntity = SceneManager::pActiveScene->CreateEntity("Light");
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
			/* 4 */"../Data/Models/gltf_models/DamagedHelmet/glTF/DamagedHelmet.gltf",
			/* 5 */"../Data/glTFModels/2.0/ABeautifulGame/glTF/ABeautifulGame.gltf",
			/* 6 */"../Data/glTFModels/2.0/ToyCar/glTF/ToyCar.gltf",
			/* 7 */"../Data/Models/gltf_models/FlightHelmet/glTF/FlightHelmet.gltf",
			/* 8 */"../Data/Models/cerberus/cerberus.gltf",
			/* 9 */"../Data/Models/lamborghini_temerario_gt3_2026/scene.gltf",
			/* 10 */"../Data/Models/Sketchfab_Scene/Sketchfab_Scene.gltf",
			/* 11 */"../Data/Models/beautiful_city/scene.gltf",
			/* 12 */"../Data/Models/futuristic_muscle_car_launch_control_ready/scene.gltf",
			/* 13 */"../Data/Models/mecha_ramen_high_poly/scene.gltf",
			/* 14 */"../Data/Models/modural_robot_mecha_chimera_dyan_high-poly_mesh/scene.gltf",
		};

		//SceneManager::pActiveScene->LoadGltfScene(paths[9]);

		Entity lightEntity = m_SceneManager->pActiveScene->CreateEntity("Sun Light");
		DirectionalLightComponent& lc = lightEntity.AddComponent<DirectionalLightComponent>();
		lc.Direction = glm::vec3(0.0f, -1.0f, 0.0f);

		uint32_t range = 10;
		float radiusMin = 0.8f;
		float radiusMax = 3.0f;
		float colorMin = 0.5f;
		float colorMax = 1.0f;
		float intensityMin = 3.0f;
		float intensityMax = 5.0f;
		GenerateRandomLights(MAX_LIGHTS, range, radiusMin, radiusMax, colorMin, colorMax, intensityMin, intensityMax);

		m_Renderer = Renderer::Create();
		m_Renderer->Init();

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
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
		float frameTime = 0.0f;

		while (!ShouldClose()) {
			auto newTime = std::chrono::high_resolution_clock::now();
			frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			for (Layer* layer : m_LayerStack)
				layer->OnUpdate(frameTime);

			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();

			m_ImGuiLayer->End();

			m_EditorCamera->OnUpdate(frameTime);
			m_Renderer->RenderScene(frameTime);
			m_Window->ProcessEvents();
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent &e) {
		return false;
	}

	bool Application::OnWindowResize(WindowResizeEvent &e) {
		m_Renderer->m_WindowResized = true;
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
