#pragma once

// INCLUDES
#include "Scene.hpp"
#include "Window.hpp"
#include "Camera.hpp"
#include "SceneManager.hpp"
#include "Renderer/Renderer.hpp"
#include "Events/MouseEvent.hpp"
#include "Renderer/GpuAdapter.hpp"
#include "Events/ApplicationEvent.hpp"
#include "Platform/Windows/WindowsWindow.hpp"
#include "AssetManager.hpp"
#include <Layers/LayerStack.hpp>
#include "ImGuiLayer.hpp"
//-------------------------------------------

namespace Brisk 
{
	class Application {
	public:
		Application() = default;
		~Application() = default;
		Application(std::string name);
		void OnEvent(Event& event);
		void Run();
		void Close();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		inline bool ShouldClose() const { return m_Window->WindowShouldClose(); }
		static std::shared_ptr<WindowsWindow> GetWindow()
		{ 
			return std::static_pointer_cast<WindowsWindow>(m_Window);
		}

		void GenerateRandomLights(uint32_t count, float range = 10.0f);

		static ImGuiLayer* GetGuiLayer() { return m_ImGuiLayer; }
		static const std::shared_ptr<GpuAdapter> GetGpuAdapter() { return m_Adapter; }
		static const std::shared_ptr<Renderer> GetRenderer() { return m_Renderer; }
		static const std::shared_ptr<Camera> GetCamera() { return m_EditorCamera; }

	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
		bool OnMouseMoved(MouseMovedEvent& e);
		bool OnMouseScrolled(MouseScrolledEvent& e);
	private:
		LayerStack m_LayerStack;
		static ImGuiLayer* m_ImGuiLayer;
		static std::shared_ptr<GpuAdapter> m_Adapter;
		static std::shared_ptr<Renderer> m_Renderer;
		static std::shared_ptr<Window> m_Window;
		static std::shared_ptr<Camera> m_EditorCamera;
		std::shared_ptr<Scene> m_ActiveScene;
		std::shared_ptr<SceneManager> m_SceneManager;
		std::shared_ptr<AssetManager> m_AssetManager;
		std::vector<std::shared_ptr<Scene>> m_Scenes;
		bool m_LoadingScene;
	};
}