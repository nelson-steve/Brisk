#pragma once

#include "Graphics/Swapchain.hpp"

namespace Brisk {
	class Renderer {
	public:
		virtual ~Renderer() {};

		virtual void Create() = 0;
		virtual void Release() = 0;

		virtual void SetupRenderingPipeline(Swapchain* swapchain) = 0;

		virtual void PreRender() = 0;
		virtual void Render() = 0;
		virtual void PostRender() = 0;
	};
}