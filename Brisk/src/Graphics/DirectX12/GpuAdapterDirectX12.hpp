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

		virtual void WaitIdle() override { }
		virtual void Release() override { assert(false); }
		virtual void ReleasePools() override { assert(false); }
		void LogDirectXDebugs();

		ComPtr<IDXGIFactory6> GetDXGIFactory() const { return m_DxgiFactory; }
		ComPtr<ID3D12Device> GetDevice() const { return m_Device; }
		ID3D12CommandQueue* GetGraphicsQueue() const { return m_GraphicsQueue.Get(); }
		ID3D12CommandQueue* GetComputeQueue() const { return m_ComputeQueue.Get(); }
		ID3D12CommandQueue* GetTransferQueue() const { return m_TransferQueue.Get(); }

		ComPtr<ID3D12DescriptorHeap> GetCbvSrvUavHeap() const { return m_CbvSrvUavHeap; }
		ComPtr<ID3D12DescriptorHeap> GetSamplerHeap() const { return m_SamplerHeap; }
		ComPtr<ID3D12DescriptorHeap> GetRtvHeap() const { return m_RtvHeap; }
		ComPtr<ID3D12DescriptorHeap> GetDsvHeap() const { return m_DsvHeap; }

		uint32_t GetAndIncrementCbvSrvUavHeapIndex()
		{ 
			uint32_t tempIndex = m_CbvSrvUavHeapCurrentIndex;
			m_CbvSrvUavHeapCurrentIndex++;
			return tempIndex;
		}
		uint32_t GetAndIncrementSamplerHeapIndex()
		{ 
			uint32_t tempIndex = m_CbvSrvUavHeapCurrentIndex;
			m_SamplerHeapCurrentIndex++;
			return tempIndex;
		}
		uint32_t GetAndIncrementRtvHeapIndex()
		{ 
			uint32_t tempIndex = m_CbvSrvUavHeapCurrentIndex;
			m_RtvHeapCurrentIndex++;
			return tempIndex;
		}
		uint32_t GetAndIncrementDsvHeapIndex()
		{ 
			uint32_t tempIndex = m_DsvHeapCurrentIndex;
			m_DsvHeapCurrentIndex++;
			return tempIndex;
		}

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

		ComPtr<ID3D12DescriptorHeap> m_CbvSrvUavHeap;
		ComPtr<ID3D12DescriptorHeap> m_SamplerHeap;
		ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
		ComPtr<ID3D12DescriptorHeap> m_DsvHeap;

		uint32_t m_CbvSrvUavHeapSize;
		uint32_t m_SamplerHeapSize;
		uint32_t m_RtvHeapSize;
		uint32_t m_DsvHeapSize;

		uint32_t m_CbvSrvUavHeapCurrentIndex;
		uint32_t m_SamplerHeapCurrentIndex;
		uint32_t m_RtvHeapCurrentIndex;
		uint32_t m_DsvHeapCurrentIndex;
	};
}