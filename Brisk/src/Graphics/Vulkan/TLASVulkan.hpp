#pragma once
// INCLUDES
#include "Engine/Renderer/TLAS.hpp"
//----------------------------------
#include <vk_mem_alloc.h>
#include <volk.h>
//--------------------

namespace Brisk
{
	class TLASVulkan : public TLAS {
	public:
		virtual void Build(std::shared_ptr<BLAS> blas) override;

		const VkAccelerationStructureKHR* GetHandle() { return &handle; }
	private:
		VmaAllocation m_Allocation;
		VkAccelerationStructureKHR handle{ VK_NULL_HANDLE };
		VkDeviceAddress deviceAddress{ 0 };
		VkBuffer buffer{ VK_NULL_HANDLE };
	};
}