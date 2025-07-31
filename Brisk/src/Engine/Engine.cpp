// INCLUDES
#include "Engine/Engine.hpp"
#include "Core/Log.hpp"
//--------------------

namespace Brisk
{
	Engine::EngineSettings Engine::s_EngineSettings;
	uint32_t Engine::s_TexturesOffset = 0;
	std::shared_ptr<Application> Engine::s_Application;

	void Engine::InitSystems() {
		Log::Init();

		s_EngineSettings.API = Engine::EngineSettings::GraphicsAPI::DirectX12;

		s_Application = std::make_shared<Application>();
		s_Application->CreateApplication();
	}

	void Engine::Run() {
		auto currentTime = std::chrono::high_resolution_clock::now();
		while (!s_Application->ShouldClose()) {
			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;
			s_Application->Update(frameTime);
		}
	}

	void Engine::Terminate() {
		s_Application->Close();
	}
}