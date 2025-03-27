#include "FramebufferDirectX12.hpp"

namespace Brisk
{
    void FramebufferDirectX12::Init(const FramebufferSpecs& specs) {
        //ID3D12Device* device = ;

        D3D12_RESOURCE_DESC rtDesc = {};
        rtDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        rtDesc.Width = specs.Width;
        rtDesc.Height = specs.Height;
        rtDesc.DepthOrArraySize = 1;
        rtDesc.MipLevels = 1;
        rtDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        rtDesc.SampleDesc.Count = 1;
        rtDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        rtDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        clearValue.Color[0] = 0.0f;
        clearValue.Color[1] = 0.0f;
        clearValue.Color[2] = 0.0f;
        clearValue.Color[3] = 1.0f;

        device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &rtDesc,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            &clearValue,
            IID_PPV_ARGS(&m_RenderTarget)
        );

        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.NumDescriptors = 1;
        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_RTVHeap));

        m_RTVHandle = m_RTVHeap->GetCPUDescriptorHandleForHeapStart();
        device->CreateRenderTargetView(m_RenderTarget.Get(), nullptr, m_RTVHandle);
    }

    void FramebufferDirectX12::Destroy() {
        m_RenderTarget.Reset();
        m_RTVHeap.Reset();
    }

    void FramebufferDirectX12::Bind() {
        //ID3D12GraphicsCommandList* commandList =;
        commandList->OMSetRenderTargets(1, &m_RTVHandle, FALSE, nullptr);
    }
}
