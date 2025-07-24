// INCLUDES
#include "BufferVulkan.hpp"
#include "Engine/Engine.hpp"
#include "UtilitiesVulkan.hpp"
#include "GpuAdapterVulkan.hpp"
#include "Engine/Application.hpp"
//-------------------------------
#include <memory>
#include "CommandBufferVulkan.hpp"
//---------------
namespace Brisk 
{
	void BufferVulkan::Init(uint32_t size, void* data, Core::BufferUsage usageFlags, Core::MemoryProperty memoryProperty, bool mapPersistant) {
		VkBufferUsageFlags usage{};
        VmaMemoryUsage vmaUsage = VMA_MEMORY_USAGE_UNKNOWN;
		usage = UtilitiesVulkan::BufferUsageToVkFormat(usageFlags);
		m_Size = size;

        bool hostVisible = false;
        bool hostCoherent = false;
        bool hostCached = false;
        bool deviceLocal = false;

        VmaAllocator cachedAllocator = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetVmaAllocator();

        if (Core::HasFlag(memoryProperty, Core::MemoryProperty::HostVisible)) {
            hostVisible = true;
        }
        if (Core::HasFlag(memoryProperty, Core::MemoryProperty::HostCoherent)) {
            hostCoherent = true;
        }
        if (Core::HasFlag(memoryProperty, Core::MemoryProperty::HostCached)) {
            hostCached = true;
        }
        if (Core::HasFlag(memoryProperty, Core::MemoryProperty::DeviceLocal)) {
            deviceLocal = true;
        }

        if (deviceLocal && !hostVisible) {
            vmaUsage = VMA_MEMORY_USAGE_GPU_ONLY;
        }
        else if (hostVisible && hostCoherent) {
            vmaUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        }
        else if (hostVisible && !hostCoherent) {
            vmaUsage = VMA_MEMORY_USAGE_CPU_ONLY;
        }
        else {
            vmaUsage = VMA_MEMORY_USAGE_GPU_ONLY;
        }

        bool needsStaging = (deviceLocal && data != nullptr);

        if (needsStaging) {
            // Create staging buffer
            VkBufferCreateInfo stagingBufferInfo{};
            stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            stagingBufferInfo.size = size;
            stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VmaAllocationCreateInfo stagingAllocInfo{};
            stagingAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

            VkBuffer stagingBuffer;
            VmaAllocation stagingAllocation;
            if (vmaCreateBuffer(cachedAllocator, &stagingBufferInfo, &stagingAllocInfo, &stagingBuffer, &stagingAllocation, nullptr) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create staging buffer");
            }

            // Map staging buffer and copy data
            void* mappedStaging = nullptr;
            vmaMapMemory(cachedAllocator, stagingAllocation, &mappedStaging);
            std::memcpy(mappedStaging, data, (size_t)size);
            vmaUnmapMemory(cachedAllocator, stagingAllocation);

            // Create device local buffer
            VkBufferCreateInfo deviceBufferInfo{};
            deviceBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            deviceBufferInfo.size = size;
            deviceBufferInfo.usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            deviceBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VmaAllocationCreateInfo deviceAllocInfo{};
            deviceAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

            if (vmaCreateBuffer(cachedAllocator, &deviceBufferInfo, &deviceAllocInfo, &m_Handle, &m_Allocation, nullptr) != VK_SUCCESS) {
                vmaDestroyBuffer(cachedAllocator, stagingBuffer, stagingAllocation);
                throw std::runtime_error("Failed to create device local buffer");
            }

            VkDebugUtilsObjectNameInfoEXT nameInfo = {};
            nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            nameInfo.objectType = VK_OBJECT_TYPE_BUFFER;
            nameInfo.objectHandle = (uint64_t)m_Handle;
            nameInfo.pObjectName = "storage buffer";

#if _DEBUG
            vkSetDebugUtilsObjectNameEXT(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &nameInfo);
#endif

            {
                VkCommandBufferAllocateInfo allocInfoCmd{};
                allocInfoCmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                allocInfoCmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                allocInfoCmd.commandPool = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetGraphicsCommandPool();
                allocInfoCmd.commandBufferCount = 1;

                VkCommandBuffer commandBuffer;
                vkAllocateCommandBuffers(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), &allocInfoCmd, &commandBuffer);

                VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

                vkBeginCommandBuffer(commandBuffer, &beginInfo);

                VkBufferCopy copyRegion{};
                copyRegion.size = size;
                vkCmdCopyBuffer(commandBuffer, stagingBuffer,  m_Handle, 1, &copyRegion);

                vkEndCommandBuffer(commandBuffer);

                VkSubmitInfo submitInfo{};
                submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &commandBuffer;

                vkQueueSubmit(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
                vkQueueWaitIdle(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetGraphicsQueue());

                // --- 7. Cleanup command buffer ---
                vkFreeCommandBuffers(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), 
                    std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetGraphicsCommandPool(), 1, &commandBuffer);
            }

            vmaDestroyBuffer(cachedAllocator, stagingBuffer, stagingAllocation);

            isMapped = false;
            mappedPtr = nullptr;
            return;
        }

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = vmaUsage;
        if (hostCoherent) allocInfo.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;

        if (vmaCreateBuffer(cachedAllocator, &bufferInfo, &allocInfo, &m_Handle, &m_Allocation, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create buffer");
        }

        if (allocInfo.flags & VMA_ALLOCATION_CREATE_MAPPED_BIT) {
            VmaAllocationInfo allocInfo;
            vmaGetAllocationInfo(cachedAllocator, m_Allocation, &allocInfo);
            mappedPtr = allocInfo.pMappedData;
            isMapped = true;
        }
        else {
            mappedPtr = nullptr;
            isMapped = false;
        }

        if (data != nullptr) {
            if (isMapped) {
                std::memcpy(mappedPtr, data, (size_t)size);
                if (!hostCoherent) {
                    // Flush if not coherent
                    VmaAllocationInfo allocInfo;
                    vmaGetAllocationInfo(cachedAllocator, m_Allocation, &allocInfo);
                    //mappedPtr = allocInfo.pMappedData;
                    VkDeviceMemory mem = allocInfo.deviceMemory;;
                    VkMappedMemoryRange range{};
                    range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
                    range.memory = mem;
                    range.offset = 0;
                    range.size = size;
                    vkFlushMappedMemoryRanges(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), 1, &range);
                }
            }
            else {
                // Map/unmap for one-time upload
                void* mapped = nullptr;
                vmaMapMemory(cachedAllocator, m_Allocation, &mapped);
                std::memcpy(mapped, data, (size_t)size);
                vmaUnmapMemory(cachedAllocator, m_Allocation);
            }
        }

        if (!mapPersistant && isMapped && !hostCoherent) {
            vmaUnmapMemory(cachedAllocator, m_Allocation);
            mappedPtr = nullptr;
            isMapped = false;
        }
	}

	void BufferVulkan::UpdatePersistantData(uint32_t size, void* data) {
		memcpy(mappedPtr, data, m_Size);
	}

    void BufferVulkan::MemoryPipelineBarrier(std::shared_ptr<CommandBuffer> cmd) {
        VkMemoryBarrier memoryBarrier{ VK_STRUCTURE_TYPE_MEMORY_BARRIER };
        memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        vkCmdPipelineBarrier(
            std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(),
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, // srcStage
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, // dstStage
            0,
            1, &memoryBarrier,
            0, nullptr,
            0, nullptr
        );
    }

	void BufferVulkan::Release() {
        BRISK_CORE_INFO("Destroying vma buffer");
		vmaDestroyBuffer(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetVmaAllocator(), m_Handle, m_Allocation);
	}
}