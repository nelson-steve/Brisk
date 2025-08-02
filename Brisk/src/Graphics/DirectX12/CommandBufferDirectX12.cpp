#include "CommandBufferDirectX12.hpp"
#include "GpuAdapterDirectX12.hpp"
#include "Engine/Engine.hpp"

namespace Brisk
{
	void CommandBufferDirectX12::Bind() {
		m_ParentAllocator->Reset();
		m_CommandList->Reset(m_ParentAllocator.Get(), nullptr);
	}

	void CommandBufferDirectX12::UnBind() {
		m_CommandList->Close();
	}

	void CommandBufferDirectX12::Allocate(PoolType type) {
		ComPtr<ID3D12GraphicsCommandList> commandList;

		switch (type)
		{
			case PoolType::Graphics:
				m_ParentAllocator = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetGraphicsCommandAllocator();
				break;
			case PoolType::Compute:
				m_ParentAllocator = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetComputeCommandAllocator();
				break;
			case PoolType::Transfer:
				m_ParentAllocator = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetTransferCommandAllocator();
				break;
		}

		Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateCommandList(
			0,                                 // Node mask (use 0 for single-GPU)
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			m_ParentAllocator.Get(),
			nullptr,
			IID_PPV_ARGS(&commandList));
	}

	void CommandBufferDirectX12::Reset() {
		m_CommandList->Reset(m_ParentAllocator.Get(), nullptr);
	}


	void CommandBufferDirectX12::SetParentAlloator(std::shared_ptr<CommandBufferAllocator> allocator) {
		assert(false);
	}
}	