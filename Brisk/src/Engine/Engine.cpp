#include "Engine/Engine.hpp"
#include "Core/Log.hpp"

namespace Brisk
{
	EngineInfo Engine::s_EngineInfo;
	std::unique_ptr<Editor> Engine::s_Editor;
	std::shared_ptr<Application> Engine::s_Application;

	void Engine::InitSystems() {
		Log::Init();
		s_Application = std::make_shared<Application>();
		s_Application->CreateApplication();

		//s_Editor = std::make_unique<Editor>();
		//s_Editor->Create();

		//glfwSetCursorPosCallback((GLFWwindow*)s_MainWindow->GetWindowHandle(), mouseCallback); 
		//glfwSetScrollCallback((GLFWwindow*)s_MainWindow->GetWindowHandle(), scrollCallback);
	}

	void Engine::Update() {
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
		s_Editor->Release();
	}
}