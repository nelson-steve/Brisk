#include "pch.hpp"
#include "SemaphoreDirectX12.hpp"
#include "GpuAdapterDirectX12.hpp"
#include "Engine/Engine.hpp"

namespace Brisk 
{
    void SemaphoreDirectX12::Init()
    {
        D3D12_FENCE_FLAGS fenceFlags = D3D12_FENCE_FLAG_NONE;
        HRESULT hr = Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateFence(0, fenceFlags, IID_PPV_ARGS(&m_Fence));
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create DirectX 12 fence");
        }
    }
}