#pragma once

#include "Window.hpp"
#include "Renderer/GpuAdapter.hpp"
#include "Renderer/Renderer.hpp"
#include "Platform/Windows/WindowsWindow.hpp"

namespace Brisk 
{
	class Application {
	public:
		Application() = default;
		~Application() = default;
		std::shared_ptr<Window> CreateApplication();
		void Update(float deltaTime);
		void Close();

		inline bool ShouldClose() const { return m_Window->WindowShouldClose(); }
		std::shared_ptr<WindowsWindow> GetWindow()
		{ 
			return std::static_pointer_cast<WindowsWindow>(m_Window);
		}
		const std::shared_ptr<GpuAdapter> GetGpuAdapter() const { return m_Adapter; }
	private:
		std::shared_ptr<GpuAdapter> m_Adapter;
		std::shared_ptr<Renderer> m_Renderer;
		std::shared_ptr<Window> m_Window;
	};
}