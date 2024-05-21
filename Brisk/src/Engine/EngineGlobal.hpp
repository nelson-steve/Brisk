#pragma once

#include <string>

/// <summary>
/// Store engine information with default values
/// </summary>
static struct EngineInfo {
	/// <summary>
	/// Different Graphics APIs used
	/// Only support Vulkan at the moment
	/// </summary>
	static enum class GraphicsAPI {
		Vulkan,
		DirectX
	};
	std::string EngineName;
	GraphicsAPI API;
};