#include "TextureVulkan.hpp"
#include "UtilitiesVulkan.hpp"
#include "GpuAdapterVulkan.hpp"
#include "CommandBufferVulkan.hpp"

namespace Brisk 
{
    //void TextureVulkan::Init() {
    //    assert(m_Width != 0 && m_Height != 0);
    //    VkImageCreateInfo imageinfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    //    imageinfo.pNext = nullptr;
    //    imageinfo.flags = 0;
    //    imageinfo.imageType = VK_IMAGE_TYPE_2D;
    //    imageinfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    //    imageinfo.extent.width = m_Width;
    //    imageinfo.extent.height = m_Height;
    //    imageinfo.extent.depth = 1;
    //    imageinfo.mipLevels = 1;
    //    imageinfo.arrayLayers = 1;
    //    imageinfo.samples = VK_SAMPLE_COUNT_1_BIT;
    //    imageinfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    //    imageinfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    //    imageinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    //    imageinfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //    if (vkCreateImage(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &imageinfo, nullptr, &m_Image) != VK_SUCCESS) {
    //        throw std::runtime_error("Failed to create image");
    //    }

    //    VkMemoryRequirements memRequirements;
    //    vkGetImageMemoryRequirements(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), m_Image, &memRequirements);

    //    VkMemoryAllocateInfo allocInfo{};
    //    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    //    allocInfo.allocationSize = memRequirements.size;
    //    allocInfo.memoryTypeIndex = UtilitiesVulkan::FindMemoryType(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetPhysicalDevice(), memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    //    if (vkAllocateMemory(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &allocInfo, nullptr, &m_Memory) != VK_SUCCESS) {
    //        throw std::runtime_error("failed to allocate image memory!");
    //    }

    //    vkBindImageMemory(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), m_Image, m_Memory, 0);

    //    // Image view
    //    VkImageViewCreateInfo imageView{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    //    imageView.pNext = nullptr;
    //    imageView.flags = 0;
    //    imageView.image = m_Image;
    //    imageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
    //    imageView.format = VK_FORMAT_R8G8B8A8_UNORM;
    //    imageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //    imageView.subresourceRange.baseMipLevel = 0;
    //    imageView.subresourceRange.levelCount = 1;
    //    imageView.subresourceRange.baseArrayLayer = 0;
    //    imageView.subresourceRange.layerCount = 1;

    //    if (vkCreateImageView(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &imageView, nullptr, &m_ImageView) != VK_SUCCESS) {
    //        throw std::runtime_error("failed to create descriptor pool!");
    //    }

    //    VkSamplerCreateInfo samplerInfo{};
    //    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    //    samplerInfo.magFilter = VK_FILTER_LINEAR;
    //    samplerInfo.minFilter = VK_FILTER_LINEAR;
    //    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    //    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; // outside image bounds just use border color
    //    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    //    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    //    samplerInfo.minLod = -1000;
    //    samplerInfo.maxLod = 1000;
    //    samplerInfo.maxAnisotropy = 1.0f;
    //    if (vkCreateSampler(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS) {
    //        throw std::runtime_error("failed to create descriptor pool!");
    //    }
    //}

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

        vkSetDebugUtilsObjectNameEXT(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &nameInfo);


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
        if ((specs.p_Usage & Texture::TextureUsage::ImageUsageSampled) != Texture::TextureUsage::Undefined) {
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
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetGraphicsQueue().FamilyIndex;

        if (vkCreateCommandPool(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &poolInfo, nullptr, &pool) != VK_SUCCESS) {
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

            vkQueueSubmit(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetGraphicsQueue().Handle, 1, &submit, VK_NULL_HANDLE);
            vkQueueWaitIdle(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetGraphicsQueue().Handle);

            vkFreeCommandBuffers(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), pool, 1, &cmd);
        }

