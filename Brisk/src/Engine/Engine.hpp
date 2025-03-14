#pragma once

// INCLUDES
#include "Application.hpp"
#include "Editor/Editor.hpp"
#include "EngineGlobal.hpp"
//------------------------
#include <memory>
#include <string>
//---------------

namespace Brisk
{
	/// <summary>
	/// Engine class holding all major systems of the engine
	/// </summary>
	class Engine {
	public:
		struct EngineSettings {
			enum class GraphicsAPI {
				Vulkan,
				DirectX12
			};
			GraphicsAPI API;
		};
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
		static std::shared_ptr<Application> s_Application;
		/// <summary>
		/// Stores basic information about the engine
		/// </summary>
		static EngineSettings s_EngineSettings;

		/// <summary>
		/// Editor handle
		/// </summary>
		static std::unique_ptr<Editor> s_Editor;
	};
}