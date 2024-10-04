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
	private:
		std::unique_ptr<Renderer> m_Renderer;
		std::unique_ptr<Window> m_Window;
	};
}