        //stagingBuffer.Release();
    }

    void TextureVulkan::Init(tinygltf::Image image, TextureSampler sampler) {
        m_DeviceCached = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice();
        VkPhysicalDevice physicalDevice = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetPhysicalDevice();

        unsigned char* buffer = nullptr;
        bool delete_buffer = false;
        VkDeviceSize buffer_size = 0;
        if (image.component == 3) {
            buffer_size = image.width * image.height * 4;
            buffer = new unsigned char[buffer_size];
            unsigned char* rgba = buffer;
            unsigned char* rgb = &image.image[0];
            for (int i = 0; i < image.width * image.height; i++) {
                for (int j = 0; j < 3; j++) {
                    rgba[j] = rgb[j];
                }
                rgba += 4;
                rgb += 3;
            }
            delete_buffer = true;
        }
        else {
            buffer = &image.image[0];
            buffer_size = image.image.size();
        }

        VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

        VkFormatProperties formatProperties;

        m_Specs.p_Width = image.width;
        m_Specs.p_Height = image.height;
        //m_mip_levels = static_cast<uint32_t>(floor(log2(std::max(m_width, m_height))) + 1.0);

        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);
        assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT);
        assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT);

        VkMemoryAllocateInfo memAllocInfo{};
        memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        VkMemoryRequirements memReqs{};

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingMemory;

        VkBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = buffer_size;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        if (vkCreateBuffer(m_DeviceCached, &bufferCreateInfo, nullptr, &stagingBuffer)) {
            throw std::runtime_error("failed to create buffer!");
        }
        vkGetBufferMemoryRequirements(m_DeviceCached, stagingBuffer, &memReqs);
        memAllocInfo.allocationSize = memReqs.size;
        memAllocInfo.memoryTypeIndex = UtilitiesVulkan::FindMemoryType(physicalDevice, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        if (vkAllocateMemory(m_DeviceCached, &memAllocInfo, nullptr, &stagingMemory)) {
            throw std::runtime_error("failed to load texture image!");
        }
        if (vkBindBufferMemory(m_DeviceCached, stagingBuffer, stagingMemory, 0)) {
            throw std::runtime_error("failed to load texture image!");
        }

        uint8_t* data;
        if (vkMapMemory(m_DeviceCached, stagingMemory, 0, memReqs.size, 0, (void**)&data)) {
            throw std::runtime_error("failed to map memory!");
        }
        memcpy(data, buffer, buffer_size);
        vkUnmapMemory(m_DeviceCached, stagingMemory);

        VkImageCreateInfo image_create_info{};
        image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.format = format;
        //image_create_info.mipLevels = m_mip_levels;
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = 1;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
        image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_create_info.extent.width = m_Specs.p_Width;
        image_create_info.extent.height = m_Specs.p_Height;
        image_create_info.extent.depth = 1;
        image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        if (vkCreateImage(m_DeviceCached, &image_create_info, nullptr, &m_Image)) {
            throw std::runtime_error("failed to create image!");
        }
        vkGetImageMemoryRequirements(m_DeviceCached, m_Image, &memReqs);
        memAllocInfo.allocationSize = memReqs.size;
        memAllocInfo.memoryTypeIndex = UtilitiesVulkan::FindMemoryType(physicalDevice, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        if (vkAllocateMemory(m_DeviceCached, &memAllocInfo, nullptr, &m_Memory)) {
            throw std::runtime_error("failed to allocate memory!");
        }
        if (vkBindImageMemory(m_DeviceCached, m_Image, m_Memory, 0)) {
            throw std::runtime_error("failed to find memory!");
        }

        VkCommandPool cmdPool;
        VkCommandPoolCreateInfo commandPoolCreateInfo = {};
        commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolCreateInfo.queueFamilyIndex = 0;  // Assuming 0 is the queue family index for graphics/transfer
        commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        if (vkCreateCommandPool(m_DeviceCached, &commandPoolCreateInfo, nullptr, &cmdPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }

        VkCommandBuffer copy_cmd;
        {
            VkCommandBufferAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = cmdPool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;  // Primary command buffer
            allocInfo.commandBufferCount = 1;

            if (vkAllocateCommandBuffers(m_DeviceCached, &allocInfo, &copy_cmd) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate command buffers!");
            }

            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;  // One-time submission flag
            beginInfo.pInheritanceInfo = nullptr;

            if (vkBeginCommandBuffer(copy_cmd, &beginInfo) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate command buffers!");
            }
        }

        VkImageSubresourceRange subresource_range = {};
        subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresource_range.levelCount = 1;
        subresource_range.layerCount = 1;

        {
            VkImageMemoryBarrier image_memory_barrier{};
            image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            image_memory_barrier.srcAccessMask = 0;
            image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            image_memory_barrier.image = m_Image;
            image_memory_barrier.subresourceRange = subresource_range;
            vkCmdPipelineBarrier(copy_cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
        }

        VkBufferImageCopy buffer_copy_region = {};
        buffer_copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        buffer_copy_region.imageSubresource.mipLevel = 0;
        buffer_copy_region.imageSubresource.baseArrayLayer = 0;
        buffer_copy_region.imageSubresource.layerCount = 1;
        buffer_copy_region.imageExtent.width = m_Specs.p_Width;
        buffer_copy_region.imageExtent.height = m_Specs.p_Height;
        buffer_copy_region.imageExtent.depth = 1;

        vkCmdCopyBufferToImage(copy_cmd, stagingBuffer, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_copy_region);

        {
            VkImageMemoryBarrier image_memory_barrier{};
            image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            image_memory_barrier.image = m_Image;
            image_memory_barrier.subresourceRange = subresource_range;
            vkCmdPipelineBarrier(copy_cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
        }


        {
            if (vkEndCommandBuffer(copy_cmd) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate command buffers!");
            }

            VkSubmitInfo submitInfo = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &copy_cmd;

            if (vkQueueSubmit(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetGraphicsQueue().Handle, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate command buffers!");
            }
        }

        // TODO: implement this
        //device->flushCommandBuffer(copyCmd, copyQueue, true);

        // Generate the mip chain (glTF uses jpg and png, so we need to create this manually)
        //VkCommandBuffer blit_cmd = m_graphics_device->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
        //for (uint32_t i = 1; i < m_mip_levels; i++) {
        //    VkImageBlit imageBlit{};

        //    imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        //    imageBlit.srcSubresource.layerCount = 1;
        //    imageBlit.srcSubresource.mipLevel = i - 1;
        //    imageBlit.srcOffsets[1].x = int32_t(m_width >> (i - 1));
        //    imageBlit.srcOffsets[1].y = int32_t(m_height >> (i - 1));
        //    imageBlit.srcOffsets[1].z = 1;

        //    imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        //    imageBlit.dstSubresource.layerCount = 1;
        //    imageBlit.dstSubresource.mipLevel = i;
        //    imageBlit.dstOffsets[1].x = int32_t(m_width >> i);
        //    imageBlit.dstOffsets[1].y = int32_t(m_height >> i);
        //    imageBlit.dstOffsets[1].z = 1;

        //    VkImageSubresourceRange mipSubRange = {};
        //    mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        //    mipSubRange.baseMipLevel = i;
        //    mipSubRange.levelCount = 1;
        //    mipSubRange.layerCount = 1;

        //    {
        //        VkImageMemoryBarrier imageMemoryBarrier{};
        //        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        //        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        //        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        //        imageMemoryBarrier.srcAccessMask = 0;
        //        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        //        imageMemoryBarrier.image = m_texture_image;
        //        imageMemoryBarrier.subresourceRange = mipSubRange;
        //        vkCmdPipelineBarrier(blit_cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
        //    }

        //    vkCmdBlitImage(blit_cmd, m_texture_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_texture_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);

        //    {
        //        VkImageMemoryBarrier imageMemoryBarrier{};
        //        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        //        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        //        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        //        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        //        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        //        imageMemoryBarrier.image = m_texture_image;
        //        imageMemoryBarrier.subresourceRange = mipSubRange;
        //        vkCmdPipelineBarrier(blit_cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
        //    }
        //}

        VkCommandBuffer blit_cmd;
        VkCommandBufferAllocateInfo allocBlitInfo = {};
        allocBlitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocBlitInfo.commandPool = cmdPool;
        allocBlitInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;  // Primary command buffer
        allocBlitInfo.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(m_DeviceCached, &allocBlitInfo, &blit_cmd) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }

        VkCommandBufferBeginInfo beginBlitInfo = {};
        beginBlitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginBlitInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;  // One-time submission flag
        beginBlitInfo.pInheritanceInfo = nullptr;

        if (vkBeginCommandBuffer(blit_cmd, &beginBlitInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }

        //

        //subresource_range.levelCount = m_mip_levels;
        VkImageLayout layout;
        layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        {
            VkImageMemoryBarrier imageMemoryBarrier{};
            imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            imageMemoryBarrier.image = m_Image;
            imageMemoryBarrier.subresourceRange = subresource_range;
            vkCmdPipelineBarrier(blit_cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
        }

        //m_graphics_device->FlushCommandBuffer(blit_cmd, copy_queue, true);
        {
            // Step 5: End Command Buffer Recording
            if (vkEndCommandBuffer(blit_cmd) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate command buffers!");
            }

            VkSubmitInfo submitInfo = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &blit_cmd;

            if (vkQueueSubmit(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetGraphicsQueue().Handle, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate command buffers!");
            }
        }

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = (VkFilter) sampler.mag_filter;
        samplerInfo.minFilter = (VkFilter)sampler.min_filter;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.addressModeU = (VkSamplerAddressMode)sampler.address_modeU;
        samplerInfo.addressModeV = (VkSamplerAddressMode)sampler.address_modeV;
        samplerInfo.addressModeW = (VkSamplerAddressMode)sampler.address_modeW;
        samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        samplerInfo.maxAnisotropy = 1.0;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxLod = (float)1;
        samplerInfo.maxAnisotropy = 8.0f;
        if (vkCreateSampler(m_DeviceCached, &samplerInfo, nullptr, &m_Sampler)) {
            throw std::runtime_error("failed to create sampler!");
        }

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_Image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.layerCount = 1;
        viewInfo.subresourceRange.levelCount = 1;
        if (vkCreateImageView(m_DeviceCached, &viewInfo, nullptr, &m_ImageView)) {
            throw std::runtime_error("failed to create image view!");
        }

        m_Descriptor.sampler = m_Sampler;
        m_Descriptor.imageView = m_ImageView;
        m_Descriptor.imageLayout = layout;

        if (delete_buffer)
            delete[] buffer;

        //vkFreeMemory(m_DeviceCached, stagingMemory, nullptr);
        //vkDestroyBuffer(m_DeviceCached, stagingBuffer, nullptr);
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
            if (p.texture == nullptr) {
                BRISK_APP_INFO("No special image found for transition");
                barrier.image = m_Image;
            }
            else {
                barrier.image = std::static_pointer_cast<TextureVulkan>(p.texture)->GetImage();
            }
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
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