#pragma once

#include "Engine/Renderer/Renderer.hpp"
#include "../GpuContextVulkan.hpp"

namespace Brisk {
	class RendererVulkan : public Renderer {
	public:
		virtual void Create() override;
		virtual void Release() override;

		virtual void SetupSwapchain(const Swapchain* swapchain) override;

		virtual void PreRender() override;
		virtual void Render() override;
		virtual void PostRender() override;
	private:
		GpuContextVulkan* m_GpuContext;
	};
}