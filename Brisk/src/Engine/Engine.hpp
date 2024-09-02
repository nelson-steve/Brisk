#pragma once

#include "EngineGlobal.hpp"
#include "WindowBase.hpp"
#include "Graphics/Swapchain.hpp"
#include "Renderer/Renderer.hpp"
#include "Camera.hpp"

#include <glm/glm.hpp>

#include <string>

namespace Brisk 
{
	class Renderer;
	class Editor;

	//namespace Globals{
	//	static GraphicsAPI GPU_API = GraphicsAPI::Vulkan;
	//}

	//struct GlobalGPUDevice {
	//	static GraphicsDeviceVulkan* m_GPUDeviceVulkan;
	//	static Swapchain* m_Swapchain;
	//	static PhysicalDevice* s_PhysicalDevice;
	//};

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

		static Editor* s_Editor;
		static Camera* s_Camera;
		static Renderer* s_Renderer;
		static Swapchain* s_Swapchain;
	private:
	};
}