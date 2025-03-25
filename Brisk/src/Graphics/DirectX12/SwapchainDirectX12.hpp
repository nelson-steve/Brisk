#pragma once

#include "Engine/Renderer/Swapchain.hpp"
#include "Graphics/DirectX12/GpuAdapterDirectX12.hpp"

namespace Brisk
{
	class SwapchainDirectX12 : public Swapchain {
	public:
		virtual void Create(Mode mode) override;
		virtual void Release() override;

		virtual uint32_t GetExtentWidth() const override {
			DXGI_SWAP_CHAIN_DESC desc;
			swapChain->GetDesc(&desc);

			UINT width = desc.BufferDesc.Width;
			return width;
		}
		virtual uint32_t GetExtentHeight() const override {
			DXGI_SWAP_CHAIN_DESC desc;
			swapChain->GetDesc(&desc);

			UINT height = desc.BufferDesc.Height;
			return height;
		}

		virtual void AquireNextImage(uint64_t timeout, std::shared_ptr<Semaphore> semaphore, std::shared_ptr<Fence> fence, uint32_t* pImageIndex) override {
			UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();
			*pImageIndex = backBufferIndex;
		}

		SwapchainDirectX12(std::shared_ptr<Window> window);
	private:
		ComPtr<IDXGISwapChain4> swapChain;

		friend class SwapchainFactory;
	};
}