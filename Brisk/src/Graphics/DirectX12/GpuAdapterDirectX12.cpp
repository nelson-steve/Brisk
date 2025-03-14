#include "GpuAdapterDirectX12.hpp"

namespace Brisk
{
	void GpuAdapterDirectX12::Init() {
		HRESULT hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&m_DxgiFactory));
		if (FAILED(hr)) {
			// Handle error
		}

		for (UINT i = 0; m_DxgiFactory->EnumAdapters1(i, &m_Adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
			DXGI_ADAPTER_DESC1 desc;
			m_Adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;

			// Found a valid GPU
			break;
		}

		hr = D3D12CreateDevice(m_Adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_Device));
		if (FAILED(hr)) {
			//hr = D3D12CreateDevice(m_Adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_Device));
		}

	}
}