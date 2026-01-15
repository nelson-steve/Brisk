#pragma once

// INCLUDES
#include "Engine/Renderer/CommandBuffer.hpp"
#include "Engine/Renderer/CommandBufferAllocator.hpp"
//------------------------------------------
#include <directx/d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
//--------------------
using Microsoft::WRL::ComPtr;

namespace Brisk
{
	class CommandBufferDirectX12 : public CommandBuffer {
	public:
		virtual void Bind(bool singleUse) override;
		virtual void UnBind() override;
		virtual void Reset() override;
		virtual void Allocate(PoolType type) override;

		ComPtr<ID3D12GraphicsCommandList6> Get() { return m_CommandList; }

		void SetParentAlloator(std::shared_ptr<CommandBufferAllocator> allocator);
	private:
		ComPtr<ID3D12CommandAllocator> m_ParentAllocator;
		ComPtr<ID3D12GraphicsCommandList6> m_CommandList;
	};
}