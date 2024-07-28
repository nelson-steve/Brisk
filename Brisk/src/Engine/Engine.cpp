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

		ShaderInfo vertexShader;
		vertexShader.Path = "Shaders/Vulkan/Compiled/TriangleVS.spv";
		vertexShader.Type = ShaderType::Vertex;

		ShaderInfo fragmentShader;
		fragmentShader.Path = "Shaders/Vulkan/Compiled/TriangleFS.spv";
		fragmentShader.Type = ShaderType::Fragment;

		m_GPUDevice->SetupGraphicsPipeline( 
			{
				vertexShader,
				fragmentShader,
			}

		);
	}

	void Engine::Update() {
		while (!s_MainWindow->WindowShouldClose()) {
			s_MainWindow->ProcessEvents();
			m_GPUDevice->Draw();
		}

		m_GPUDevice->WaitDeviceIdle();
	}

	void Engine::Terminate() {
		s_MainWindow->DestroyWindow();
		delete s_MainWindow;
		m_Swapchain->Release();
		m_GPUDevice->ReleaseGraphicsPipeline();
		s_PhysicalDevice->Release();

		m_GPUDevice->Release();
		delete m_GPUDevice;
	}
}