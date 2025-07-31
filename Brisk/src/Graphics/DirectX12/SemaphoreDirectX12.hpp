#pragma once

#include "Engine/Renderer/Semaphore.hpp"
#include <cassert>
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
//--------------------
using Microsoft::WRL::ComPtr;

namespace Brisk
{
    class SemaphoreDirectX12 : public Semaphore
    {
    public:
        void Init() override;
        void Release() override { assert(false); }

        ComPtr<ID3D12Fence> GetFence() const { return m_Fence; }
    private:
        ComPtr<ID3D12Fence> m_Fence;
    };
}
