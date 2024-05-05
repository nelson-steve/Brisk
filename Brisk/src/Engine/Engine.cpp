#include "Engine/Engine.hpp"
#include "Core/Log.hpp"
#include "Graphics/Vulkan/Swapchain.hpp"

namespace Brisk
{
	Engine* s_Instance;
	EngineInfo Engine::s_EngineInfo;
	WindowBase* Engine::s_MainWindow;

	GraphicsDeviceVulkan* Engine::m_GPUDeviceVulkan;
	Swapchain* Engine::m_Swapchain;
	PhysicalDevice* Engine::s_PhysicalDevice;


	void Engine::Init() {
		Log::Init();
		s_MainWindow = WindowCreator::CreateWindowsWindow(1280, 720);
		m_GPUDeviceVulkan = new GraphicsDeviceVulkan();
		m_GPUDeviceVulkan->Create();
		m_Swapchain = m_GPUDeviceVulkan->CreateSwapchain(s_MainWindow);
		m_Swapchain->Create();
	}

	void Engine::Update() {
		while (!s_MainWindow->WindowShouldClose()) {
			s_MainWindow->ProcessEvents();
		}
	}

	void Engine::Terminate() {
		s_MainWindow->DestroyWindow();
		delete s_MainWindow;
		m_GPUDeviceVulkan->Release();
		delete m_GPUDeviceVulkan;
	}
}