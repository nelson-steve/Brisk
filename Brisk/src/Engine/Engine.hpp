#pragma once

#include <string>

#include "Graphics/GPUDevice.hpp"
#include "Graphics/Vulkan/PhysicalDevice.hpp"
#include "EngineGlobal.hpp"

namespace Brisk 
{
	class Renderer;

	//namespace Globals{
	//	static GraphicsAPI GPU_API = GraphicsAPI::Vulkan;
	//}

	//struct GlobalGPUDevice {
	//	static GraphicsDeviceVulkan* m_GPUDeviceVulkan;
	//	static Swapchain* m_Swapchain;
	//	static PhysicalDevice* s_PhysicalDevice;
	//};

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

		static Renderer* s_Renderer;
		static Swapchain* s_Swapchain;
	private:
	};
}