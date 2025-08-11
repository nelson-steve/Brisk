#pragma once

#include "Engine/Renderer/Semaphore.hpp"
#include <wrl.h>
#include <directx/d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <Core/Log.hpp>
//--------------------
using Microsoft::WRL::ComPtr;

namespace Brisk
{
    class SemaphoreDirectX12 : public Semaphore
    {
    public:
        void Init() override;
        void Release() override { BRISK_CORE_ASSERT(false); }

        ComPtr<ID3D12Fence> GetFence() const { return m_Fence; }
    private:
        ComPtr<ID3D12Fence> m_Fence;
    };
}
