#include "pch.hpp"
#include "BufferDirectX12.hpp"
#include "GpuAdapterDirectX12.hpp"
#include "Engine/Engine.hpp"
#include "UtilitiesDirectX12.hpp"

#include <directx/d3dx12.h>

namespace Brisk
{
    void BufferDirectX12::Init(const BufferDesc& desc) {
        D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT;

        bool constantBuffer = false;
        bool shaderResource = false;
        bool unordererdAcess = false;

        switch (desc.p_Memory)
        {
            case BufferDesc::MemoryUsage::CPU_Only:
                heapType = D3D12_HEAP_TYPE_CUSTOM;
                break;
            case BufferDesc::MemoryUsage::GPU_Only:
                heapType = D3D12_HEAP_TYPE_DEFAULT;
                shaderResource = true;
                break;
            case BufferDesc::MemoryUsage::CPU_To_GPU:
                heapType = D3D12_HEAP_TYPE_UPLOAD;
                constantBuffer = true;
                break;
            case BufferDesc::MemoryUsage::GPU_To_CPU:
                heapType = D3D12_HEAP_TYPE_READBACK;
                break;
        }
        
        // Create Consant Buffer
        if(desc.p_Memory == BufferDesc::MemoryUsage::CPU_Only)
        {
            // Determine resource flags
            D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
            if (desc.p_AllowUAV)      flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            if (desc.p_Usage == Core::BufferUsage::StorageBuffer)
                flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

            // Size must be aligned to 256 bytes for constant buffers
            UINT64 bufferSize = desc.p_Size;
            if (desc.p_Usage == Core::BufferUsage::UniformBuffer)
                bufferSize = (bufferSize + 255) & ~255ULL;

            // Describe the buffer
            D3D12_RESOURCE_DESC resourceDesc = {};
            resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            resourceDesc.Alignment = 0;
            resourceDesc.Width = bufferSize;
            resourceDesc.Height = 1;
            resourceDesc.DepthOrArraySize = 1;
            resourceDesc.MipLevels = 1;
            resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
            resourceDesc.SampleDesc.Count = 1;
            resourceDesc.SampleDesc.Quality = 0;
            resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            resourceDesc.Flags = flags;

            // Heap properties
            D3D12_HEAP_PROPERTIES heapProps = {};
            heapProps.Type = heapType;
            heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            heapProps.CreationNodeMask = 1;
            heapProps.VisibleNodeMask = 1;

            // Create resource
            HRESULT hr = Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateCommittedResource(
                &heapProps,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                (heapType == D3D12_HEAP_TYPE_UPLOAD) ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_COMMON,
                nullptr,
                IID_PPV_ARGS(&m_Buffer)
            );

            if (FAILED(hr))
                throw std::runtime_error("Failed to create buffer: " + std::to_string(hr));

            // Optionally persistently map upload/readback buffers
            if (desc.p_Persistant && (heapType == D3D12_HEAP_TYPE_UPLOAD || heapType == D3D12_HEAP_TYPE_READBACK))
            {
                void* mappedData = nullptr;
                m_Buffer->Map(0, nullptr, &mappedData);
                if (desc.p_Data && mappedData)
                    memcpy(mappedData, desc.p_Data, desc.p_Size);
            }
            else if (desc.p_Data && heapType == D3D12_HEAP_TYPE_UPLOAD)
            {
                void* mappedData = nullptr;
                m_Buffer->Map(0, nullptr, &mappedData);
                memcpy(mappedData, desc.p_Data, desc.p_Size);
                m_Buffer->Unmap(0, nullptr);
            }

            uint32_t index = Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetAndIncrementCbvSrvUavHeapIndex();

            m_CpuHandle = Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetCbvSrvUavHeap()->GetCPUDescriptorHandleForHeapStart();
            uint32_t descriptorSize = Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            m_CpuHandle.ptr += descriptorSize * index;

            m_GpuHandle = Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetCbvSrvUavHeap()->GetGPUDescriptorHandleForHeapStart();
            descriptorSize = Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            m_GpuHandle.ptr += descriptorSize * index;

            D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
            cbvDesc.BufferLocation = m_Buffer->GetGPUVirtualAddress();
            cbvDesc.SizeInBytes = (desc.p_Size + 255) & ~255;

            Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateConstantBufferView(&cbvDesc, m_CpuHandle);
        }

        // Create SRV buffer
        if (desc.p_Memory == BufferDesc::MemoryUsage::GPU_Only && desc.p_AllowSRV)
        {
            D3D12_HEAP_PROPERTIES defaultHeapProps = {};
            defaultHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

            D3D12_RESOURCE_DESC bufferDesc = {};
            bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            bufferDesc.Width = desc.p_Size;
            bufferDesc.Height = 1;
            bufferDesc.DepthOrArraySize = 1;
            bufferDesc.MipLevels = 1;
            bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
            bufferDesc.SampleDesc.Count = 1;
            bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

            if (desc.p_Data) {
                HRESULT hr = Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateCommittedResource(
                    &defaultHeapProps,
                    D3D12_HEAP_FLAG_NONE,
                    &bufferDesc,
                    D3D12_RESOURCE_STATE_COPY_DEST,
                    nullptr,
                    IID_PPV_ARGS(&m_Buffer)
                );
                if (FAILED(hr)) throw std::runtime_error("Failed to create GPU buffer");

                D3D12_HEAP_PROPERTIES uploadHeapProps = {};
                uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

                bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
                Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferUpload;
                hr = Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateCommittedResource(
                    &uploadHeapProps,
                    D3D12_HEAP_FLAG_NONE,
                    &bufferDesc,
                    D3D12_RESOURCE_STATE_GENERIC_READ,
                    nullptr,
                    IID_PPV_ARGS(&vertexBufferUpload)
                );
                if (FAILED(hr)) throw std::runtime_error("Failed to create upload buffer");

                void* mappedPtr = nullptr;
                D3D12_RANGE range = { 0, 0 };
                vertexBufferUpload->Map(0, &range, &mappedPtr);
                if (desc.p_Data)
                    memcpy(mappedPtr, desc.p_Data, desc.p_Size);
                vertexBufferUpload->Unmap(0, nullptr);

                D3D12_SUBRESOURCE_DATA subresourceData = {};
                subresourceData.pData = desc.p_Data;
                subresourceData.RowPitch = desc.p_Size;
                subresourceData.SlicePitch = desc.p_Size;

                ComPtr<ID3D12CommandAllocator> commandAllocator;
                ComPtr<ID3D12GraphicsCommandList> commandList;

                Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
                Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));

                UpdateSubresources(commandList.Get(), m_Buffer.Get(), vertexBufferUpload.Get(), 0, 0, 1, &subresourceData);

                D3D12_RESOURCE_BARRIER barrier = {};
                barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barrier.Transition.pResource = m_Buffer.Get();
                barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
                barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
                barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

                commandList->ResourceBarrier(1, &barrier);

                commandList->Close();

                ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
                Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetGraphicsQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

                ComPtr<ID3D12Fence> fence;
                UINT64 fenceValue = 1;
                Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
                HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

                Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetGraphicsQueue()->Signal(fence.Get(), fenceValue);
                if (fence->GetCompletedValue() < fenceValue) {
                    fence->SetEventOnCompletion(fenceValue, fenceEvent);
                    WaitForSingleObject(fenceEvent, INFINITE);
                }
                CloseHandle(fenceEvent);

                commandAllocator->Reset();
                commandList->Reset(commandAllocator.Get(), nullptr);
            }
            else {
                HRESULT hr = Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateCommittedResource(
                    &defaultHeapProps,
                    D3D12_HEAP_FLAG_NONE,
                    &bufferDesc,
                    D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                    nullptr,
                    IID_PPV_ARGS(&m_Buffer)
                );
                if (FAILED(hr)) throw std::runtime_error("Failed to create GPU buffer");
            }

