#include "PipelineDirectX12.hpp"
#include "GpuAdapterDirectX12.hpp"
#include "Engine/Engine.hpp"
#include "UtilitiesDirectX12.hpp"

#include <d3dcompiler.h>
#include <dxcapi.h>
#include <directx/d3dx12.h>
#include <Core/Log.hpp>
#include "CommandBufferDirectX12.hpp"
#include <filesystem>

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

        std::string path = specs.pShaderPathsDX[1];
        // Retrieving the name of shader
        size_t lastSlash = path.find_last_of("/\\");
        std::string filenameWithExt = (lastSlash == std::string::npos) ? path : path.substr(lastSlash + 1);

        size_t lastDot = filenameWithExt.find_last_of('.');
        std::string filename = (lastDot == std::string::npos) ? filenameWithExt : filenameWithExt.substr(0, lastDot);

        if (filename == "DepthPrePass_frag") {
            CD3DX12_ROOT_PARAMETER rootParameters[2];

            // MVPBuffer (b0)
            rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

            // MeshData (b1)
            rootParameters[1].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_VERTEX);

            CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc;
            rootSigDesc.Init(_countof(rootParameters), rootParameters,
                0, nullptr, // No static samplers
                D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

            ComPtr<ID3DBlob> serializedRootSig = nullptr;
            ComPtr<ID3DBlob> errorBlob = nullptr;

            HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc,
                D3D_ROOT_SIGNATURE_VERSION_1,
                &serializedRootSig, &errorBlob);

            Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateRootSignature(0, serializedRootSig->GetBufferPointer(),
                serializedRootSig->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));

        }
        else if (filename == "GeometryPass_frag") {
            CD3DX12_ROOT_PARAMETER1 rootParameters[5];

            // 0. MVPBuffer (b0, space0)
            rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);

            // 1. MeshData for VS (b1)
            rootParameters[1].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);

            // 2. MeshData for PS (b2) – only needed if you keep that MeshData block in PS
            rootParameters[2].InitAsConstantBufferView(2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);

            // 3. Global textures (t0, space3)
            CD3DX12_DESCRIPTOR_RANGE1 textureRange;
            textureRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 512, 0, 3); // u_GlobalTextures
            rootParameters[3].InitAsDescriptorTable(1, &textureRange, D3D12_SHADER_VISIBILITY_PIXEL);

            // 4. Material buffer (t0, space4)
            CD3DX12_DESCRIPTOR_RANGE1 materialRange;
            materialRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 4); // ssbo_Materials
            rootParameters[4].InitAsDescriptorTable(1, &materialRange, D3D12_SHADER_VISIBILITY_PIXEL);

            // Static sampler (s0, space0)
            CD3DX12_STATIC_SAMPLER_DESC staticSampler(
                0, D3D12_FILTER_MIN_MAG_MIP_LINEAR,
                D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                0.0f, 1,
                D3D12_COMPARISON_FUNC_ALWAYS,
                D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK,
                0.0f, D3D12_FLOAT32_MAX,
                D3D12_SHADER_VISIBILITY_PIXEL,
                0 // space0
            );

            D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc = {};
            rootSigDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
            rootSigDesc.Desc_1_1.NumParameters = _countof(rootParameters);
            rootSigDesc.Desc_1_1.pParameters = rootParameters;
            rootSigDesc.Desc_1_1.NumStaticSamplers = 1;
            rootSigDesc.Desc_1_1.pStaticSamplers = &staticSampler;
            rootSigDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

            ComPtr<ID3DBlob> signatureBlob;
            ComPtr<ID3DBlob> errorBlob;
            HRESULT hr = D3DX12SerializeVersionedRootSignature(
                &rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &signatureBlob, &errorBlob);

            if (FAILED(hr)) {
                if (errorBlob) {
                    OutputDebugStringA((char*)errorBlob->GetBufferPointer());
                }
            }

            Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateRootSignature(
                0, signatureBlob->GetBufferPointer(),
                signatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));

        }
        else if (filename == "LightingPass_frag") {
            CD3DX12_ROOT_PARAMETER1 rootParameters[5];

            rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);

            CD3DX12_DESCRIPTOR_RANGE1 rangeLights;
            rangeLights.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1); // t0, space1
            rootParameters[1].InitAsDescriptorTable(1, &rangeLights, D3D12_SHADER_VISIBILITY_PIXEL);

            CD3DX12_DESCRIPTOR_RANGE1 gbufferSRVs;
            gbufferSRVs.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 6, 0, 2); // t0–t5, space2
            rootParameters[2].InitAsDescriptorTable(1, &gbufferSRVs, D3D12_SHADER_VISIBILITY_PIXEL);

            CD3DX12_DESCRIPTOR_RANGE1 samplerRange;
            samplerRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, 2); // s0, space2
            rootParameters[3].InitAsDescriptorTable(1, &samplerRange, D3D12_SHADER_VISIBILITY_PIXEL);

            CD3DX12_DESCRIPTOR_RANGE1 clusterRanges;
            clusterRanges.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 2, 5); // t2–t3, space5
            rootParameters[4].InitAsDescriptorTable(1, &clusterRanges, D3D12_SHADER_VISIBILITY_PIXEL);

            CD3DX12_STATIC_SAMPLER_DESC staticSamplerDesc(
                0,
                D3D12_FILTER_MIN_MAG_MIP_LINEAR,
                D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                0.0f,
                1,
                D3D12_COMPARISON_FUNC_ALWAYS,
                D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK,
                0.0f, D3D12_FLOAT32_MAX,
                D3D12_SHADER_VISIBILITY_PIXEL,
                2                                 // space2
            );

            D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc = {};
            rootSigDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
            rootSigDesc.Desc_1_1.NumParameters = _countof(rootParameters);
            rootSigDesc.Desc_1_1.pParameters = rootParameters;
            rootSigDesc.Desc_1_1.NumStaticSamplers = 0;
            rootSigDesc.Desc_1_1.pStaticSamplers = nullptr;
            rootSigDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

            ComPtr<ID3DBlob> signatureBlob;
            ComPtr<ID3DBlob> errorBlob;
            HRESULT hr = D3DX12SerializeVersionedRootSignature(
                &rootSigDesc,
                D3D_ROOT_SIGNATURE_VERSION_1_1,
                &signatureBlob,
                &errorBlob
            );

            if (FAILED(hr)) {
                if (errorBlob) {
                    OutputDebugStringA((char*)errorBlob->GetBufferPointer());
                }
            }

            Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateRootSignature(
                0,
                signatureBlob->GetBufferPointer(),
                signatureBlob->GetBufferSize(),
                IID_PPV_ARGS(&m_RootSignature)
            );

        }
        else {
            BRISK_CORE_ERROR("Invalid file name, cannnot generate root signature");
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
            psoDesc.RTVFormats[i] = DXGI_FORMAT_R8G8B8A8_UNORM;
        }
        if (specs.pRenderPass->HasDepth()) {
            psoDesc.DSVFormat = DXGI_FORMAT_D16_UNORM; // TODO: Dont use hardcoded value
            //psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
            D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
            depthStencilDesc.DepthEnable = specs.pDepthTestEnable;
            depthStencilDesc.DepthWriteMask = specs.pDepthWriteEnable ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
            depthStencilDesc.DepthFunc = specs.pCompareOp == CompareOp::COMPARE_OP_LESS ? D3D12_COMPARISON_FUNC_LESS : D3D12_COMPARISON_FUNC_LESS;
            depthStencilDesc.StencilEnable = FALSE;
            psoDesc.DepthStencilState = depthStencilDesc;
        }
        psoDesc.SampleDesc.Count = 1;
        psoDesc.SampleMask = UINT_MAX;

        HRESULT hr = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PipelineState));
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create pipeline state");
        }
	}

	void PipelineDirectX12::Init(const ComputePipelineSpecs& specs) {

	}

	void PipelineDirectX12::Bind(std::shared_ptr<CommandBuffer> cmd) {
        std::static_pointer_cast<CommandBufferDirectX12>(cmd)->Get()->SetPipelineState(m_PipelineState.Get());
        std::static_pointer_cast<CommandBufferDirectX12>(cmd)->Get()->SetGraphicsRootSignature(m_RootSignature.Get());
        std::static_pointer_cast<CommandBufferDirectX12>(cmd)->Get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	void PipelineDirectX12::BindPushConstant(std::shared_ptr<CommandBuffer> cmd, uint32_t, void* data, uint32_t offset, bool vertexShader) {

	}
}