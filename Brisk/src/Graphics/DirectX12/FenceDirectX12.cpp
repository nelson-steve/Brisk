#include "pch.hpp"
#include "FenceDirectX12.hpp"
#include "GpuAdapterDirectX12.hpp"
#include "Engine/Engine.hpp"

namespace Brisk
{
	void FenceDirectX12::Init() {
		HRESULT hr = std::static_pointer_cast<GpuAdapterDirectX12>(Application::GetGpuAdapter())->GetDevice()->CreateFence(m_FenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence));
		if (FAILED(hr)) {
			throw std::runtime_error("Failed to create fence.");
		}

		// Create a Win32 event for waiting
		m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_FenceEvent == nullptr) {
			throw std::runtime_error("Failed to create fence event.");
		}
	}

	void FenceDirectX12::Wait() {
		if (m_Fence->GetCompletedValue() < m_FenceValue) {
			m_Fence->SetEventOnCompletion(m_FenceValue, m_FenceEvent);
			WaitForSingleObject(m_FenceEvent, INFINITE);
		}
	}

	void FenceDirectX12::Reset() {
		m_FenceValue++;
	}

	void FenceDirectX12::Release() {

	}
}