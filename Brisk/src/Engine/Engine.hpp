#include <string>

#include "Window.hpp"

namespace Brisk 
{
	/// <summary>
	/// Different Graphics APIs used
	/// Only support Vulkan at the moment
	/// </summary>
	enum class GraphicsAPI {
		Vulkan,
		DirectX
	};

	/// <summary>
	/// Store engine information with default values
	/// </summary>
	struct EngineInfo {
		std::string EngineName = "Brisk";
		GraphicsAPI API = GraphicsAPI::Vulkan;
	};

	/// <summary>
	/// Main Engine class holding all major systems of the engine
	/// </summary>
	static class Engine 
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
		static Window* s_MainWindow;
		/// <summary>
		/// Stores basic information about the engine
		/// </summary>
		static EngineInfo s_EngineInfo;
	private:
	};
}