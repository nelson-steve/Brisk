#pragma once

#include "Engine/Renderer/GpuAdapter.hpp"

#include <cassert>

#include <directx/d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

namespace Brisk
{
	class GpuAdapterDirectX12 : public GpuAdapter {
	public:
		virtual void Init() override;

		virtual void WaitIdle() override { assert(false); }
		virtual void Release() override { assert(false); }
		virtual void ReleasePools() override { assert(false); }
		virtual void LogDirectXDebugs() override;

		ComPtr<IDXGIFactory6> GetDXGIFactory() const { return m_DxgiFactory; }
		ComPtr<ID3D12Device> GetDevice() const { return m_Device; }
		ID3D12CommandQueue* GetGraphicsQueue() const { return m_GraphicsQueue.Get(); }
		ID3D12CommandQueue* GetComputeQueue() const { return m_ComputeQueue.Get(); }
		ID3D12CommandQueue* GetTransferQueue() const { return m_TransferQueue.Get(); }

		ID3D12CommandAllocator* GetGraphicsCommandAllocator() const { return m_GraphicsCommandAllocator.Get(); }
		ID3D12CommandAllocator* GetComputeCommandAllocator() const { return m_ComputeCommandAllocator.Get(); }
		ID3D12CommandAllocator* GetTransferCommandAllocator() const { return m_TransferCommandAllocator.Get(); }

	private:
		ComPtr<IDXGIFactory6> m_DxgiFactory;
		ComPtr<ID3D12Device> m_Device;
		ComPtr<IDXGIAdapter1> m_Adapter;

		ComPtr<ID3D12CommandQueue> m_GraphicsQueue;
		ComPtr<ID3D12CommandQueue> m_ComputeQueue;
		ComPtr<ID3D12CommandQueue> m_TransferQueue;

		ComPtr<ID3D12CommandAllocator> m_GraphicsCommandAllocator;
		ComPtr<ID3D12CommandAllocator> m_ComputeCommandAllocator;
		ComPtr<ID3D12CommandAllocator> m_TransferCommandAllocator;
	};
}