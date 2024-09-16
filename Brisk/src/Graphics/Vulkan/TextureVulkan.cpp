#include "TextureVulkan.hpp"
#include "BufferVulkan.hpp"
#include "GpuContextVulkan.hpp"
#include "VulkanUtilities.hpp"

#include <stdexcept>

namespace Brisk {
    void TextureVulkan::Create(const std::string& path) {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }

        BufferVulkan buffer;
        buffer.Allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        buffer.Create(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

        void* data;
        vkMapMemory(GpuContextVulkan::s_GPUDevice->GetDevice(), buffer.GetMemory(), 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(GpuContextVulkan::s_GPUDevice->GetDevice(), buffer.GetMemory());

        stbi_image_free(pixels);

        VkImageCreateInfo imageinfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        imageinfo.pNext = nullptr;
        imageinfo.flags = 0;
        imageinfo.imageType = VK_IMAGE_TYPE_2D;
        imageinfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        imageinfo.extent.width = texWidth;
        imageinfo.extent.height = texHeight;
        imageinfo.mipLevels = 1;
        imageinfo.arrayLayers = 1;
        imageinfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageinfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageinfo.usage = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageinfo.queueFamilyIndexCount = 0;
        imageinfo.pQueueFamilyIndices = 0;
        imageinfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        if (vkCreateImage(GpuContextVulkan::s_GPUDevice->GetDevice(), &imageinfo, nullptr, &m_Image) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(GpuContextVulkan::s_GPUDevice->GetDevice(), m_Image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex =VulkanUtilities::FindMemoryType(GpuContextVulkan::s_GPUDevice->GetPhysicalDevice(), memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(GpuContextVulkan::s_GPUDevice->GetDevice(), &allocInfo, nullptr, &m_Memory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(GpuContextVulkan::s_GPUDevice->GetDevice(), m_Image, m_Memory, 0);

        VkCommandPool pool;
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = GpuContextVulkan::s_GPUDevice->GetGraphicsQueue().FamilyIndex;

        if (vkCreateCommandPool(GpuContextVulkan::s_GPUDevice->GetDevice(), &poolInfo, nullptr, &pool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }

        {
            VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
            allocInfo.commandBufferCount = 1;
            allocInfo.commandPool = pool;
            VkCommandBuffer cmd;
            if (vkAllocateCommandBuffers(GpuContextVulkan::s_GPUDevice->GetDevice(), nullptr, &cmd) != VK_SUCCESS) {
                throw std::runtime_error("Failed to allocate command buffer");
            }

            VkCommandBufferBeginInfo beingInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
            beingInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkBeginCommandBuffer(cmd, &beingInfo);
            // transition the image from undefined to transfer dst optimal
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.image = m_Image;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            vkCmdPipelineBarrier(cmd,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

            vkEndCommandBuffer(cmd);

            VkSubmitInfo submit{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
            submit.commandBufferCount = 1;
            submit.pCommandBuffers = &cmd;

            vkQueueSubmit(GpuContextVulkan::s_GPUDevice->GetGraphicsQueue().Handle, 1, &submit, VK_NULL_HANDLE);
            vkQueueWaitIdle(GpuContextVulkan::s_GPUDevice->GetGraphicsQueue().Handle);

            vkFreeCommandBuffers(GpuContextVulkan::s_GPUDevice->GetDevice(), pool, 1, &cmd);
        }

        {
            VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
            allocInfo.commandBufferCount = 1;
            allocInfo.commandPool = pool;
            VkCommandBuffer cmd;
            if (vkAllocateCommandBuffers(GpuContextVulkan::s_GPUDevice->GetDevice(), nullptr, &cmd) != VK_SUCCESS) {
                throw std::runtime_error("Failed to allocate command buffer");
            }

            VkCommandBufferBeginInfo beingInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
            beingInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkBeginCommandBuffer(cmd, &beingInfo);

            VkBufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = { 0, 0, 0 };
            region.imageExtent = {
                (uint32_t)texWidth,
                (uint32_t)texHeight,
                1
            };
            vkCmdCopyBufferToImage(cmd, m_StagingBuffer, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

            vkEndCommandBuffer(cmd);

            VkSubmitInfo submit{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
            submit.commandBufferCount = 1;
            submit.pCommandBuffers = &cmd;

            vkQueueSubmit(GpuContextVulkan::s_GPUDevice->GetGraphicsQueue().Handle, 1, &submit, VK_NULL_HANDLE);
            vkQueueWaitIdle(GpuContextVulkan::s_GPUDevice->GetGraphicsQueue().Handle);

            vkFreeCommandBuffers(GpuContextVulkan::s_GPUDevice->GetDevice(), pool, 1, &cmd);
        }

        {
            VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
            allocInfo.commandBufferCount = 1;
            allocInfo.commandPool = pool;
            VkCommandBuffer cmd;
            if (vkAllocateCommandBuffers(GpuContextVulkan::s_GPUDevice->GetDevice(), nullptr, &cmd) != VK_SUCCESS) {
                throw std::runtime_error("Failed to allocate command buffer");
            }

            VkCommandBufferBeginInfo beingInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
            beingInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkBeginCommandBuffer(cmd, &beingInfo);
            // transition the image from undefined to transfer dst optimal
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.image = m_Image;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            vkCmdPipelineBarrier(cmd,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

            vkEndCommandBuffer(cmd);

            VkSubmitInfo submit{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
            submit.commandBufferCount = 1;
            submit.pCommandBuffers = &cmd;

            vkQueueSubmit(GpuContextVulkan::s_GPUDevice->GetGraphicsQueue().Handle, 1, &submit, VK_NULL_HANDLE);
            vkQueueWaitIdle(GpuContextVulkan::s_GPUDevice->GetGraphicsQueue().Handle);

            vkFreeCommandBuffers(GpuContextVulkan::s_GPUDevice->GetDevice(), pool, 1, &cmd);
        }

        vkDestroyBuffer(GpuContextVulkan::s_GPUDevice->GetDevice(), m_StagingBuffer, nullptr);
        vkFreeMemory(GpuContextVulkan::s_GPUDevice->GetDevice(), m_StagingBufferMemory, nullptr);
    }
}