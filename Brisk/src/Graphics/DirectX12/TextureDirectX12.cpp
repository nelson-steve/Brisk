#include "TextureDirectX12.hpp"

#include "Engine/Engine.hpp"
#include "GpuAdapterDirectX12.hpp"
#include "CommandBufferDirectX12.hpp"
#include "UtilitiesDirectX12.hpp"

#include <fastgltf/core.hpp>
#include <Core/Log.hpp>
#include <directx/d3dx12_core.h>
#include <directx/d3dx12_barriers.h>
#include <directx/d3dx12_resource_helpers.h>

namespace Brisk
{
    void TextureDirectX12::Init(const TextureSpecification& specs)
    {
        m_Specs = specs;

        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

        D3D12_RESOURCE_DESC texDesc = {};
        texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        texDesc.Width = specs.p_Width;
        texDesc.Height = specs.p_Height;
        texDesc.DepthOrArraySize = 1;
        texDesc.MipLevels = specs.p_MipLevels;
        texDesc.Format = UtilitiesDirectX12::ToTypeless(UtilitiesDirectX12::FormatToDXGIFormat(specs.p_Format));
        texDesc.SampleDesc.Count = 1;
        texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        texDesc.Flags = specs.p_IsDepth ? D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL : UtilitiesDirectX12::ImageFlagsFromUsage(specs.p_Usage);

        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Format = UtilitiesDirectX12::FormatToDXGIFormat(specs.p_Format);
        clearValue.Color[0] = 1.0f; clearValue.Color[1] = 0.0f; // Red
        clearValue.Color[2] = 0.0f; clearValue.Color[3] = 1.0f;
        clearValue.DepthStencil.Depth = 1.0f;
        clearValue.DepthStencil.Stencil = 0;

        D3D12_RESOURCE_STATES state{};
        if ((specs.p_Usage & Core::TextureUsage::ImageUsageTransferSrc) != Core::TextureUsage::Undefined)
            state = D3D12_RESOURCE_STATE_COPY_SOURCE;
        else if ((specs.p_Usage & Core::TextureUsage::ImageUsageTransferDst) != Core::TextureUsage::Undefined)
            state = D3D12_RESOURCE_STATE_COPY_DEST;
        else if ((specs.p_Usage & Core::TextureUsage::ImageUsageDepthStencilAttachment) != Core::TextureUsage::Undefined)
            state = D3D12_RESOURCE_STATE_DEPTH_WRITE;
        else if ((specs.p_Usage & Core::TextureUsage::ImageUsageColorAttachment) != Core::TextureUsage::Undefined)
            state = D3D12_RESOURCE_STATE_RENDER_TARGET;

        HRESULT hr = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &texDesc,
            state,
            &clearValue,
            IID_PPV_ARGS(&m_Texture)
        );

        if (FAILED(hr)) {
            BRISK_CORE_ERROR("Failed to create DirectX12 Texture");
            return;
        }

        std::wstring wideStrDebugName = std::wstring(specs.p_DebugName.begin(), specs.p_DebugName.end());
        m_Texture->SetName(wideStrDebugName.c_str());

