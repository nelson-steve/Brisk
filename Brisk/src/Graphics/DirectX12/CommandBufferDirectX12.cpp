#include "CommandBufferDirectX12.hpp"
#include "GpuAdapterDirectX12.hpp"
#include "Engine/Engine.hpp"

namespace Brisk
{
	void CommandBufferDirectX12::Allocate(ComPtr<ID3D12CommandAllocator> allocator) {
		m_ParentAllocator = allocator;

		// Create Command Allocator
		std::static_pointer_cast<GpuAdapterDirectX12>(Engine::s_Application->GetGpuAdapter())->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_ParentAllocator));

		// Create Command List
		Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateCommandList(
			0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_ParentAllocator.Get(), nullptr, IID_PPV_ARGS(&m_CommandList));

		// Command list starts open, close it initially
		m_CommandList->Close();
	}
	void CommandBufferDirectX12::Bind() {
		m_ParentAllocator->Reset();
		m_CommandList->Reset(m_ParentAllocator.Get(), nullptr);
	}

	void CommandBufferDirectX12::UnBind() {
		m_CommandList->Close();
	}
}	