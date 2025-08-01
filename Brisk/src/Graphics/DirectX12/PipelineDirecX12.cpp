#include "PipelineDirectX12.hpp"
#include "GpuAdapterDirectX12.hpp"
#include "Engine/Engine.hpp"
#include "UtilitiesDirectX12.hpp"

#include <directx/d3dx12.h>
#include <Core/Log.hpp>

namespace Brisk
{
	void PipelineDirectX12::Init(const GraphicsPipelineSpecs& specs) {
        ComPtr<ID3DBlob> vertexShaderBlob;
        ComPtr<ID3DBlob> fragmentShaderBlob;
        for (const std::string& path : specs.pShaderPathsDX) {
            std::wstring wideStr = std::wstring(path.begin(), path.end());
            LPCWSTR pathWstring = wideStr.c_str();
            std::wstring shaderPath = std::filesystem::current_path().wstring() + pathWstring;

            HRESULT hr;
            if (shaderPath.find(L"_vert") != std::wstring::npos)
            {
                hr = D3DReadFileToBlob(shaderPath.c_str(), &vertexShaderBlob);
            }
            else if (shaderPath.find(L"_frag") != std::wstring::npos)
            {
                hr = D3DReadFileToBlob(shaderPath.c_str(), &fragmentShaderBlob);
            }
            else
            {
                BRISK_CORE_ERROR("Unknown shader type {}", path);
            }

            if (hr == ERROR_FILE_NOT_FOUND) {
                BRISK_CORE_ERROR("File not found {}", path);
            }
            else if (hr == ERROR_PATH_NOT_FOUND) {
                BRISK_CORE_ERROR("Invalid path specified {}", path);
            }
            else if (FAILED(hr)) {
                BRISK_CORE_ERROR("Failed to load shader file {}", path);
            }
        }

        CD3DX12_ROOT_PARAMETER rootParams[2];
        rootParams[0].InitAsConstantBufferView(0); // b0
        rootParams[1].InitAsConstantBufferView(1); // b1

        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
        rootSignatureDesc.NumParameters = 0;
        rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        rootSignatureDesc.Init(_countof(rootParams), rootParams, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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

        std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayouts;

        if (specs.pLayout.has_value()) {
            for (const auto& attr : specs.pLayout.value().pAttributes) {
                D3D12_INPUT_ELEMENT_DESC element = {};
                element.SemanticName = UtilitiesDirectX12::SemanticFromLocation(attr.pLocation);
                element.SemanticIndex = UtilitiesDirectX12::SemanticIndexFromLocation(attr.pLocation);
                element.Format = UtilitiesDirectX12::FormatToDXGIFormat(attr.pFormat);
                element.InputSlot = attr.pBinding;
                element.AlignedByteOffset = attr.pOffset;
                element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
                element.InstanceDataStepRate = 0;

                inputLayouts.push_back(element);
            }
        }

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.pRootSignature = m_RootSignature.Get();
        psoDesc.InputLayout = { inputLayouts.data(), UINT(inputLayouts.size())};
        psoDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
        psoDesc.PS = { fragmentShaderBlob->GetBufferPointer(), fragmentShaderBlob->GetBufferSize() };
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.RasterizerState.FrontCounterClockwise = true;
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = specs.pRenderPass->GetColorAttachmentCount();
        for (int i = 0; i < specs.pRenderPass->GetColorAttachmentCount() && i < 8; i++) {
            psoDesc.RTVFormats[i] = DXGI_FORMAT_R16G16B16A16_FLOAT;
        }
        psoDesc.DSVFormat = DXGI_FORMAT_D16_UNORM; // TODO: Dont use hardcoded value
        psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        psoDesc.SampleDesc.Count = 1;
        psoDesc.SampleMask = UINT_MAX;

        hr = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PipelineState));
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create pipeline state");
        }
	}
	void PipelineDirectX12::Init(const ComputePipelineSpecs& specs) {

	}

	void PipelineDirectX12::Bind(std::shared_ptr<CommandBuffer> cmd) {

	}

	void PipelineDirectX12::BindPushConstant(std::shared_ptr<CommandBuffer> cmd, uint32_t, void* data, uint32_t offset, bool vertexShader) {

	}
}