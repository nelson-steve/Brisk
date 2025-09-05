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
#include "BufferDirectX12.hpp"

#include <directx/d3d12shader.h>

namespace Brisk
{
	void PipelineDirectX12::Init(const GraphicsPipelineSpecs& specs) {
        HRESULT hr;
        ComPtr<ID3DBlob> vertexShaderBlob;
        ComPtr<ID3DBlob> fragmentShaderBlob;

        std::vector<D3D12_ROOT_PARAMETER> rootParams;
        std::vector<D3D12_DESCRIPTOR_RANGE> descriptorRanges;
        descriptorRanges.reserve(100);

        for (const std::string& path : specs.pShaderPathsDX) {
            std::wstring wideStr = std::wstring(path.begin(), path.end());
            LPCWSTR pathWstring = wideStr.c_str();
            std::wstring shaderPath = std::filesystem::current_path().wstring() + pathWstring;

            std::vector<char>* shaderData = UtilitiesDirectX12::ReadShaderFile(path);

            ComPtr<IDxcUtils> utils;
            DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));

            DxcBuffer buffer{};
            buffer.Ptr = shaderData->data();
            buffer.Size = shaderData->size();
            buffer.Encoding = DXC_CP_ACP;

            ComPtr<ID3D12ShaderReflection> reflection;
            utils->CreateReflection(&buffer, IID_PPV_ARGS(&reflection));

            // Query shader description
            D3D12_SHADER_DESC shaderDesc;
            reflection->GetDesc(&shaderDesc);

            std::cout << "Shader Input Parameters: " << shaderDesc.InputParameters << "\n";
            std::cout << "Constant Buffers: " << shaderDesc.ConstantBuffers << "\n";
            std::cout << "Bound Resources: " << shaderDesc.BoundResources << "\n";

            for (UINT i = 0; i < shaderDesc.BoundResources; ++i)
            {
                D3D12_SHADER_INPUT_BIND_DESC bindDesc;
                reflection->GetResourceBindingDesc(i, &bindDesc);

                std::cout << "Resource " << i << ": " << bindDesc.Name
                    << ", Type = " << bindDesc.Type
                    << ", Bind Point = " << bindDesc.BindPoint << "\n";
            }

            for (UINT i = 0; i < shaderDesc.BoundResources; ++i)
            {
                D3D12_SHADER_INPUT_BIND_DESC bindDesc{};
                reflection->GetResourceBindingDesc(i, &bindDesc);

                D3D12_DESCRIPTOR_RANGE range{};

                switch (bindDesc.Type)
                {
                    case D3D_SIT_CBUFFER:
                        range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
                        break;
                    case D3D_SIT_TBUFFER: // treat like SRV
                    case D3D_SIT_TEXTURE:
                    case D3D_SIT_STRUCTURED:
                    case D3D_SIT_BYTEADDRESS:
                        range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                        break;
                    case D3D_SIT_UAV_RWTYPED:
                    case D3D_SIT_UAV_RWSTRUCTURED:
                    case D3D_SIT_UAV_RWBYTEADDRESS:
                        range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
                        break;
                    case D3D_SIT_SAMPLER:
                        range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
                        break;
                    default:
                        continue; // skip unknown types
                }

                range.NumDescriptors = 1;
                range.BaseShaderRegister = bindDesc.BindPoint;
                range.RegisterSpace = bindDesc.Space;
                range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

                descriptorRanges.push_back(range);

                CD3DX12_ROOT_PARAMETER param{};
                param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                param.DescriptorTable.NumDescriptorRanges = 1;
                param.DescriptorTable.pDescriptorRanges = &descriptorRanges.back();
                param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

                rootParams.push_back(param);

                ShaderResource shaderResource{};
                shaderResource.p_Name = bindDesc.Name;
                shaderResource.p_Space = bindDesc.Space;
                shaderResource.p_Binding = bindDesc.BindPoint;
                shaderResource.p_RootIndex = rootParams.size() - 1;

                m_ShaderResources.push_back(shaderResource);

                switch (bindDesc.Type)
                {
                    case D3D_SIT_CBUFFER:
                        param.InitAsConstantBufferView(bindDesc.BindPoint, bindDesc.Space);
                        break;
                    case D3D_SIT_TBUFFER: // treat like SRV
                    case D3D_SIT_TEXTURE:
                    case D3D_SIT_STRUCTURED:
                    case D3D_SIT_BYTEADDRESS:
                        break;
                    case D3D_SIT_UAV_RWTYPED:
                    case D3D_SIT_UAV_RWSTRUCTURED:
                    case D3D_SIT_UAV_RWBYTEADDRESS:
                        param.InitAsUnorderedAccessView(bindDesc.BindPoint, bindDesc.Space);
                        break;
                    case D3D_SIT_SAMPLER:
                        break;
                    default:
                        BRISK_CORE_WARN("Unknown resource type");
                        break;
                }
            }

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

