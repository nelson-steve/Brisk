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
//-------------------------------------------

namespace Brisk 
{
	class Application {
	public:
		Application() = default;
		~Application() = default;
		void CreateApplication();
		void OnEvent(Event& event);
		void Update(float deltaTime);
		void Close();

		inline bool ShouldClose() const { return m_Window->WindowShouldClose(); }
		std::shared_ptr<WindowsWindow> GetWindow()
		{ 
			return std::static_pointer_cast<WindowsWindow>(m_Window);
		}

		const std::shared_ptr<GpuAdapter> GetGpuAdapter() const { return m_Adapter; }
		const std::shared_ptr<Renderer> GetRenderer() const { return m_Renderer; }
		const std::shared_ptr<Camera> GetCamera() const { return m_EditorCamera; }

	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
		bool OnMouseMoved(MouseMovedEvent& e);
		bool OnMouseScrolled(MouseScrolledEvent& e);
	private:
		std::shared_ptr<GpuAdapter> m_Adapter;
		std::shared_ptr<Renderer> m_Renderer;
		std::shared_ptr<Window> m_Window;
		std::shared_ptr<Camera> m_EditorCamera;
		std::shared_ptr<Scene> m_ActiveScene;
		std::shared_ptr<SceneManager> m_SceneManager;
		std::shared_ptr<AssetManager> m_AssetManager;
		std::vector<std::shared_ptr<Scene>> m_Scenes;
		bool m_LoadingScene;
	};
}