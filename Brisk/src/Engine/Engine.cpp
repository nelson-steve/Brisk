#include "Engine/Engine.hpp"
#include "Core/Log.hpp"
#include "Graphics/GPUDevice.hpp"
#include "Graphics/Factories/SwapchainFactory.hpp"

namespace Brisk
{
	EngineInfo Engine::s_EngineInfo;
	WindowBase* Engine::s_MainWindow;

	GPUDevice* Engine::m_GPUDevice;
	Swapchain* Engine::m_Swapchain;
	PhysicalDevice* Engine::s_PhysicalDevice;

	void Engine::Init() {
		Log::Init();
		s_MainWindow = WindowCreator::CreateWindowsWindow(1280, 720);

		m_GPUDevice = GPUDevice::CreateDevice();
		m_GPUDevice->Create();
		m_Swapchain =  SwapchainFactory::CreateSwapchain(s_MainWindow);
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
		m_Swapchain->Release();
		s_PhysicalDevice->Release();
		m_GPUDevice->Release();
		delete m_GPUDevice;
	}
}