        D3D12_ROOT_SIGNATURE_DESC rootSigDesc{};
        rootSigDesc.NumParameters = (UINT)rootParams.size();
        rootSigDesc.pParameters = rootParams.data();
        rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        ComPtr<ID3DBlob> serialized;
        ComPtr<ID3DBlob> errors;
        hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
            &serialized, &errors);
        if (FAILED(hr))
        {
            if (errors)
                std::cerr << (char*)errors->GetBufferPointer() << "\n";
            throw std::runtime_error("Failed to serialize root signature");
        }

        Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateRootSignature(0, serialized->GetBufferPointer(),
            serialized->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));

        std::cout << std::endl;

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

        hr = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PipelineState));
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create pipeline state");
        }
	}

	void PipelineDirectX12::Init(const ComputePipelineSpecs& specs) {
        CD3DX12_DESCRIPTOR_RANGE ranges[2];
        ranges[0] .Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);  // SRV t0
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);  // UAV u0

        CD3DX12_ROOT_PARAMETER rootParams[2];
        rootParams[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);
        rootParams[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_ALL);

        CD3DX12_ROOT_PARAMETER cbvParam;
        cbvParam.InitAsConstantBufferView(0); // CBV at b0

        CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc;
        rootSigDesc.Init(_countof(rootParams), rootParams, 0, nullptr,
            D3D12_ROOT_SIGNATURE_FLAG_NONE);

        ComPtr<ID3DBlob> serializedRootSig = nullptr;
        ComPtr<ID3DBlob> errorBlob = nullptr;
        if (FAILED(D3D12SerializeRootSignature(&rootSigDesc,
            D3D_ROOT_SIGNATURE_VERSION_1,
            &serializedRootSig, &errorBlob))) {
            throw std::runtime_error("Failed to serialize root signature");
        }
        if (FAILED(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateRootSignature(0, serializedRootSig->GetBufferPointer(),
            serializedRootSig->GetBufferSize(),
            IID_PPV_ARGS(&m_RootSignature)))) {
            throw std::runtime_error("Failed to create root signature");
        }

        std::wstring wideStr = std::wstring(specs.pShaderPath.begin(), specs.pShaderPath.end());
        LPCWSTR pathWstring = wideStr.c_str();
        std::wstring shaderPath = std::filesystem::current_path().wstring() + pathWstring;

        ComPtr<ID3DBlob> csBlob;
        if (FAILED(D3DReadFileToBlob(shaderPath.c_str(), &csBlob))) {
            throw std::runtime_error("Failed to read file to blob");
        }

        D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
        computePsoDesc.pRootSignature = m_RootSignature.Get();
        computePsoDesc.CS = { csBlob->GetBufferPointer(), csBlob->GetBufferSize() };
        computePsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

        if (FAILED(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&m_PipelineState)))) {
            throw std::runtime_error("Failed to create pipeline state");
        }
	}

	void PipelineDirectX12::Bind(std::shared_ptr<CommandBuffer> cmd) {
        ID3D12DescriptorHeap* heaps[] = 
        { 
            Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetCbvSrvUavHeap().Get(),
            Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetSamplerHeap().Get(),
        };
        std::static_pointer_cast<CommandBufferDirectX12>(cmd)->Get()->SetDescriptorHeaps(_countof(heaps), heaps);

        std::static_pointer_cast<CommandBufferDirectX12>(cmd)->Get()->SetPipelineState(m_PipelineState.Get());
        std::static_pointer_cast<CommandBufferDirectX12>(cmd)->Get()->SetGraphicsRootSignature(m_RootSignature.Get());

        for (int i = 0; i < m_ShaderResources.size(); ++i) {
            if (m_ShaderResources[i].m_Buffer) {
                std::static_pointer_cast<CommandBufferDirectX12>(cmd)->Get()->SetGraphicsRootDescriptorTable(
                    m_ShaderResources[i].p_RootIndex,
                    std::static_pointer_cast<BufferDirectX12>(m_ShaderResources[i].m_Buffer)->GetGpuHandle());
            }
        }

        std::static_pointer_cast<CommandBufferDirectX12>(cmd)->Get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

    void PipelineDirectX12::UpdateResources(const std::string& name, std::vector<std::shared_ptr<Texture>> textures, std::shared_ptr<Buffer> buffer) {
        BRISK_CORE_ASSERT(name != "");
        BRISK_CORE_ASSERT(textures.size() > 0 || buffer != nullptr);
        bool found = false;
        for (int i = 0; i < m_ShaderResources.size(); ++i) {
            if (m_ShaderResources[i].p_Name == name) {
                found = true;
                if (buffer != nullptr) {
                    m_ShaderResources[i].m_Buffer = buffer;
                }
            }
        }

        BRISK_CORE_ASSERT(found);
    }

	void PipelineDirectX12::BindPushConstant(std::shared_ptr<CommandBuffer> cmd, uint32_t, void* data, uint32_t offset, Core::ShaderStageFlags stages) {

	}
}