        if ((specs.p_Usage & Core::TextureUsage::ImageUsageDepthStencilAttachment) != Core::TextureUsage::Undefined) {
            m_DsvHandle = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDsvHeap()->GetCPUDescriptorHandleForHeapStart();
            uint32_t descriptorSize = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
            uint32_t index = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetAndIncrementDsvHeapIndex();
            m_DsvHandle.ptr += descriptorSize * index;

            D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
            dsvDesc.Format = UtilitiesDirectX12::FormatToDXGIFormat(specs.p_Format);
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

            Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateDepthStencilView(m_Texture.Get(), &dsvDesc, m_DsvHandle);
        }
        if ((specs.p_Usage & Core::TextureUsage::ImageUsageSampled) != Core::TextureUsage::Undefined) {
            m_SrvHandle = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetCbvSrvUavHeap()->GetCPUDescriptorHandleForHeapStart();
            uint32_t descriptorSize = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            uint32_t index = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetAndIncrementCbvSrvUavHeapIndex();
            m_SrvHandle.ptr += descriptorSize * index;

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = specs.p_IsDepth ? DXGI_FORMAT_R16_UNORM : UtilitiesDirectX12::FormatToDXGIFormat(specs.p_Format);
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Texture2D.MipLevels = 1;

            Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateShaderResourceView(m_Texture.Get(), &srvDesc, m_SrvHandle);
        }
        if ((specs.p_Usage & Core::TextureUsage::ImageUsageColorAttachment) != Core::TextureUsage::Undefined) {
            m_RtvHandle = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetRtvHeap()->GetCPUDescriptorHandleForHeapStart();
            uint32_t descriptorSize = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            uint32_t index = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetAndIncrementRtvHeapIndex();
            m_RtvHandle.ptr += descriptorSize * index;

            D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
            rtvDesc.Format = UtilitiesDirectX12::FormatToDXGIFormat(specs.p_Format);
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

            Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateRenderTargetView(m_Texture.Get(), &rtvDesc, m_RtvHandle);
        }
    }

    void TextureDirectX12::Init(const fastgltf::Image& image, const fastgltf::Asset& asset) {

    }

    void TextureDirectX12::TransitionImageLayout(std::shared_ptr<CommandBuffer> cmd, std::vector<ImageBarrierParams> params)
    {
        auto dxCmdList = std::static_pointer_cast<CommandBufferDirectX12>(cmd)->Get();

        std::vector<D3D12_RESOURCE_BARRIER> barriers;
        for (const auto& param : params)
        {
            D3D12_RESOURCE_BARRIER barrier = {};
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Transition.pResource = m_Texture.Get();
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            barrier.Transition.StateBefore = UtilitiesDirectX12::ImageLayoutToD3D12ResourceState(param.oldLayout);
            barrier.Transition.StateAfter = UtilitiesDirectX12::ImageLayoutToD3D12ResourceState(param.newLayout);
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

            barriers.push_back(barrier);
        }

        dxCmdList->ResourceBarrier(static_cast<UINT>(barriers.size()), barriers.data());
    }

    void TextureDirectX12::CopyImage(std::shared_ptr<CommandBuffer> cmd, std::shared_ptr<Texture> src, std::shared_ptr<Texture> dest, uint32_t width, uint32_t height)
    {
        auto dxCmdList = std::static_pointer_cast<CommandBufferDirectX12>(cmd)->Get();

        D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
        srcLocation.pResource = std::static_pointer_cast<TextureDirectX12>(src)->GetResource();
        srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        srcLocation.SubresourceIndex = 0;

        D3D12_TEXTURE_COPY_LOCATION destLocation = {};
        destLocation.pResource = std::static_pointer_cast<TextureDirectX12>(dest)->GetResource();
        destLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        destLocation.SubresourceIndex = 0;

        dxCmdList->CopyTextureRegion(&destLocation, 0, 0, 0, &srcLocation, nullptr);
    }

    void TextureDirectX12::Init(const std::string& path) {
        ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
        HRESULT hr = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(&m_CommandAllocator));
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        if (!pixels)
            throw std::runtime_error("Failed to load texture image: " + path);

        m_Specs.p_Width = texWidth;
        m_Specs.p_Height = texHeight;

        D3D12_RESOURCE_DESC textureDesc = {};
        textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        textureDesc.Width = texWidth;
        textureDesc.Height = texHeight;
        textureDesc.DepthOrArraySize = 1;
        textureDesc.MipLevels = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
        if (FAILED(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateCommittedResource(
            &defaultHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&m_Texture)
        ))) {
            stbi_image_free(pixels);
            throw std::runtime_error("Failed to create texture resource!");
        }

        UINT64 uploadBufferSize;
        Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->GetCopyableFootprints(&textureDesc, 0, 1, 0, nullptr, nullptr, nullptr, &uploadBufferSize);

        CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
        CD3DX12_RESOURCE_DESC uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

        ComPtr<ID3D12Resource> uploadBuffer;
        if (FAILED(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateCommittedResource(
            &uploadHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &uploadBufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&uploadBuffer)
        ))) {
            stbi_image_free(pixels);
            throw std::runtime_error("Failed to create upload buffer!");
        }

        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = pixels;
        textureData.RowPitch = texWidth * 4;
        textureData.SlicePitch = textureData.RowPitch * texHeight;

        ComPtr<ID3D12GraphicsCommandList> commandList;
        Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateCommandList(
            0, D3D12_COMMAND_LIST_TYPE_DIRECT,
            m_CommandAllocator.Get(),
            nullptr,
            IID_PPV_ARGS(&commandList)
        );

        UpdateSubresources(commandList.Get(), m_Texture.Get(), uploadBuffer.Get(), 0, 0, 1, &textureData);

        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            m_Texture.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
        );
        commandList->ResourceBarrier(1, &barrier);

        commandList->Close();

        ID3D12CommandList* cmdLists[] = { commandList.Get() };
        std::static_pointer_cast<GpuAdapterDirectX12>(Engine::s_Application->GetGpuAdapter())->GetGraphicsQueue()->ExecuteCommandLists(_countof(cmdLists), cmdLists);
        Engine::s_Application->GetGpuAdapter()->WaitIdle();

        stbi_image_free(pixels);
    }
}
