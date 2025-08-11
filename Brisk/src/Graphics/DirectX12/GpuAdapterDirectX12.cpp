#include "GpuAdapterDirectX12.hpp"

#include "Core/Log.hpp"

#include <comdef.h>
#include <cerrno>
#include <iostream>

namespace Brisk
{
//#define DX_CHECK(x)                                                   \
//    {                                                                 \
//        HRESULT hr__ = (x);                                           \
//        if (FAILED(hr__)) {                                           \
//            LogDXError(hr__, __FILE__, __LINE__, #x);                 \
//            __debugbreak(); /* or throw/return */                     \
//        }                                                             \
//    }

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

	void GpuAdapterDirectX12::LogDirectXDebugs() {
		ComPtr<ID3D12InfoQueue> infoQueue;
		if (SUCCEEDED(m_Device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
			UINT64 numMessages = infoQueue->GetNumStoredMessages();

			for (UINT64 i = 0; i < numMessages; ++i) {
				SIZE_T messageLength = 0;
				infoQueue->GetMessage(i, nullptr, &messageLength); // Get the length

				D3D12_MESSAGE* message = (D3D12_MESSAGE*)malloc(messageLength);
				if (infoQueue->GetMessage(i, message, &messageLength) == S_OK) {
					// Print to command prompt
					printf("D3D12: %s\n", message->pDescription);

					// You could also use OutputDebugStringA(message->pDescription);
				}

				free(message);
			}

			// Clear the queue after processing
			infoQueue->ClearStoredMessages();
		}
	}

	void GpuAdapterDirectX12::Init() {
		// Creating DXGIFactory
		// Used to enumerate graphics adapters
		// Can also be used to enumerate the connected displays to each graphics adapter
		// Creates swapchain as well
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

		ComPtr<ID3D12DeviceRemovedExtendedDataSettings> dredSettings;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&dredSettings)))) {
			dredSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
			dredSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
		}
#endif

		for (UINT i = 0; m_DxgiFactory->EnumAdapters1(i, &m_Adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
			DXGI_ADAPTER_DESC1 desc;
			m_Adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;

			// Found a valid GPU
			break;
		}

		// Currenly do no support all feature levels
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
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);

			// Suppress info messages
			D3D12_MESSAGE_SEVERITY denySeverities[] = { D3D12_MESSAGE_SEVERITY_INFO };
			// Allow corruption, error and warning messages
			D3D12_MESSAGE_SEVERITY allowSeverities[] = { D3D12_MESSAGE_SEVERITY_CORRUPTION, D3D12_MESSAGE_SEVERITY_ERROR, D3D12_MESSAGE_SEVERITY_WARNING };
			D3D12_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumSeverities = _countof(denySeverities);
			filter.DenyList.pSeverityList = denySeverities;
			filter.AllowList.pSeverityList = allowSeverities;

			infoQueue->PushStorageFilter(&filter);
		}

		// Create graphics queue
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		hr = m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_GraphicsQueue));
		if (FAILED(hr)) {
			BRISK_CORE_ERROR("Failed to create Direct command queue");
		}

		// Create compute queue
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		hr = m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_ComputeQueue));
		if (FAILED(hr)) {
			BRISK_CORE_ERROR("Failed to create Compute command queue");
		}

		// Create transfer queue
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		hr = m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_TransferQueue));
		if (FAILED(hr)) {
			BRISK_CORE_ERROR("Failed to create Copy command queue");
		}

		// Create heap for Constant Buffer View, Shader Resource View, Unordered Access View
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = 10000;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		hr = m_Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_CbvSrvUavHeap));
		if (FAILED(hr)) {
			BRISK_CORE_ERROR("Failed to create DirectX12 Descriptor Heap");
			return;
		}

		// Create heap for Samplers
		heapDesc.NumDescriptors = 128;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		hr = m_Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_SamplerHeap));
		if (FAILED(hr)) {
			BRISK_CORE_ERROR("Failed to create DirectX12 Descriptor Heap");
			return;
		}

		// Create heap for Render Target View
		heapDesc.NumDescriptors = 128;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		hr = m_Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_RtvHeap));
		if (FAILED(hr)) {
			BRISK_CORE_ERROR("Failed to create DirectX12 Descriptor Heap");
			return;
		}

		// Create heap for Depth Stencil View
		heapDesc.NumDescriptors = 64;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		hr = m_Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_DsvHeap));
		if (FAILED(hr)) {
			BRISK_CORE_ERROR("Failed to create DirectX12 Descriptor Heap");
			return;
		}

		m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence));
		m_FenceValue = 0;

		m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_FenceEvent == nullptr)
			throw std::runtime_error("Failed to create fence event.");

	}

	void GpuAdapterDirectX12::WaitIdle() {
		const UINT64 fenceToWaitFor = ++m_FenceValue;
		m_GraphicsQueue->Signal(m_Fence.Get(), fenceToWaitFor);

		if (m_Fence->GetCompletedValue() < fenceToWaitFor)
		{
			m_Fence->SetEventOnCompletion(fenceToWaitFor, m_FenceEvent);
			WaitForSingleObject(m_FenceEvent, INFINITE);
		}
	}
}