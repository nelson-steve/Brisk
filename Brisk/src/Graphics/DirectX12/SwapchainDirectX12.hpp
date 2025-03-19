#pragma once

#include "Engine/Renderer/Swapchain.hpp"
#include "Graphics/DirectX12/GpuAdapterDirectX12.hpp"

namespace Brisk
{
	class SwapchainVulkan : public Swapchain {
	public:
		virtual void Create(Mode mode) override;
		virtual void Release() override;

		SwapchainVulkan(std::shared_ptr<Window> window);
	private:
		ComPtr<IDXGISwapChain4> swapChain;

		friend class SwapchainFactory;
	};
}