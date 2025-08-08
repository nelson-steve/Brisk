#include "QueueDirectX12.hpp"
#include <Core/Log.hpp>
#include "GpuAdapterDirectX12.hpp"
#include "Engine/Engine.hpp"
#include "CommandBufferDirectX12.hpp"
#include "FenceDirectX12.hpp"
#include "SwapchainDirectX12.hpp"

namespace Brisk
{
    void QueueDirectX12::Init(Queue::QueueType type) {
        switch (type)
        {
            case Brisk::Queue::QueueType::Graphics:
                m_CommandQueue = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetGraphicsQueue();
                break;
            case Brisk::Queue::QueueType::Compute:
                m_CommandQueue = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetComputeQueue();
                break;
            case Brisk::Queue::QueueType::Transfer:
                m_CommandQueue = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetTransferQueue();
                break;
            default:
                BRISK_CORE_ERROR("Invalid Queue Type");
                break;
        }
    }

    void QueueDirectX12::Submit(SubmitInfo submitInfo, std::shared_ptr<Fence> fence) {
        std::vector<ID3D12GraphicsCommandList6*> commandBuffers(submitInfo.pCmdBuffers.size());
        for (size_t i = 0; i < submitInfo.pCmdBuffers.size(); ++i) {
            commandBuffers[i] = std::static_pointer_cast<CommandBufferDirectX12>(submitInfo.pCmdBuffers[i])->Get().Get();
        }
        std::vector<ID3D12CommandList*> baseCommandLists(commandBuffers.begin(), commandBuffers.end());

        m_CommandQueue->ExecuteCommandLists(1, baseCommandLists.data());

        if (fence) {
            fence->Reset();
            m_CommandQueue->Signal(std::static_pointer_cast<FenceDirectX12>(fence)->Get().Get(), std::static_pointer_cast<FenceDirectX12>(fence)->GetValue());
        }
    }

    void QueueDirectX12::Present(Queue::PresentInfo info) {
        std::static_pointer_cast<SwapchainDirectX12>(Engine::s_Application->GetRenderer()->GetSwapchain())->Present();
    }
}