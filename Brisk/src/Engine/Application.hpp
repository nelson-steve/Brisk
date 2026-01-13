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
#include "Layers/LayerStack.hpp"
#include "JobSystem.hpp"
#include "ImGuiLayer.hpp"

#include <mutex>
//-------------------------------------------

namespace Brisk 
{
	struct RendererSettings {
		bool RayTracing = false;

		float knee = 1.0;
		float threshold = 1.0;
		float intensity = 1.0;

		bool CSM = true;
	};

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
		static std::shared_ptr<WindowsWindow> GetWindow() { return std::static_pointer_cast<WindowsWindow>(m_Window); }
		static ImGuiLayer* GetGuiLayer() { return m_ImGuiLayer; }
		static const std::shared_ptr<GpuAdapter> GetGpuAdapter() { return m_Adapter; }
		static const std::shared_ptr<Renderer> GetRenderer() { return m_Renderer; }
		static const std::shared_ptr<Camera> GetEditorCamera() { return m_ActiveCamera; }
		static JobSystem& GetJobSystem() { return m_JobSystem; }
		static RendererSettings& GetRendererSettings() { return m_RendererSettings; }
	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
		bool OnMouseMoved(MouseMovedEvent& e);
		bool OnMouseScrolled(MouseScrolledEvent& e);
	private:
		static RendererSettings m_RendererSettings;
		LayerStack m_LayerStack;
		static JobSystem m_JobSystem;
		static ImGuiLayer* m_ImGuiLayer;
		static std::shared_ptr<GpuAdapter> m_Adapter;
		static std::shared_ptr<Renderer> m_Renderer;
		static std::shared_ptr<Window> m_Window;
		static std::shared_ptr<Camera> m_ActiveCamera;
		static std::shared_ptr<Camera> m_EditorCamera;
		static std::shared_ptr<Camera> m_EditorCameraRT;
		std::shared_ptr<SceneManager> m_SceneManager;
		std::shared_ptr<AssetManager> m_AssetManager;
	};
}