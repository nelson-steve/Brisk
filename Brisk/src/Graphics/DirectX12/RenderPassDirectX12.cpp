#include "RenderPassDirectX12.hpp"
#include <directx/d3dx12_default.h>

namespace Brisk
{
    void RenderPassDirectX12::Init(const RenderPassSpecs& specs) {
        D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
        rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        Microsoft::WRL::ComPtr<ID3DBlob> signature;
        Microsoft::WRL::ComPtr<ID3DBlob> error;
        D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);

        //ID3D12Device* device = ;
        //device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));

        // Create pipeline state (placeholder, actual implementation depends on shaders, blend state, etc.)
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.pRootSignature = m_RootSignature.Get();
        psoDesc.RasterizerState = CD3D12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3D12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;

        //device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PipelineState));
    }

    void RenderPassDirectX12::Begin(std::shared_ptr<CommandBuffer> cmd, uint32_t imageIndex) {
    }

    void RenderPassDirectX12::End(std::shared_ptr<CommandBuffer> cmd) {
    }

    void RenderPassDirectX12::AddRenderTarget(std::shared_ptr<Swapchain> swapchain) {
    }

    void RenderPassDirectX12::AddRenderTarget(std::shared_ptr<Texture> texture) {

    }
}