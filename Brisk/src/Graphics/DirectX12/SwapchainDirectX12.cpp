#include "SwapchainDirectX12.hpp"
#include "Graphics/DirectX12/GpuAdapterDirectX12.hpp"
#include "Engine/Engine.hpp"

namespace Brisk
{
    SwapchainDirectX12::SwapchainDirectX12(std::shared_ptr<Window> window)
        : Swapchain(window, window->GetWidth(), window->GetHeight()) {}

	void SwapchainDirectX12::Create(Mode mode) {
        DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount = mode == Mode::DOUBLE_BUFFERING ? 2 : 3;
        swapChainDesc.Width = Engine::s_Application->GetWindow()->GetWidth();
        swapChainDesc.Height = Engine::s_Application->GetWindow()->GetHeight();
        swapChainDesc.Format = backBufferFormat;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1;

        ComPtr<IDXGISwapChain1> tempSwapChain;
        
        HRESULT hr = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDXGIFactory()->CreateSwapChainForHwnd(
            std::static_pointer_cast<GpuAdapterDirectX12>(Engine::s_Application->GetGpuAdapter())->GetGraphicsQueue(),
            (HWND)Engine::s_Application->GetWindow()->GetHWNDWindowHandle(),
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
	}

    void SwapchainDirectX12::Present() {
        HRESULT hr = m_SwapChain->Present(false, 0);
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to present swapchain in DirectX12.");
        }
    }

    void SwapchainDirectX12::Release() {
        m_SwapChain.Reset();
    }
}