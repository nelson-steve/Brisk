#pragma once

#include "Engine/Engine.hpp"

namespace Brisk 
{
	class Application {
	public:
		std::shared_ptr<Window> CreateApplication();
		void Update(float deltaTime);
		void Close();

		inline bool ShouldClose() const { return m_Window->WindowShouldClose(); }
		const std::shared_ptr<Window> GetWindow() const { return m_Window; }
	private:
		std::unique_ptr<Renderer> m_Renderer;
		std::shared_ptr<Window> m_Window;
	};
}