            uint32_t index = Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetAndIncrementCbvSrvUavHeapIndex();

            m_CpuHandle = Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetCbvSrvUavHeap()->GetCPUDescriptorHandleForHeapStart();
            uint32_t descriptorSize = Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            m_CpuHandle.ptr += descriptorSize * index;

            m_GpuHandle = Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetCbvSrvUavHeap()->GetGPUDescriptorHandleForHeapStart();
            descriptorSize = Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            m_GpuHandle.ptr += descriptorSize * index;

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Format = DXGI_FORMAT_UNKNOWN; // Structured buffer has no fixed format
            srvDesc.Buffer.FirstElement = 0;
            //srvDesc.Buffer.NumElements = elementCount;
            srvDesc.Buffer.StructureByteStride = sizeof(MaterialData);
            srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

            Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateShaderResourceView(m_Buffer.Get(), &srvDesc, m_CpuHandle);
        }

        // Create UAV buffer
        if (desc.p_Memory == BufferDesc::MemoryUsage::GPU_Only && desc.p_AllowUAV)
        {
            D3D12_HEAP_PROPERTIES defaultHeapProps = {};
            defaultHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

            D3D12_RESOURCE_DESC bufferDesc = {};
            bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            bufferDesc.Width = desc.p_Size;
            bufferDesc.Height = 1;
            bufferDesc.DepthOrArraySize = 1;
            bufferDesc.MipLevels = 1;
            bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
            bufferDesc.SampleDesc.Count = 1;
            bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

            if (desc.p_Data) {
                HRESULT hr = Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateCommittedResource(
                    &defaultHeapProps,
                    D3D12_HEAP_FLAG_NONE,
                    &bufferDesc,
                    D3D12_RESOURCE_STATE_COPY_DEST,
                    nullptr,
                    IID_PPV_ARGS(&m_Buffer)
                );
                if (FAILED(hr)) throw std::runtime_error("Failed to create GPU buffer");

                D3D12_HEAP_PROPERTIES uploadHeapProps = {};
                uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

                bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
                Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferUpload;
                hr = Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateCommittedResource(
                    &uploadHeapProps,
                    D3D12_HEAP_FLAG_NONE,
                    &bufferDesc,
                    D3D12_RESOURCE_STATE_GENERIC_READ,
                    nullptr,
                    IID_PPV_ARGS(&vertexBufferUpload)
                );
                if (FAILED(hr)) throw std::runtime_error("Failed to create upload buffer");

                void* mappedPtr = nullptr;
                D3D12_RANGE range = { 0, 0 };
                vertexBufferUpload->Map(0, &range, &mappedPtr);
                if (desc.p_Data)
                    memcpy(mappedPtr, desc.p_Data, desc.p_Size);
                vertexBufferUpload->Unmap(0, nullptr);

                D3D12_SUBRESOURCE_DATA subresourceData = {};
                subresourceData.pData = desc.p_Data;
                subresourceData.RowPitch = desc.p_Size;
                subresourceData.SlicePitch = desc.p_Size;

                ComPtr<ID3D12CommandAllocator> commandAllocator;
                ComPtr<ID3D12GraphicsCommandList> commandList;

                Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
                Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));

                UpdateSubresources(commandList.Get(), m_Buffer.Get(), vertexBufferUpload.Get(), 0, 0, 1, &subresourceData);

                D3D12_RESOURCE_BARRIER barrier = {};
                barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barrier.Transition.pResource = m_Buffer.Get();
                barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
                barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
                barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

                commandList->ResourceBarrier(1, &barrier);

                commandList->Close();

                ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
                Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetGraphicsQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

                ComPtr<ID3D12Fence> fence;
                UINT64 fenceValue = 1;
                Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
                HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

                Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetGraphicsQueue()->Signal(fence.Get(), fenceValue);
                if (fence->GetCompletedValue() < fenceValue) {
                    fence->SetEventOnCompletion(fenceValue, fenceEvent);
                    WaitForSingleObject(fenceEvent, INFINITE);
                }
                CloseHandle(fenceEvent);

                commandAllocator->Reset();
                commandList->Reset(commandAllocator.Get(), nullptr);
            }
            else {
                HRESULT hr = Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateCommittedResource(
                    &defaultHeapProps,
                    D3D12_HEAP_FLAG_NONE,
                    &bufferDesc,
                    D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                    nullptr,
                    IID_PPV_ARGS(&m_Buffer)
                );
                if (FAILED(hr)) throw std::runtime_error("Failed to create GPU buffer");
            }

            uint32_t index = Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetAndIncrementCbvSrvUavHeapIndex();

            m_CpuHandle = Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetCbvSrvUavHeap()->GetCPUDescriptorHandleForHeapStart();
            uint32_t descriptorSize = Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            m_CpuHandle.ptr += descriptorSize * index;

            m_GpuHandle = Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetCbvSrvUavHeap()->GetGPUDescriptorHandleForHeapStart();
            descriptorSize = Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            m_GpuHandle.ptr += descriptorSize * index;

            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
            uavDesc.Format = DXGI_FORMAT_UNKNOWN; // Structured buffer has no fixed format
            uavDesc.Buffer.FirstElement = 0;
            //srvDesc.Buffer.NumElements = elementCount;
            uavDesc.Buffer.StructureByteStride = sizeof(MaterialData);
            uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

            Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateUnorderedAccessView(m_Buffer.Get(), nullptr, &uavDesc, m_CpuHandle);
        }
    }

    void BufferDirectX12::Release() {
        if (m_Buffer) {
            m_Buffer->Release();
            m_Buffer = nullptr;
        }
    }

    void BufferDirectX12::UpdatePersistantData(uint32_t size, void* data, uint64_t offset) {
        if (m_MappedPointer) {
            memcpy(m_MappedPointer, data, size);
        }
    }
}