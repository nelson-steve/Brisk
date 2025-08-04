#include "BufferDirectX12.hpp"
#include <stdexcept>
#include "GpuAdapterDirectX12.hpp"
#include "Engine/Engine.hpp"

#include <directx/d3dx12.h>

namespace Brisk
{
    void BufferDirectX12::Init(uint32_t size, void* data, Core::BufferUsage usageFlags, Core::MemoryProperty memoryProperty, bool mapPersistant) {
        m_Size = size;
        if (Core::HasFlag(memoryProperty, Core::MemoryProperty::HostVisible) && Core::HasFlag(memoryProperty, Core::MemoryProperty::HostCoherent)) {
            D3D12_HEAP_PROPERTIES defaultHeapProps = {};
            defaultHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

            D3D12_RESOURCE_DESC bufferDesc = {};
            bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            bufferDesc.Width = size;
            bufferDesc.Height = 1;
            bufferDesc.DepthOrArraySize = 1;
            bufferDesc.MipLevels = 1;
            bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
            bufferDesc.SampleDesc.Count = 1;
            bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

            D3D12_HEAP_PROPERTIES uploadHeapProps = {};
            uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

            HRESULT hr = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateCommittedResource(
                &uploadHeapProps,
                D3D12_HEAP_FLAG_NONE,
                &bufferDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&m_Buffer)
            );
            if (FAILED(hr)) throw std::runtime_error("Failed to create upload vertex buffer");

            if (mapPersistant) {
                D3D12_RANGE range = { 0, 0 };
                m_Buffer->Map(0, &range, &m_MappedPointer);
                if (data) {
                    memcpy(m_MappedPointer, data, size);
                }
            }
            else {
                D3D12_RANGE range = { 0, 0 };
                m_Buffer->Map(0, &range, &m_MappedPointer);
                if (data) {
                    memcpy(m_MappedPointer, data, size);
                }
                m_Buffer->Unmap(0, nullptr);
            }
        }
        else if (Core::HasFlag(memoryProperty, Core::MemoryProperty::DeviceLocal)) {
            D3D12_HEAP_PROPERTIES defaultHeapProps = {};
            defaultHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

            D3D12_RESOURCE_DESC bufferDesc = {};
            bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            bufferDesc.Width = size;
            bufferDesc.Height = 1;
            bufferDesc.DepthOrArraySize = 1;
            bufferDesc.MipLevels = 1;
            bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
            bufferDesc.SampleDesc.Count = 1;
            bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

            HRESULT hr = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateCommittedResource(
                &defaultHeapProps,
                D3D12_HEAP_FLAG_NONE,
                &bufferDesc,
                D3D12_RESOURCE_STATE_COPY_DEST,
                nullptr,
                IID_PPV_ARGS(&m_Buffer)
            );
            if (FAILED(hr)) throw std::runtime_error("Failed to create GPU vertex buffer");

            D3D12_HEAP_PROPERTIES uploadHeapProps = {};
            uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

            Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferUpload;
            hr = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateCommittedResource(
                &uploadHeapProps,
                D3D12_HEAP_FLAG_NONE,
                &bufferDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&vertexBufferUpload)
            );
            if (FAILED(hr)) throw std::runtime_error("Failed to create upload vertex buffer");

            void* mappedPtr = nullptr;
            D3D12_RANGE range = { 0, 0 };
            vertexBufferUpload->Map(0, &range, &mappedPtr);
            if(data)
                memcpy(mappedPtr, data, size);
            vertexBufferUpload->Unmap(0, nullptr);

            D3D12_SUBRESOURCE_DATA subresourceData = {};
            subresourceData.pData = data;
            subresourceData.RowPitch = size;
            subresourceData.SlicePitch = size;

            ComPtr<ID3D12CommandAllocator> commandAllocator;
            ComPtr<ID3D12GraphicsCommandList> commandList;

            Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
            Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));

            UpdateSubresources(commandList.Get(), m_Buffer.Get(), vertexBufferUpload.Get(), 0, 0, 1, &subresourceData);

            D3D12_RESOURCE_BARRIER barrier = {};
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Transition.pResource = m_Buffer.Get();
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(1, &barrier);

            commandList->Close();

            ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
            Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetGraphicsQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

            ComPtr<ID3D12Fence> fence;
            UINT64 fenceValue = 1;
            Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
            HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

            Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetGraphicsQueue()->Signal(fence.Get(), fenceValue);
            if (fence->GetCompletedValue() < fenceValue) {
                fence->SetEventOnCompletion(fenceValue, fenceEvent);
                WaitForSingleObject(fenceEvent, INFINITE);
            }
            CloseHandle(fenceEvent);

            commandAllocator->Reset();
            commandList->Reset(commandAllocator.Get(), nullptr);
        }
    }

    void BufferDirectX12::Release() {
        if (m_Buffer) {
            m_Buffer->Release();
            m_Buffer = nullptr;
        }
    }

    void BufferDirectX12::UpdatePersistantData(uint32_t size, void* data) {
        if (m_MapPersistent && m_MappedPointer) {
            memcpy(m_MappedPointer, data, size);
        }
    }
}