#pragma once

#include "Engine/Renderer/GpuAdapter.hpp"

#include <cassert>

#include <d3d12.h>
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

		ComPtr<IDXGIFactory6> GetDXGIFactory() const { return m_DxgiFactory; }
		ComPtr<ID3D12Device> GetDevice() const { return m_Device; }
		ID3D12CommandQueue* GetCommandQueue() const { return m_CommandQueue.Get(); }

	private:
		ComPtr<IDXGIFactory6> m_DxgiFactory;
		ComPtr<ID3D12Device> m_Device;
		ComPtr<IDXGIAdapter1> m_Adapter;

		ComPtr<ID3D12CommandQueue> m_CommandQueue;
	};
}