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
			m_SwapChain->GetDesc(&desc);

			UINT width = desc.BufferDesc.Width;
			return width;
		}

		virtual uint32_t GetExtentHeight() const override {
			DXGI_SWAP_CHAIN_DESC desc;
			m_SwapChain->GetDesc(&desc);

			UINT height = desc.BufferDesc.Height;
			return height;
		}

		virtual void AcquireNextImage(uint64_t timeout, std::shared_ptr<Semaphore> semaphore, std::shared_ptr<Fence> fence, uint32_t* pImageIndex) override {
			UINT backBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
			*pImageIndex = backBufferIndex;
		}

		virtual void TransitionCurrentImage(std::shared_ptr<CommandBuffer> cmd, Texture::ImageBarrierParams params, int imageIndex) override { assert(false); }
		virtual void Blit(std::shared_ptr<CommandBuffer> cmd, std::shared_ptr<Texture> image, int imageIndex) override { assert(false); }

		void Present();

		SwapchainDirectX12(std::shared_ptr<Window> window);
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_RtvHandles;
	private:
		ComPtr<IDXGISwapChain4> m_SwapChain;
		std::vector<ComPtr<ID3D12Resource>> m_BackBuffers;

		friend class SwapchainFactory;
	};
}