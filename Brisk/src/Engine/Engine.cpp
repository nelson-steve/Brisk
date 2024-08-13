#include "Engine/Engine.hpp"
#include "Core/Log.hpp"
#include "Graphics/GPUDevice.hpp"
#include "Graphics/Factories/SwapchainFactory.hpp"
#include "Renderer/Renderer.hpp"

namespace Brisk
{
	EngineInfo Engine::s_EngineInfo;
	WindowBase* Engine::s_MainWindow;

	GPUContext* Engine::s_GPUContext;
	Swapchain* Engine::s_Swapchain;
	PhysicalDevice* Engine::s_PhysicalDevice;
	Renderer* Engine::s_Renderer;

	void Engine::Init() {
		Log::Init();
		s_MainWindow = WindowCreator::CreateWindowsWindow(1280, 720);

		s_GPUContext = GPUContext::CreateContext();
		s_GPUContext->Create();
		s_Swapchain = SwapchainFactory::CreateSwapchain(s_MainWindow);
		s_Swapchain->Create();

		s_Renderer = new Renderer();
		s_Renderer->Initialize();
	}

	void Engine::Update() {
		while (!s_MainWindow->WindowShouldClose()) {
			s_MainWindow->ProcessEvents();
			Engine::s_GPUContext->Sync();
			s_Renderer->Render();
			//s_GPUContext->Draw();
		}

		s_GPUContext->WaitDeviceIdle();
	}

	void Engine::Terminate() {
		s_MainWindow->DestroyWindow();
		delete s_MainWindow;
		s_Swapchain->Release();
		s_PhysicalDevice->Release();

		s_GPUContext->Release();
		delete s_GPUContext;
	}
}