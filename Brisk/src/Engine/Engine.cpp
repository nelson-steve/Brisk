#include "Engine/Engine.hpp"
#include "Graphics/Vulkan/GraphicsDeviceVulkan.hpp"
#include "Core/Log.hpp"

namespace Brisk
{
	Engine* s_Instance;
	EngineInfo Engine::s_EngineInfo;
	Window* Engine::s_MainWindow;

	void Engine::Init() {
		Log::Init();
		BRISK_APP_INFO("SpdLog has been initialized successfully");
		s_MainWindow = WindowCreator::CreateWindowsWindow(640, 480);
		GraphicsDeviceVulkan::Create();
	}

	void Engine::Update() {
		while (!s_MainWindow->WindowShouldClose()) {
			s_MainWindow->PollEvents();
		}
	}

	void Engine::Terminate() {
		s_MainWindow->DestroyWindow();
		delete s_MainWindow;
		GraphicsDeviceVulkan::Release();
	}
}