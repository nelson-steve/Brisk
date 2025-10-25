#pragma once

// INCLUDES
#include "Engine/Renderer/Buffer.hpp"
//-----------------------------------
#include <directx/d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <Core/Log.hpp>
//---------------------
using Microsoft::WRL::ComPtr;

namespace Brisk 
{
	class BufferDirectX12 : public Buffer {
	public:
		virtual void Init(const BufferDesc& desc) override;
		virtual void Release() override;
		virtual void UpdatePersistantData(uint32_t size, void* data, size_t ptrOffset) override;
		virtual void MemoryPipelineBarrier(std::shared_ptr<CommandBuffer> cmd, MemoryBarrierParams barrier) override { BRISK_CORE_ASSERT(false); }

		D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle() const { return m_CpuHandle; }
		D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() const { return m_GpuHandle; }

		inline ID3D12Resource* Get() const {
			return m_Buffer.Get();
		}

		inline uint32_t SizeInBytes() const { return m_Size; }

		uint64_t AlignSize(uint64_t size, uint64_t alignment) const {
			return (size + (alignment - 1)) & ~(alignment - 1);
		}
	private:
		ComPtr<ID3D12Resource> m_Buffer;
		D3D12_CPU_DESCRIPTOR_HANDLE m_CpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_GpuHandle;
		uint32_t m_Size;
		bool m_MapPersistent = false;
		void* m_MappedPointer = nullptr;
	};
}