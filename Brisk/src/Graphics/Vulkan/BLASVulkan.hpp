#pragma once
// INCLUDES
#include "Engine/Renderer/BLAS.hpp"
#include "GpuAdapterVulkan.hpp"
//----------------------------------
#include <vk_mem_alloc.h>
#include <volk.h>
//--------------------

namespace Brisk
{
	class BLASVulkan : public BLAS {
	public:
		virtual void Init(const BLASSpecs& specs, std::shared_ptr<CommandBuffer> cmd) override;
		VkDeviceAddress GetDeviceAddress() {
			VkBufferDeviceAddressInfoKHR bufferDeviceAddresInfo{
				.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
				.buffer = buffer
			};
			return vkGetBufferDeviceAddressKHR(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), &bufferDeviceAddresInfo);
		}
	private:
		VmaAllocation m_Allocation;
		VkAccelerationStructureKHR handle{ VK_NULL_HANDLE };
		VkDeviceAddress deviceAddress{ 0 };
		VkDeviceMemory memory{ VK_NULL_HANDLE };
		VkBuffer buffer{ VK_NULL_HANDLE };
	};
}