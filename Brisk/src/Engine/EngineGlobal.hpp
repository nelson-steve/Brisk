#pragma once

#include <string>

/// <summary>
/// Store engine information with default values
/// </summary>
struct EngineInfo {
	/// <summary>
	/// Different Graphics APIs used
	/// Only support Vulkan at the moment
	/// </summary>
	enum class GraphicsAPI {
		Vulkan,
		DirectX
	};
	std::string EngineName;
	GraphicsAPI API;
};