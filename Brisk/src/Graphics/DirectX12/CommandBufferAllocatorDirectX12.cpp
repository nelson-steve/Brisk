#include "CommandBufferAllocatorDirectX12.hpp"
#include "GpuAdapterDirectX12.hpp"
#include "Engine/Engine.hpp"
#include <stdexcept>

namespace Brisk
{
	void CommandBufferAllocatorDirectX12::Init() {
		HRESULT hr = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator));
		if (FAILED(hr)) {
			throw std::runtime_error("Failed to create DirectX 12 Command Allocator");
		}
	}

	void CommandBufferAllocatorDirectX12::Allocate(std::shared_ptr<CommandBuffer> cmd) {
        ComPtr<ID3D12GraphicsCommandList> commandList;

        HRESULT hr = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            m_CommandAllocator.Get(),
            nullptr,
            IID_PPV_ARGS(&commandList)
        );

        if (FAILED(hr)) {
            throw std::runtime_error("Failed to allocate DirectX 12 Command List");
        }

        commandList->Close();
	}
}