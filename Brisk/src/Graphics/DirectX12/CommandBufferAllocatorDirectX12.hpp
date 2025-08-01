#pragma once

#include "Engine/Renderer/CommandBufferAllocator.hpp"

#include <directx/d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

namespace Brisk
{
	class CommandBufferAllocatorDirectX12 : public CommandBufferAllocator {
	public:
		virtual void Init() override;
		virtual void Allocate(std::shared_ptr<CommandBuffer> cmd) override;
	private:
		ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	};
}