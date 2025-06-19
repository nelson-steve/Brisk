#include "TextureVulkan.hpp"
#include "UtilitiesVulkan.hpp"
#include "GpuAdapterVulkan.hpp"
#include "CommandBufferVulkan.hpp"

namespace Brisk 
{
    VkSampler TextureVulkan::m_Sampler;

    void TextureVulkan::Init(const TextureSpecification& specs) {
        m_Specs = specs;
        VkImageCreateInfo imageinfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        imageinfo.pNext = nullptr;
        imageinfo.flags = specs.p_Type == Texture::TextureType::CUBEMAP ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
        imageinfo.imageType = specs.p_Type == Texture::TextureType::TEXTURE3D ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D;
        m_Format = UtilitiesVulkan::FormatToVkFormat(specs.p_Format);
        imageinfo.format = m_Format;
        imageinfo.extent.width = specs.p_Width;
        imageinfo.extent.height = specs.p_Height;
        imageinfo.extent.depth = specs.p_Depth;
        imageinfo.mipLevels = specs.p_MipLevels;
        imageinfo.arrayLayers = specs.p_ArrayLayers; 
        imageinfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageinfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageinfo.usage = UtilitiesVulkan::ImageUsageToVulkanType(specs.p_Usage);
        imageinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageinfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        if (vkCreateImage(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &imageinfo, nullptr, &m_Image) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image");
        }

        VkDebugUtilsObjectNameInfoEXT nameInfo = {};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = VK_OBJECT_TYPE_IMAGE;
        nameInfo.objectHandle = (uint64_t)m_Image;
        nameInfo.pObjectName = specs.p_DebugName.c_str();

#if _DEBUG
        vkSetDebugUtilsObjectNameEXT(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &nameInfo);
#endif

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), m_Image, &memRequirements);
        //vkGetImageMemoryRequirements(static_cast<VkDevice>(Engine::s_Application->GetNativeDevice()), m_Image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = UtilitiesVulkan::FindMemoryType(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetPhysicalDevice(), memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &allocInfo, nullptr, &m_Memory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), m_Image, m_Memory, 0);

        // Image view
        VkImageViewCreateInfo imageView{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        imageView.pNext = nullptr;
        imageView.flags = 0;
        imageView.image = m_Image;
        imageView.viewType = specs.p_Type == Texture::TextureType::TEXTURE3D ? VK_IMAGE_VIEW_TYPE_3D : VK_IMAGE_VIEW_TYPE_2D;
        imageView.format = m_Format;
        imageView.subresourceRange.aspectMask = specs.p_IsDepth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        imageView.subresourceRange.baseMipLevel = 0;
        imageView.subresourceRange.levelCount = 1;
        imageView.subresourceRange.baseArrayLayer = 0;
        imageView.subresourceRange.layerCount = specs.p_ArrayLayers;

        if (vkCreateImageView(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &imageView, nullptr, &m_ImageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }

        // Init Sampler
        if(m_Sampler == VK_NULL_HANDLE)
        {
            VkSamplerCreateInfo samplerInfo{};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.magFilter = VK_FILTER_LINEAR;
            samplerInfo.minFilter = VK_FILTER_LINEAR;
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; // outside image bounds just use border color
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.minLod = -1000;
            samplerInfo.maxLod = 1000;
            samplerInfo.maxAnisotropy = 1.0f;
            if (vkCreateSampler(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor pool!");
            }
        }

        m_Descriptor.sampler = m_Sampler;
        m_Descriptor.imageView = m_ImageView;
/*        if ((specs.p_Usage & Texture::TextureUsage::ImageUsageColorAttachment) != Texture::TextureUsage::Undefined) {
            m_Descriptor.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
        else */
        if ((specs.p_Usage & Core::TextureUsage::ImageUsageSampled) != Core::TextureUsage::Undefined) {
            m_Descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
    }

    void TextureVulkan::Init(const std::string& path) {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }
        m_Specs.p_Width = texWidth;
        m_Specs.p_Width = texHeight;

        BufferVulkan stagingBuffer;
        //stagingBuffer.Init(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        //stagingBuffer.Allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        //void* data;
        //vkMapMemory(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), stagingBuffer.GetMemory(), 0, imageSize, 0, &data);
        //memcpy(data, pixels, static_cast<size_t>(imageSize));
        VkMappedMemoryRange range[1] = {};
        range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        //range[0].memory = stagingBuffer.GetMemory();
        range[0].size = imageSize;
        if (vkFlushMappedMemoryRanges(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), 1, range) != VK_SUCCESS) {
            throw std::runtime_error("error");
        }
        //vkUnmapMemory(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), stagingBuffer.GetMemory());

        stbi_image_free(pixels);

        VkImageCreateInfo imageinfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        imageinfo.pNext = nullptr;
        imageinfo.flags = 0;
        imageinfo.imageType = VK_IMAGE_TYPE_2D;
        imageinfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        imageinfo.extent.width = texWidth;
        imageinfo.extent.height = texHeight;
        imageinfo.extent.depth = 1;
        imageinfo.mipLevels = 1;
        imageinfo.arrayLayers = 1;
        imageinfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageinfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageinfo.usage = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageinfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        if (vkCreateImage(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &imageinfo, nullptr, &m_Image) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), m_Image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = UtilitiesVulkan::FindMemoryType(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetPhysicalDevice(), memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &allocInfo, nullptr, &m_Memory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), m_Image, m_Memory, 0);

        // Image view
        VkImageViewCreateInfo imageView{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        imageView.pNext = nullptr;
        imageView.flags = 0;
        imageView.image = m_Image;
        imageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageView.format = VK_FORMAT_R8G8B8A8_UNORM;
        imageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageView.subresourceRange.baseMipLevel = 0;
        imageView.subresourceRange.levelCount = 1;
        imageView.subresourceRange.baseArrayLayer = 0;
        imageView.subresourceRange.layerCount = 1;

        if (vkCreateImageView(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &imageView, nullptr, &m_ImageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }

        // Init Sampler
        {
            VkSamplerCreateInfo samplerInfo{};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.magFilter = VK_FILTER_LINEAR;
            samplerInfo.minFilter = VK_FILTER_LINEAR;
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; // outside image bounds just use border color
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.minLod = -1000;
            samplerInfo.maxLod = 1000;
            samplerInfo.maxAnisotropy = 1.0f;
            if (vkCreateSampler(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor pool!");
            }
        }

        VkCommandPool pool;
        VkCommandPoolCreateInfo graphicsPoolInfo{};
        graphicsPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        graphicsPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        graphicsPoolInfo.queueFamilyIndex = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetGraphicsQueueFamily();

        if (vkCreateCommandPool(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &graphicsPoolInfo, nullptr, &pool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }

        {
            VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
            allocInfo.commandBufferCount = 1;
            allocInfo.commandPool = pool;
            VkCommandBuffer cmd;
            if (vkAllocateCommandBuffers(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &allocInfo, &cmd) != VK_SUCCESS) {
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
                VK_PIPELINE_STAGE_HOST_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

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
            vkCmdCopyBufferToImage(cmd, stagingBuffer.Get(), m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

            VkImageMemoryBarrier barrier1{};
            barrier1.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier1.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier1.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier1.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier1.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier1.image = m_Image;
            barrier1.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier1.subresourceRange.baseMipLevel = 0;
            barrier1.subresourceRange.levelCount = 1;
            barrier1.subresourceRange.baseArrayLayer = 0;
            barrier1.subresourceRange.layerCount = 1;

            vkCmdPipelineBarrier(cmd,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier1);

            vkEndCommandBuffer(cmd);

            VkSubmitInfo submit{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
            submit.commandBufferCount = 1;
            submit.pCommandBuffers = &cmd;

            vkQueueSubmit(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetGraphicsQueue(), 1, &submit, VK_NULL_HANDLE);
            vkQueueWaitIdle(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetGraphicsQueue());

            vkFreeCommandBuffers(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), pool, 1, &cmd);
        }
        // Release staging buffer
    }
    void TextureVulkan::TransitionImageLayout(std::shared_ptr<CommandBuffer> cmd, std::vector<ImageBarrierParams> params) {
        VkPipelineStageFlags srcFlag = UtilitiesVulkan::PipelineStageToVkPipelineStageFlags(params[0].srcStage);
        VkPipelineStageFlags dstFlag = UtilitiesVulkan::PipelineStageToVkPipelineStageFlags(params[0].dstStage);
        std::vector<VkImageMemoryBarrier> barriers;
        for (auto& p : params) {
            VkImageMemoryBarrier barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
            barrier.srcAccessMask = UtilitiesVulkan::AccessTypeToVkAccessFlags(p.srcAccess);
            barrier.dstAccessMask = UtilitiesVulkan::AccessTypeToVkAccessFlags(p.dstAccess);
            barrier.oldLayout = UtilitiesVulkan::ImageLayoutToVkImageLayout(p.oldLayout);
            barrier.newLayout = UtilitiesVulkan::ImageLayoutToVkImageLayout(p.newLayout);
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bool isDepth = false;
            barrier.image = m_Image;
            if (m_Format == VK_FORMAT_D16_UNORM)
                isDepth = true;
            barrier.subresourceRange.aspectMask = isDepth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;

            barriers.push_back(barrier);

            m_Descriptor.imageLayout = UtilitiesVulkan::ImageLayoutToVkImageLayout(p.newLayout);
        }

        vkCmdPipelineBarrier(
            std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), 
            srcFlag,
            dstFlag,
            0, 0, nullptr, 0, nullptr, barriers.size(), barriers.data());
    }

    void TextureVulkan::CopyImage(std::shared_ptr<CommandBuffer> cmd, std::shared_ptr<Texture> src, std::shared_ptr<Texture> dest, uint32_t width, uint32_t height) {
        VkImageCopy copyRegion = {};
        copyRegion.extent = { width, height, 1 };
        copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.srcSubresource.layerCount = 6;
        copyRegion.dstSubresource = copyRegion.srcSubresource;
        //vkCmdCopyImage(layoutCmd,
        //    m_cubemap.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        //    m_env_texuture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        //    1, &copyRegion);
    }
}