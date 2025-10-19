// INCLUDES
#include "Engine/Engine.hpp"
//--------------------

namespace Brisk
{
	Engine::EngineSettings Engine::s_EngineSettings;
	uint32_t Engine::s_TexturesOffset;

	void Engine::InitSystems() {
		s_EngineSettings.API = Engine::EngineSettings::GraphicsAPI::Vulkan;
	}

	void Engine::Terminate() {
	}
}