#include "SwapchainDirectX12.hpp"
#include "Graphics/DirectX12/GpuAdapterDirectX12.hpp"
#include "Engine/Engine.hpp"
#include "CommandBufferDirectX12.hpp"
#include "UtilitiesDirectX12.hpp"

namespace Brisk
{
    SwapchainDirectX12::SwapchainDirectX12(std::shared_ptr<Window> window)
        : Swapchain(window, window->GetWidth(), window->GetHeight()) {}

	void SwapchainDirectX12::Create(Mode mode) {
        DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount = mode == Mode::DOUBLE_BUFFERING ? 2 : 3;
        swapChainDesc.Width = Application::GetWindow()->GetWidth();
        swapChainDesc.Height = Application::GetWindow()->GetHeight();
        swapChainDesc.Format = backBufferFormat;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1;

        ComPtr<IDXGISwapChain1> tempSwapChain;
        
        HRESULT hr = Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDXGIFactory()->CreateSwapChainForHwnd(
            std::static_pointer_cast<GpuAdapterDirectX12>(Application::GetGpuAdapter())->GetGraphicsQueue(),
            (HWND)Application::GetWindow()->GetHWNDWindowHandle(),
            &swapChainDesc,
            nullptr,
            nullptr,
            &tempSwapChain
        );

        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create swap chain");
        }

        // Convert to IDXGISwapChain4 for modern features
        tempSwapChain.As(&m_SwapChain);

        UINT rtvDescriptorSize = std::static_pointer_cast<GpuAdapterDirectX12>(Application::GetGpuAdapter())->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = std::static_pointer_cast<GpuAdapterDirectX12>(Application::GetGpuAdapter())->GetRtvHeap()->GetCPUDescriptorHandleForHeapStart();

        m_BackBuffers.resize((uint32_t)mode);
        for (UINT i = 0; i < (uint32_t)mode; ++i) {
            m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_BackBuffers[i]));
            std::static_pointer_cast<GpuAdapterDirectX12>(Application::GetGpuAdapter())->GetDevice()->CreateRenderTargetView(m_BackBuffers[i].Get(), nullptr, rtvHandle);
            m_RtvHandles.push_back(rtvHandle);
            rtvHandle.ptr += rtvDescriptorSize;
        }
	}

    void SwapchainDirectX12::TransitionCurrentImage(std::shared_ptr<CommandBuffer> cmd, Texture::ImageBarrierParams params, int imageIndex) {
        D3D12_RESOURCE_BARRIER barrier{};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = m_BackBuffers[imageIndex].Get();
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = UtilitiesDirectX12::ImageLayoutToD3D12ResourceState(params.oldLayout);
        barrier.Transition.StateAfter = UtilitiesDirectX12::ImageLayoutToD3D12ResourceState(params.newLayout);

        std::static_pointer_cast<CommandBufferDirectX12>(cmd)->Get()->ResourceBarrier(1, &barrier);
    }

    void SwapchainDirectX12::Present() {
        HRESULT hr = m_SwapChain->Present(true, 0);
        if (FAILED(hr)) {
            HRESULT error = std::static_pointer_cast<GpuAdapterDirectX12>(Application::GetGpuAdapter())->GetDevice()->GetDeviceRemovedReason();
            throw std::runtime_error("Failed to present swapchain in DirectX12.");
        }
    }

    void SwapchainDirectX12::Release() {
        m_SwapChain.Reset();
    }
}