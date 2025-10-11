#pragma once

// INCLUDES
#include "RHI.hpp"
#include "CommandBuffer.hpp"
#include "Engine/Renderer/Texture.hpp"
//----------------
#include <cstdint>
#include <vector>
#include <memory>
//---------------

namespace Brisk
{
    struct MemoryBarrierParams {
        Core::AccessType srcAccess;
        Core::AccessType dstAccess;
        Core::PipelineStage srcStage;
        Core::PipelineStage dstStage;
    };

    struct BufferDesc
    {
        std::string p_Name;                // Debug name

        uint64_t p_Size = 0;               // Size in bytes
        uint64_t p_ElementSize = 0;        // Size of one element (for structured buffers)

        void* p_Data;

        Core::BufferUsage p_Usage;

        enum class MemoryUsage : uint32_t
        {
            CPU_Only,                    //
            GPU_Only,                    // VRAM only
            CPU_To_GPU,                  // Upload heap in DirectX12, Host-visible in Vulkan
            GPU_To_CPU                   // Readback heap in DirectX12, Host-visible in Vulkan
        } p_Memory = MemoryUsage::GPU_Only;

        bool p_AllowUAV = false;           // Allow unordered access (RW in shaders)
        bool p_AllowSRV = false;           // Allow shader resource view
        bool p_AllowCopySrc = false;       // Can be copy source
        bool p_AllowCopyDst = false;       // Can be copy destination
        bool p_Persistant = false;         // Should stay mapped
    };

	class Buffer {
	public:
		virtual void Init(const BufferDesc& desc) = 0;
		virtual void Release() = 0;
		virtual void UpdatePersistantData(uint32_t size, void* data) = 0;
		
		virtual void MemoryPipelineBarrier(std::shared_ptr<CommandBuffer> cmd, MemoryBarrierParams barrier) = 0;

		static std::shared_ptr<Buffer> Create();
	protected:
		inline uint64_t GetSize() const { 
			return m_Desc.p_Size; 
		}
	protected:
        BufferDesc m_Desc;
	};
}