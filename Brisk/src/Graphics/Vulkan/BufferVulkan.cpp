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
	void BufferVulkan::Init(const BufferDesc& desc) {
        m_Desc = desc;
        VmaMemoryUsage vmaUsage = VMA_MEMORY_USAGE_UNKNOWN;
        VkBufferUsageFlags usageFlags = 0;
        usageFlags = UtilitiesVulkan::BufferUsageToVkFormat(desc.p_Usage);

        if (desc.p_AllowUAV)
            usageFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        //if (desc.p_AllowSRV)
        //    usageFlags |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT |
        //    VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
        if (desc.p_AllowCopySrc)
            usageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        if (desc.p_AllowCopyDst)
            usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        bool hostVisible = false;
        bool hostCoherent = false;
        bool hostCached = false;
        bool deviceLocal = false;

        m_CachedAllocator = std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetVmaAllocator();

        if (desc.p_Memory == BufferDesc::MemoryUsage::CPU_To_GPU) {
            vmaUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
            hostVisible = true;
            hostCoherent = true;
        }
        else if (desc.p_Memory == BufferDesc::MemoryUsage::GPU_Only) {
            vmaUsage = VMA_MEMORY_USAGE_GPU_ONLY;
            deviceLocal = true;
        }
        else if (desc.p_Memory == BufferDesc::MemoryUsage::GPU_To_CPU) {
            vmaUsage = VMA_MEMORY_USAGE_GPU_TO_CPU;
        }
        else if (desc.p_Memory == BufferDesc::MemoryUsage::CPU_Only) {
            vmaUsage = VMA_MEMORY_USAGE_CPU_ONLY;
        }

        // Check if the buffer stores data in gpu only, and if data is providied initialy means it needs to be copied right away
        // needing a staging buffer
        bool needsStaging = (deviceLocal && desc.p_Data != nullptr);

        if (needsStaging) {
            // Create staging buffer
            //VkBufferCreateInfo stagingBufferInfo{};
            //stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            //stagingBufferInfo.size = desc.p_Size;
            //stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            //stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            //VmaAllocationCreateInfo stagingAllocInfo{};
            //stagingAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

            //VkBuffer stagingBuffer;
            //VmaAllocation stagingAllocation;
            //if (vmaCreateBuffer(cachedAllocator, &stagingBufferInfo, &stagingAllocInfo, &stagingBuffer, &stagingAllocation, nullptr) != VK_SUCCESS) {
            //    throw std::runtime_error("Failed to create staging buffer");
            //}

            // Map staging buffer and copy data
            //void* mappedStaging = nullptr;
            //vmaMapMemory(cachedAllocator, stagingAllocation, &mappedStaging);
            //std::memcpy(mappedStaging, desc.p_Data, (size_t)desc.p_Size);
            //vmaUnmapMemory(cachedAllocator, stagingAllocation);

            Application::GetRenderer()->m_ScratchAllocator.m_ScratchBuffer->UpdatePersistantData(desc.p_Size, desc.p_Data, 0);

            // Create device local buffer
            VkBufferCreateInfo deviceBufferInfo{};
            deviceBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            deviceBufferInfo.size = desc.p_Size;
            deviceBufferInfo.usage = usageFlags | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            deviceBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VmaAllocationCreateInfo deviceAllocInfo{};
            deviceAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

            if (vmaCreateBuffer(m_CachedAllocator, &deviceBufferInfo, &deviceAllocInfo, &m_Handle, &m_Allocation, nullptr) != VK_SUCCESS) {
                //vmaDestroyBuffer(cachedAllocator, stagingBuffer, stagingAllocation);
                throw std::runtime_error("Failed to create device local buffer");
            }

            VkDebugUtilsObjectNameInfoEXT nameInfo = {};
            nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            nameInfo.objectType = VK_OBJECT_TYPE_BUFFER;
            nameInfo.objectHandle = (uint64_t)m_Handle;
            nameInfo.pObjectName = desc.p_Name.c_str();

#if _DEBUG
            vkSetDebugUtilsObjectNameEXT(Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &nameInfo);
#endif

            {
                VkCommandBufferAllocateInfo allocInfoCmd{};
                allocInfoCmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                allocInfoCmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                allocInfoCmd.commandPool = std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetGraphicsCommandPool();
                allocInfoCmd.commandBufferCount = 1;

                VkCommandBuffer commandBuffer;
                vkAllocateCommandBuffers(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), &allocInfoCmd, &commandBuffer);

                VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

                vkBeginCommandBuffer(commandBuffer, &beginInfo);

                VkBufferCopy copyRegion{};
                copyRegion.size = desc.p_Size;
                vkCmdCopyBuffer(commandBuffer, std::static_pointer_cast<BufferVulkan>(Application::GetRenderer()->m_ScratchAllocator.m_ScratchBuffer)->Get(), m_Handle, 1, &copyRegion);

                vkEndCommandBuffer(commandBuffer);

                VkSubmitInfo submitInfo{};
                submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &commandBuffer;

                vkQueueSubmit(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
                vkQueueWaitIdle(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetGraphicsQueue());

                // --- 7. Cleanup command buffer ---
                vkFreeCommandBuffers(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), 
                    std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetGraphicsCommandPool(), 1, &commandBuffer);
            }

            //vmaDestroyBuffer(cachedAllocator, stagingBuffer, stagingAllocation);

            isMapped = false;
            mappedPtr = nullptr;
            return;
        }

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = desc.p_Size;
        bufferInfo.usage = usageFlags;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = vmaUsage;
        if (hostCoherent) allocInfo.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;

        if (vmaCreateBuffer(m_CachedAllocator, &bufferInfo, &allocInfo, &m_Handle, &m_Allocation, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create buffer");
        }

        VkDebugUtilsObjectNameInfoEXT nameInfo = {};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = VK_OBJECT_TYPE_BUFFER;
        nameInfo.objectHandle = (uint64_t)m_Handle;
        nameInfo.pObjectName = desc.p_Name.c_str();

#if _DEBUG
        vkSetDebugUtilsObjectNameEXT(Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &nameInfo);
#endif

        if (allocInfo.flags & VMA_ALLOCATION_CREATE_MAPPED_BIT) {
            VmaAllocationInfo allocInfo;
            vmaGetAllocationInfo(m_CachedAllocator, m_Allocation, &allocInfo);
            mappedPtr = allocInfo.pMappedData;
            isMapped = true;
        }
        else {
            mappedPtr = nullptr;
            isMapped = false;
        }

        if (desc.p_Data != nullptr) {
            if (isMapped) {
                std::memcpy(mappedPtr, desc.p_Data, (size_t)desc.p_Size);
                if (!hostCoherent) {
                    // Flush if not coherent
                    VmaAllocationInfo allocInfo;
                    vmaGetAllocationInfo(m_CachedAllocator, m_Allocation, &allocInfo);
                    //mappedPtr = allocInfo.pMappedData;
                    VkDeviceMemory mem = allocInfo.deviceMemory;;
                    VkMappedMemoryRange range{};
                    range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
                    range.memory = mem;
                    range.offset = 0;
                    range.size = desc.p_Size;
                    vkFlushMappedMemoryRanges(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), 1, &range);
                }
            }
            else {
                // Map/unmap for one-time upload
                void* mapped = nullptr;
                vmaMapMemory(m_CachedAllocator, m_Allocation, &mapped);
                std::memcpy(mapped, desc.p_Data, (size_t)desc.p_Size);
                vmaUnmapMemory(m_CachedAllocator, m_Allocation);
            }
        }

        if (!desc.p_Persistant && isMapped && !hostCoherent) {
            vmaUnmapMemory(m_CachedAllocator, m_Allocation);
            mappedPtr = nullptr;
            isMapped = false;
        }
	}

	void BufferVulkan::UpdatePersistantData(uint32_t size, void* data, uint64_t offset) {
		memcpy(static_cast<uint8_t*>(mappedPtr) + offset, data, size);
	}

    void BufferVulkan::RecordUpload(std::shared_ptr<CommandBuffer> cmd, uint32_t size, void* data) {
        Application::GetRenderer()->m_PendingBufferUpload = true;
        size_t allocationOffset = Application::GetRenderer()->m_ScratchAllocator.Allocate(size);
        Application::GetRenderer()->m_ScratchAllocator.m_ScratchBuffer->UpdatePersistantData(size, data, allocationOffset);

        //cmd->Bind();

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = allocationOffset;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;
        vkCmdCopyBuffer(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), std::static_pointer_cast<BufferVulkan>(Application::GetRenderer()->m_ScratchAllocator.m_ScratchBuffer)->Get(), m_Handle, 1, &copyRegion);

        //cmd->UnBind();
    }

    void BufferVulkan::MemoryPipelineBarrier(std::shared_ptr<CommandBuffer> cmd, MemoryBarrierParams barrier) {
        VkBufferMemoryBarrier bufferBarrier{ VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER };
        bufferBarrier.srcAccessMask = UtilitiesVulkan::AccessTypeToVkAccessFlags(barrier.srcAccess);
        bufferBarrier.dstAccessMask = UtilitiesVulkan::AccessTypeToVkAccessFlags(barrier.dstAccess);
        bufferBarrier.buffer = m_Handle;
        bufferBarrier.size = VK_WHOLE_SIZE;
        vkCmdPipelineBarrier(
            std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(),
            UtilitiesVulkan::PipelineStageToVkPipelineStageFlags(barrier.srcStage), // srcStage
            UtilitiesVulkan::PipelineStageToVkPipelineStageFlags(barrier.srcStage), // dstStage
            0,
            0, nullptr,
            1, &bufferBarrier,
            0, nullptr
        );
    }

	void BufferVulkan::Release() {
        BRISK_CORE_INFO("Destroying vma buffer");
		vmaDestroyBuffer(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetVmaAllocator(), m_Handle, m_Allocation);
	}
}