#pragma once

#include "Engine/Renderer/GpuAdapter.hpp"

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

		ComPtr<ID3D12Device> GetDevice() const { return m_Device; }
		ComPtr<IDXGIFactory6> GetDXGIFactory() const { return m_DxgiFactory; }

		virtual void AddResource(GpuResourceType type, std::shared_ptr<Texture> texture) { assert(false); }
		virtual void AddResource(GpuResourceType type, std::shared_ptr<Buffer> buffer) { assert(false); }
	private:
		ComPtr<IDXGIFactory6> m_DxgiFactory;
		ComPtr<ID3D12Device> m_Device;
		ComPtr<IDXGIAdapter1> m_Adapter;
	};
}