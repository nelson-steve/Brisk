#pragma once

#include "Engine/Renderer/CommandBuffer.hpp"
//------------------------------------------
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
//--------------------
using Microsoft::WRL::ComPtr;

namespace Brisk
{
	class CommandBufferDirectX12 : public CommandBuffer {
	public:
		virtual void Bind() override;
		virtual void UnBind() override;

		void Allocate(ComPtr<ID3D12CommandAllocator> allocator);
	private:
		ComPtr<ID3D12CommandAllocator> m_ParentAllocator;
		ComPtr<ID3D12GraphicsCommandList> m_CommandList;
	};
}