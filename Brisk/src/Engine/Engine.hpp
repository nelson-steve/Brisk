#pragma once

#include "EngineGlobal.hpp"
#include "WindowBase.hpp"
#include "Graphics/Swapchain.hpp"
#include "Renderer/Renderer.hpp"
#include "Camera.hpp"

#include <glm/glm.hpp>

#include <string>
#include "Scene.hpp"

namespace Brisk 
{
	class Renderer;
	class Editor;

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
	class Engine 
	{
	public:
		/// <summary>
		/// Initialize all engine systems
		/// </summary>
		static void Init();
		/// <summary>
		/// Engine wide update
		/// </summary>
		static void Update();
		/// <summary>
		/// Ends all Engine systems
		/// </summary>
		static void Terminate();
	private:
		Engine() {};
	public:
		/// <summary>
		/// Main window of the engine aka the Editor window
		/// </summary>
		static WindowBase* s_MainWindow;
		/// <summary>
		/// Stores basic information about the engine
		/// </summary>
		static EngineInfo s_EngineInfo;

		/// <summary>
		/// Editor handle
		/// </summary>
		static Editor* s_Editor;

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

		static BriskScene* m_Scene;
	};
}