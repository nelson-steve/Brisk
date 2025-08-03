#include "TextureDirectX12.hpp"

#include "Engine/Engine.hpp"
#include "GpuAdapterDirectX12.hpp"
#include "CommandBufferDirectX12.hpp"

#include <fastgltf/core.hpp>

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
        texDesc.MipLevels = 1;
        texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        texDesc.SampleDesc.Count = 1;
        texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        HRESULT hr = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &texDesc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(&m_Texture)
        );

        if (FAILED(hr)) {
            //BRISK_ERROR("Failed to create DirectX12 Texture");
            return;
        }

        // Create SRV Descriptor Heap
        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
        srvHeapDesc.NumDescriptors = 1;
        srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

        hr = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_SRVHeap));
        if (FAILED(hr)) {
            //BRISK_ERROR("Failed to create DirectX12 Texture Descriptor Heap");
            return;
        }

        m_SRVHandleCPU = m_SRVHeap->GetCPUDescriptorHandleForHeapStart();
        m_SRVHandleGPU = m_SRVHeap->GetGPUDescriptorHandleForHeapStart();

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;

        Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateShaderResourceView(m_Texture.Get(), &srvDesc, m_SRVHandleCPU);
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
            //barrier.Transition.StateBefore = param.oldLayout;
            //barrier.Transition.StateAfter = param.newLayout;
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
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        if (!pixels) {
            throw std::runtime_error("Failed to load texture image!");
        }
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

        //CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
        //if (FAILED(m_Device->CreateCommittedResource(
        //    &heapProperties,
        //    D3D12_HEAP_FLAG_NONE,
        //    &textureDesc,
        //    D3D12_RESOURCE_STATE_COPY_DEST,
        //    nullptr,
        //    IID_PPV_ARGS(&m_Texture))
        //)) {
        //    throw std::runtime_error("Failed to create texture resource!");
        //}

        UINT64 uploadBufferSize;
        //m_Device->GetCopyableFootprints(&textureDesc, 0, 1, 0, nullptr, nullptr, nullptr, &uploadBufferSize);

        //CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
        //CD3DX12_RESOURCE_DESC uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
        ComPtr<ID3D12Resource> uploadBuffer;
        //if (FAILED(m_Device->CreateCommittedResource(
        //    &uploadHeapProps,
        //    D3D12_HEAP_FLAG_NONE,
        //    &uploadBufferDesc,
        //    D3D12_RESOURCE_STATE_GENERIC_READ,
        //    nullptr,
        //    IID_PPV_ARGS(&uploadBuffer))
        //)) {
        //    throw std::runtime_error("Failed to create upload buffer!");
        //}

        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = pixels;
        textureData.RowPitch = texWidth * 4;
        textureData.SlicePitch = textureData.RowPitch * texHeight;

        ComPtr<ID3D12GraphicsCommandList6> commandList;
        //m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));

        //UpdateSubresources(commandList.Get(), m_Texture.Get(), uploadBuffer.Get(), 0, 0, 1, &textureData);
        //CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_Texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        //commandList->ResourceBarrier(1, &barrier);

        commandList->Close();

        ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
        //m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        // Wait for upload to finish
        //WaitForGPU();

        stbi_image_free(pixels);
    }
}
