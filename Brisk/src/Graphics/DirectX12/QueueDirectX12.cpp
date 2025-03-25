#include "QueueDirectX12.hpp"

namespace Brisk
{
    void QueueDirectX12::Submit(SubmitInfo submitInfo, std::shared_ptr<Fence> fence) {
        //ID3D12CommandList* commandLists[] = { submitInfo.pCmdBuffers[0]->Get()};

        //m_CommandQueue->ExecuteCommandLists(1, commandLists);

        if (fence) {
            //m_CommandQueue->Signal(fence->Get(), fence->GetValue());
        }
    }

    void QueueDirectX12::Present(Queue::PresentInfo info) {
        //HRESULT hr = info.pSwapchains[0]->Present(info.vsync ? 1 : 0, 0);
        //if (FAILED(hr)) {
        //    throw std::runtime_error("Failed to present swap chain in DirectX 12.");
        //}
    }
}