#pragma once

#include "Pipeline.hpp"
#include "CommandBuffer.hpp"
#include "Graphics/Swapchain.hpp"

#include <memory>

namespace Brisk 
{
	class Renderer {
	public:
		void Init();
		void RenderScene();

		static std::unique_ptr<Renderer> Create() { return nullptr; }

	public:
		static Swapchain* swapchain;
	};
}