#pragma once

#include "Renderer/Renderer.hpp"
#include "Window.hpp"
#include "Editor/Editor.hpp"

#include "EngineGlobal.hpp"
#include "Graphics/Swapchain.hpp"
#include "Camera.hpp"
#include "Application.hpp"

#include <glm/glm.hpp>

#include <memory>
#include <string>
#include "Scene.hpp"

namespace Brisk 
{
	struct Point {
		glm::vec3 Position;
		glm::vec3 Color;
	};

	struct MVPBuffer {
		glm::mat4 Model;
		glm::mat4 View;
		glm::mat4 Projection;
	};

	/// <summary>
	/// Main Engine class holding all major systems of the engine
	/// </summary>
	class Engine {
	public:
		/// <summary>
		/// Initialize all engine systems
		/// </summary>
		static void InitSystems();
		/// <summary>
		/// Engine wide update
		/// </summary>
		static void Update();
		/// <summary>
		/// Ends all Engine systems
		/// </summary>
		static void Terminate();
	public:
		/// <summary>
		/// Main window of the engine
		/// </summary>
		static std::shared_ptr<Window> s_MainWindow;
		static std::unique_ptr<Application> s_Application;
		/// <summary>
		/// Stores basic information about the engine
		/// </summary>
		static EngineInfo s_EngineInfo;

		/// <summary>
		/// Editor handle
		/// </summary>
		static std::unique_ptr<Editor> s_Editor;

		/// <summary>
		/// Camera handle
		/// </summary>
		static Camera* s_Camera;

		/// <summary>
		/// Renderer handle
		/// </summary>
		static Renderer* s_Renderer;

		/// <summary>
		/// Swapchain handle
		/// </summary>
		static Swapchain* s_Swapchain;

		static BriskScene* m_ActiveScene;
	};
}