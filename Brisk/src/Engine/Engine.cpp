#include "Engine/Engine.hpp"
#include "Core/Log.hpp"
#include "Graphics/Factories/SwapchainFactory.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/RendererFactory.hpp"

namespace Brisk
{
	EngineInfo Engine::s_EngineInfo;
	WindowBase* Engine::s_MainWindow;

	Swapchain* Engine::s_Swapchain;
	Renderer* Engine::s_Renderer;

	void Engine::Init() {
		Log::Init();
		s_MainWindow = WindowCreator::CreateWindowsWindow(1920, 1080);

		s_Renderer = RendererFactory::CreateRenderer();
		s_Renderer->Create();

		s_Swapchain = SwapchainFactory::CreateSwapchain(s_MainWindow);
		s_Swapchain->Create();

		s_Renderer->SetupRenderingPipeline(s_Swapchain);


	}

	void Engine::Update() {
		while (!s_MainWindow->WindowShouldClose()) {
			s_MainWindow->ProcessEvents();
			s_Renderer->Render();
		}

		//s_Renderer->WaitForGPU();
		//s_GPUContext->WaitDeviceIdle();
	}

	void Engine::Terminate() {
		s_MainWindow->DestroyWindow();
		s_Swapchain->Release();
		s_Renderer->Release();
	}
}