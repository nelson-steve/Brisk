#include "GpuAdapterDirectX12.hpp"

#include "Core/Log.hpp"

#include <comdef.h>
#include <cerrno>
#include <iostream>

namespace Brisk
{
#define DX_CHECK(x)                                                   \
    {                                                                 \
        HRESULT hr__ = (x);                                           \
        if (FAILED(hr__)) {                                           \
            LogDXError(hr__, __FILE__, __LINE__, #x);                 \
            __debugbreak(); /* or throw/return */                     \
        }                                                             \
    }

	const char* FeatureLevelToString(D3D_FEATURE_LEVEL level) {
		switch (level) {
		case D3D_FEATURE_LEVEL_12_2: return "12.2";
		case D3D_FEATURE_LEVEL_12_1: return "12.1";
		case D3D_FEATURE_LEVEL_12_0: return "12.0";
		case D3D_FEATURE_LEVEL_11_1: return "11.1";
		case D3D_FEATURE_LEVEL_11_0: return "11.0";
		default: return "Unknown";
		}
	}

	void LogDXError(HRESULT hr, const char* file, int line, const char* expr) {
		_com_error err(hr);
		std::wstring msg = err.ErrorMessage();
		std::wcerr << L"[DX12 ERROR] " << msg << L"\nIn: " << expr << L"\nAt: " << file << L":" << line << std::endl;
	}


	void GpuAdapterDirectX12::Init() {
		HRESULT hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&m_DxgiFactory));
		if (FAILED(hr)) {
			BRISK_CORE_ERROR("Failed to create Dxgi factory");
			return;
		}

#if defined(_DEBUG)
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
			debugController->EnableDebugLayer();
		}
#endif

		for (UINT i = 0; m_DxgiFactory->EnumAdapters1(i, &m_Adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
			DXGI_ADAPTER_DESC1 desc;
			m_Adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;

			// Found a valid GPU
			break;
		}

		D3D_FEATURE_LEVEL featureLevels[] = {
			D3D_FEATURE_LEVEL_12_2,
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
		};

		D3D_FEATURE_LEVEL m_FeatureLevel;
		hr = E_FAIL;
		for (auto level : featureLevels) {
			hr = D3D12CreateDevice(m_Adapter.Get(), level, IID_PPV_ARGS(&m_Device));
			if (SUCCEEDED(hr)) {
				m_FeatureLevel = level;
				break;
			}
		}

		if (SUCCEEDED(hr)) {
			BRISK_CORE_INFO("Using D3D Feature Level: {}", FeatureLevelToString(m_FeatureLevel));
		}
		else {
			BRISK_CORE_ERROR("Failed to create D3D12 device with supported feature levels.");
		}

		ComPtr<ID3D12InfoQueue> infoQueue;
		if (SUCCEEDED(m_Device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
			// Suppress certain messages
			D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
			D3D12_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumSeverities = _countof(severities);
			filter.DenyList.pSeverityList = severities;

			infoQueue->PushStorageFilter(&filter);
		}

		// Create command queue
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

		hr = m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueue));
		if (FAILED(hr)) {
			BRISK_CORE_ERROR("Failed to create DirectX command queue");
		}

	}
}