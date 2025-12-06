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
		virtual void Build(std::shared_ptr<Buffer> vb, std::shared_ptr<Buffer> ib) override;
		VkDeviceAddress GetDeviceAddress() {
			VkBufferDeviceAddressInfoKHR bufferDeviceAddresInfo{
				.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
				.buffer = buffer
			};
			return vkGetBufferDeviceAddressKHR(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), &bufferDeviceAddresInfo);
		}

		VmaAllocation m_Allocation;
		std::vector<VkAccelerationStructureKHR> blases;
		std::vector<VkDeviceAddress> blasAddresses;
		VkBuffer buffer{ VK_NULL_HANDLE };
	};
}