#include "Engine/Engine.hpp"
#include "Graphics/Vulkan/GraphicsDeviceVulkan.hpp"

namespace Brisk
{
	Engine* s_Instance;
	EngineInfo Engine::s_EngineInfo;

	Engine::Engine() {
	}

	void Engine::Init() {
		GraphicsDeviceVulkan::Create();
	}
}