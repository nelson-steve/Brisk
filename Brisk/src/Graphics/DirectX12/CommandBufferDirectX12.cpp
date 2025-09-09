#include "CommandBufferDirectX12.hpp"
#include "GpuAdapterDirectX12.hpp"
#include "Engine/Engine.hpp"
#include <Core/Log.hpp>

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
		HRESULT hr;
		switch (type)
		{
			case PoolType::Graphics:
				hr = Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateCommandAllocator(
					D3D12_COMMAND_LIST_TYPE_DIRECT,
					IID_PPV_ARGS(&m_ParentAllocator));
				break;
			case PoolType::Compute:
				hr = Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateCommandAllocator(
					D3D12_COMMAND_LIST_TYPE_COMPUTE,
					IID_PPV_ARGS(&m_ParentAllocator));
				break;
			case PoolType::Transfer:
				hr = Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateCommandAllocator(
					D3D12_COMMAND_LIST_TYPE_COPY,
					IID_PPV_ARGS(&m_ParentAllocator));
				break;
		}

		if (FAILED(hr)) {
			BRISK_CORE_ERROR("Failed to create Graphics command allocator");
		}

		Application::GetGpuAdapter()->GetDevice<GpuAdapterDirectX12>()->GetDevice()->CreateCommandList(
			0,                                 // Node mask (use 0 for single-GPU)
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			m_ParentAllocator.Get(),
			nullptr,
			IID_PPV_ARGS(&m_CommandList));

		m_CommandList->Close();
	}

	void CommandBufferDirectX12::Reset() {
		//m_CommandList->Reset(m_ParentAllocator.Get(), nullptr);
	}


	void CommandBufferDirectX12::SetParentAlloator(std::shared_ptr<CommandBufferAllocator> allocator) {
		BRISK_CORE_ASSERT(false);
	}
}	