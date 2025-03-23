#pragma once

#include "Engine/Renderer/Queue.hpp"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

namespace Brisk
{
    class QueueDirectX12 : public Queue {
    public:
        virtual void Submit(SubmitInfo submitInfo, std::shared_ptr<Fence> fence) override;
        virtual void Present(Queue::PresentInfo info) override;
    private:
        ComPtr<ID3D12CommandQueue> m_CommandQueue;
    };
}