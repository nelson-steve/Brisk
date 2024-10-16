#include "Engine/Engine.hpp"
#include "Core/Log.hpp"

namespace Brisk
{
	EngineInfo Engine::s_EngineInfo;
	std::unique_ptr<Editor> Engine::s_Editor;
	std::shared_ptr<Application> Engine::s_Application;

	/*
	float lastX = 0.0f;
	float lastY = 0.0f;
	bool firstMouse = true;
	void mouseCallback(GLFWwindow* window, double xposIn, double yposIn)
	{
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

	void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
	{
		Engine::s_Camera->OnMouseScroll(yoffset);
	}
	*/

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