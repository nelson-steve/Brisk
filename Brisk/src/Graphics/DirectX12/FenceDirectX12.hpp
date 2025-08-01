#pragma once

//INCLUDES
#include "Engine/Renderer/Fence.hpp"
//-----------------
#include <directx/d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
//--------------------
using Microsoft::WRL::ComPtr;

namespace Brisk
{
	class FenceDirectX12 : public Fence {
	public:
		virtual void Init() override;

		virtual void Wait()  override;
		virtual void Reset()  override;
	private:
		ComPtr<ID3D12Fence> m_Fence;
		UINT64 m_FenceValue = 0;
		HANDLE m_FenceEvent = nullptr;
	};
}