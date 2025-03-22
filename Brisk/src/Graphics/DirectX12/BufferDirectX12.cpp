#include "BufferDirectX12.hpp"
#include <stdexcept>
#include "GpuAdapterDirectX12.hpp"
#include "Engine/Engine.hpp"

namespace Brisk
{
    void BufferVulkan::Init(uint64_t size,
        void* data,
        std::vector<Core::BufferUsage> usageFlags,
        std::vector<Core::MemoryProperty> memoryProperty,
        bool mapPersistent) {
        m_Size = size;
        m_MapPersistent = mapPersistent;

        Create(size);

        if (data) {
            UploadData(data, size);
        }
    }

    void BufferVulkan::Release() {
        if (m_Buffer) {
            m_Buffer->Release();
            m_Buffer = nullptr;
        }
    }

    void BufferVulkan::UpdatePersistantData(uint64_t size, void* data) {
        if (m_MapPersistent && m_MappedPointer) {
            memcpy(m_MappedPointer, data, size);
        }
    }

    void BufferVulkan::Create(uint64_t size) {
        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

        D3D12_RESOURCE_DESC bufferDesc = {};
        bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        bufferDesc.Width = AlignSize(size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
        bufferDesc.Height = 1;
        bufferDesc.DepthOrArraySize = 1;
        bufferDesc.MipLevels = 1;
        bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
        bufferDesc.SampleDesc.Count = 1;
        bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        HRESULT hr = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_Buffer)
        );

        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create DirectX 12 buffer.");
        }

        if (m_MapPersistent) {
            m_Buffer->Map(0, nullptr, &m_MappedPointer);
        }
    }
}