#pragma once

// INCLUDES
#include "Engine/Renderer/Buffer.hpp"
//-----------------------------------
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
//---------------------
using Microsoft::WRL::ComPtr;

namespace Brisk 
{
	class BufferDirectX12 : public Buffer {
	public:
		virtual void Init(uint64_t size,
			void* data,
			std::vector<Core::BufferUsage> usageFlags,
			std::vector<Core::MemoryProperty> memoryProperty,
			bool mapPersistant) override;
		virtual void Release() override;
		virtual void UpdatePersistantData(uint32_t size, void* data) override;
		inline ID3D12Resource* Get() const {
			return m_Buffer.Get();
		}

		D3D12_CONSTANT_BUFFER_VIEW_DESC GetDescriptor() const {
			D3D12_CONSTANT_BUFFER_VIEW_DESC desc{};
			desc.BufferLocation = m_Buffer->GetGPUVirtualAddress();
			desc.SizeInBytes = AlignSize(m_Size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
			return desc;
		}

		void UploadData(void* data, uint64_t size) {
			void* mappedMemory = nullptr;
			m_Buffer->Map(0, nullptr, &mappedMemory);
			memcpy(mappedMemory, data, size);
			m_Buffer->Unmap(0, nullptr);
		}

		uint64_t AlignSize(uint64_t size, uint64_t alignment) const {
			return (size + (alignment - 1)) & ~(alignment - 1);
		}
	private:
		void Create(uint64_t size);
	private:
		ComPtr<ID3D12Resource> m_Buffer;
		uint64_t m_Size;
		bool m_MapPersistent = false;
		void* m_MappedPointer = nullptr;
	};
}