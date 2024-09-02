#include "Engine/Engine.hpp"
#include "Core/Log.hpp"
#include "Graphics/Factories/SwapchainFactory.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/RendererFactory.hpp"
#include "Editor/Editor.hpp"

namespace Brisk
{
	EngineInfo Engine::s_EngineInfo;
	WindowBase* Engine::s_MainWindow;

	Swapchain* Engine::s_Swapchain;
	Renderer* Engine::s_Renderer;
	Camera* Engine::s_Camera;
	Editor* Engine::s_Editor;

	float lastX = 0.0f;
	float lastY = 0.0f;
	bool firstMouse = true;
	void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
	{
		// Forward the event to ImGui
		ImGui_ImplGlfw_CursorPosCallback(window, xposIn, yposIn);

		float xpos = static_cast<float>(xposIn);
		float ypos = static_cast<float>(yposIn);

		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

		lastX = xpos;
		lastY = ypos;

		Engine::s_Camera->SetMouseOffset(xoffset, yoffset);
		Engine::s_Camera->MouseMoved();
	}

	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		// Forward the event to ImGui
		ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);

		Engine::s_Camera->OnMouseScroll(yoffset);
	}

	void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
		ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

		
		// Custom logic here
	}

	void Engine::Init() {
		Log::Init();
		s_MainWindow = WindowCreator::CreateWindowsWindow(1920, 1080);

		s_Renderer = RendererFactory::CreateRenderer();
		s_Renderer->Create();

		s_Swapchain = SwapchainFactory::CreateSwapchain(s_MainWindow);
		s_Swapchain->Create(Swapchain::Mode::TRIPLE_BUFFERING);

		s_Renderer->SetupRenderingPipeline(s_Swapchain);

		s_Editor->Create();

		float aspect = s_MainWindow->GetWidth() / s_MainWindow->GetHeight();
		s_Camera = new Camera(60.0f, aspect, 0.01, 1000.0f, (GLFWwindow*)s_MainWindow->GetWindowHandle());

		glfwSetCursorPosCallback((GLFWwindow*)s_MainWindow->GetWindowHandle(), mouse_callback);
		glfwSetScrollCallback((GLFWwindow*)s_MainWindow->GetWindowHandle(), scroll_callback);
		//glfwSetMouseButtonCallback((GLFWwindow*)s_MainWindow->GetWindowHandle(), mouse_button_callback);	
	}

	void Engine::Update() {
		auto currentTime = std::chrono::high_resolution_clock::now();
		while (!s_MainWindow->WindowShouldClose()) {
			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;
			s_MainWindow->ProcessEvents();
			//s_Camera->OnUpdate(frameTime, (GLFWwindow*)s_MainWindow->GetWindowHandle());
			s_Renderer->Render();
			//s_Editor->Update();
		}

		s_Renderer->WaitDeviceIdle();
	}

	void Engine::Terminate() {
		s_MainWindow->DestroyWindow();
		s_Swapchain->Release();
		s_Renderer->Release();
	}
}