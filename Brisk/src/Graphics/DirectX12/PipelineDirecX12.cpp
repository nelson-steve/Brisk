#include "PipelineDirectX12.hpp"
#include "GpuAdapterDirectX12.hpp"
#include "Engine/Engine.hpp"
//#include <d3dx12_core.h>

namespace Brisk
{
	void PipelineDirectX12::Init(const GraphicsPipelineSpecs& specs) {
        D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
        rootSignatureDesc.NumParameters = 0; // No root parameters for now
        rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        ComPtr<ID3DBlob> signatureBlob;
        ComPtr<ID3DBlob> errorBlob;
        HRESULT hr = D3D12SerializeRootSignature(
            &rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);

        if (FAILED(hr)) {
            throw std::runtime_error("Failed to serialize root signature");
        }

        hr = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateRootSignature(
            0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
            IID_PPV_ARGS(&m_RootSignature));

        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create root signature");
        }

        // Input layout
        D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        // create pipeline state
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.pRootSignature = m_RootSignature.Get();
        //psoDesc.VS = { vertexShaderBytecode, vertexShaderSize };
        //psoDesc.PS = { pixelShaderBytecode, pixelShaderSize };
        psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.NumRenderTargets = 1;

        //D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        //psoDesc.pRootSignature = m_RootSignature.Get();
        //psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
        ////psoDesc.VS = CD3DX12_SHADER_BYTECODE(pbrVS.Get());
        ////psoDesc.PS = CD3DX12_SHADER_BYTECODE(pbrPS.Get());
        //psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        //psoDesc.RasterizerState.FrontCounterClockwise = true;
        //psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        //psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        //psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        //psoDesc.NumRenderTargets = 1;
        //psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
        //psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
        ////psoDesc.SampleDesc.Count = m_framebuffers[0].samples;
        //psoDesc.SampleMask = UINT_MAX;

        hr = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PipelineState));
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create pipeline state");
        }
	}
	void PipelineDirectX12::Init(const ComputePipelineSpecs& specs) {

	}

	void PipelineDirectX12::Bind(std::shared_ptr<CommandBuffer> cmd) {

	}

	void PipelineDirectX12::BindPushConstant(std::shared_ptr<CommandBuffer> cmd, uint32_t, void* data) {

	}

	void PipelineDirectX12::Destroy() {

	}
}