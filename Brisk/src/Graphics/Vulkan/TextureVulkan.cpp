#include "TextureVulkan.hpp"
#include "UtilitiesVulkan.hpp"
#include "GpuAdapterVulkan.hpp"
#include "CommandBufferVulkan.hpp"
#include "Engine/Renderer/Buffer.hpp"

namespace Brisk 
{
    VkSampler TextureVulkan::m_Sampler;

    void TextureVulkan::Init(const TextureSpecification& specs) {
        m_Specs = specs;
        VmaAllocator cachedAllocator = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetVmaAllocator();

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
        VmaAllocationCreateInfo imageAllocInfo{};
        imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        if (vmaCreateImage(cachedAllocator, &imageinfo, &imageAllocInfo, &m_Image, &m_ImageAllocation, nullptr)) {
            throw std::runtime_error("Failed to create image!");
        }

        VkDebugUtilsObjectNameInfoEXT nameInfo = {};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = VK_OBJECT_TYPE_IMAGE;
        nameInfo.objectHandle = (uint64_t)m_Image;
        nameInfo.pObjectName = specs.p_DebugName.c_str();

#if _DEBUG
        vkSetDebugUtilsObjectNameEXT(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &nameInfo);
#endif

        // Image view
        VkImageViewCreateInfo imageView{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        imageView.pNext = nullptr;
        imageView.flags = 0;
        imageView.image = m_Image;
        if (specs.p_Type == TextureType::CUBEMAP)
            imageView.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        else if (specs.p_Type == TextureType::TEXTURE3D)
            imageView.viewType = VK_IMAGE_VIEW_TYPE_3D;
        else
            imageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageView.format = m_Format;
        imageView.subresourceRange.aspectMask = specs.p_IsDepth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        imageView.subresourceRange.baseMipLevel = 0;
        imageView.subresourceRange.levelCount = specs.p_MipLevels;
        imageView.subresourceRange.baseArrayLayer = 0;
        imageView.subresourceRange.layerCount = specs.p_ArrayLayers;

        if (vkCreateImageView(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &imageView, nullptr, &m_ImageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }

        nameInfo = {};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = VK_OBJECT_TYPE_IMAGE_VIEW;
        nameInfo.objectHandle = (uint64_t)m_ImageView;
        nameInfo.pObjectName = specs.p_DebugName.c_str();

#if _DEBUG
        vkSetDebugUtilsObjectNameEXT(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &nameInfo);
#endif

        // Init Sampler
        if(m_Sampler == VK_NULL_HANDLE)
        {
            VkSamplerCreateInfo samplerInfo{};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.magFilter = VK_FILTER_LINEAR;
            samplerInfo.minFilter = VK_FILTER_LINEAR;
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.minLod = 0.0f;
            samplerInfo.maxLod = static_cast<float>(specs.p_MipLevels);
            samplerInfo.maxAnisotropy = 1.0f;
            samplerInfo.anisotropyEnable = VK_TRUE;
            if (vkCreateSampler(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor pool!");
            }
        }

        m_Descriptor.sampler = m_Sampler;
        m_Descriptor.imageView = m_ImageView;
        m_Descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    void TextureVulkan::Init(const std::string& path) {
        int texWidth, texHeight, texChannels;
        void* m_Pixels;
        VkDeviceSize imageSize;
        if (stbi_is_hdr(path.c_str())) {
            float* pixels = stbi_loadf(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            imageSize = (texWidth * (4 * sizeof(float))) * texHeight;
            if (!pixels) {
                throw std::runtime_error("failed to load texture image!");
            }
            m_Pixels = pixels;
        }
        else {
            bool m_is_hdr = false;
            stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            imageSize = (texWidth * (4 * sizeof(unsigned char))) * texHeight;
            if (!pixels) {
                throw std::runtime_error("failed to load texture image!");
            }
            m_Pixels = pixels;
        }

        VmaAllocator cachedAllocator = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetVmaAllocator();

        if (!m_Pixels) {
            throw std::runtime_error("failed to load texture image!");
        }
        m_Specs.p_Width = texWidth;
        m_Specs.p_Width = texHeight;

        BufferVulkan stagingBuffer;
        stagingBuffer.Init(imageSize, m_Pixels, Core::BufferUsage::TransferSrc, Core::MemoryProperty::HostVisible | Core::MemoryProperty::HostCoherent, true);

        stbi_image_free(m_Pixels);

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
        VmaAllocationCreateInfo imageAllocInfo{};
        imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        if (vmaCreateImage(cachedAllocator, &imageinfo, &imageAllocInfo, &m_Image, &m_ImageAllocation, nullptr)) {
            throw std::runtime_error("Failed to create image!");
        }

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

        if (m_Sampler == VK_NULL_HANDLE)
        {
            VkSamplerCreateInfo samplerInfo{};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.magFilter = VK_FILTER_LINEAR;
            samplerInfo.minFilter = VK_FILTER_LINEAR;
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; // outside image bounds just use border color
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.minLod = 0;
            samplerInfo.maxLod = 1000;
            samplerInfo.maxAnisotropy = 1.0f;
            if (vkCreateSampler(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor pool!");
            }
        }

        {
            VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
            allocInfo.commandBufferCount = 1;
            allocInfo.commandPool = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetGraphicsCommandPool();
            VkCommandBuffer cmd;
            if (vkAllocateCommandBuffers(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &allocInfo, &cmd) != VK_SUCCESS) {
                throw std::runtime_error("Failed to allocate command buffer");
            }

            VkCommandBufferBeginInfo beingInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
            beingInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            if (vkBeginCommandBuffer(cmd, &beingInfo) != VK_SUCCESS) {
                throw std::runtime_error("Failed to begin command buffer");
            }

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

            if (vkEndCommandBuffer(cmd) != VK_SUCCESS) {
                throw std::runtime_error("Failed to end command buffer");
            }

            VkSubmitInfo submit{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
            submit.commandBufferCount = 1;
            submit.pCommandBuffers = &cmd;

            if (vkQueueSubmit(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetGraphicsQueue(), 1, &submit, VK_NULL_HANDLE) != VK_SUCCESS) {
                throw std::runtime_error("Failed to end command buffer");
            }
            vkQueueWaitIdle(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetGraphicsQueue());

            stagingBuffer.Release();

            vkFreeCommandBuffers(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(),
                Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetGraphicsCommandPool(), 1, &cmd);
        }
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

    void TextureVulkan::Init(const fastgltf::Image& image, const fastgltf::Asset& asset) {
        VkDevice deviceCached = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice();
        VkPhysicalDevice physicalDevice = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetPhysicalDevice();
        int width;
        int height;
        int nrChannels;
        std::visit(fastgltf::visitor{
            [&](const std::monostate&) {
                BRISK_CORE_ERROR("monostate::No image data : {}", image.name);
            },
            [&](const fastgltf::sources::URI&) {
                BRISK_CORE_ERROR("URI::No image data. : {}", image.name);
            },
            [&](const fastgltf::sources::Array& arrays) {
                auto start = std::chrono::high_resolution_clock::now();

                // Loading data from using stbi from memory
                unsigned char* imageData = stbi_load_from_memory((stbi_uc*)arrays.bytes.data(),
                    static_cast<int>(arrays.bytes.size_bytes()),
                    &width, &height, &nrChannels, 4);
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> duration = end - start;
                std::cout << "Time taken: " << duration.count() << " ms\n";
                int bufferSize = width * height * 4;

                VkCommandBuffer copyCmd;
                VkCommandBuffer blitCmd;
                VkCommandBufferAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                allocInfo.commandPool = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetGraphicsCommandPool();
                allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                allocInfo.commandBufferCount = 1;

                if (vkAllocateCommandBuffers(deviceCached, &allocInfo, &copyCmd) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to allocate command buffer!");
                }
                if (vkAllocateCommandBuffers(deviceCached, &allocInfo, &blitCmd) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to allocate command buffer!");
                }

                VkSemaphoreCreateInfo semaphoreInfo{};
                semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

                VkSemaphore copyFinishedSemaphore;
                if (vkCreateSemaphore(deviceCached, &semaphoreInfo, nullptr, &copyFinishedSemaphore) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create semaphore!");
                }

                VkFence fence;
                VkFenceCreateInfo fenceCreateInfo { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
                if (vkCreateFence(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &fenceCreateInfo, nullptr, &fence) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create fence");
                }

                VmaAllocator cachedAllocator = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetVmaAllocator();

                if (imageData) {
                    m_Specs.p_Width = width;
                    m_Specs.p_Height = height;

                    // Calculating mip levels based on width and height
                    m_MipLevels = static_cast<uint32_t>(floor(log2(std::max(width, height))) + 1.0);

                    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
                    VkFormatProperties formatProperties;
                    vkGetPhysicalDeviceFormatProperties(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetPhysicalDevice(), format, &formatProperties);
                    assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT);
                    assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT);

                    // Create staging buffer
                    VkBuffer stagingBuffer;
                    VmaAllocation stagingAllocation;
                    VkBufferCreateInfo stagingBufferInfo{};
                    stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                    stagingBufferInfo.size = bufferSize;
                    stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
                    stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

                    VmaAllocationCreateInfo stagingAllocInfo{};
                    stagingAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
                    if (vmaCreateBuffer(cachedAllocator, &stagingBufferInfo, &stagingAllocInfo, &stagingBuffer, &stagingAllocation, nullptr) != VK_SUCCESS) {
                        throw std::runtime_error("Failed to create staging buffer");
                    }

                    // Filling staging buffer with data
                    void* data;
                    if (vmaMapMemory(cachedAllocator, stagingAllocation, &data) != VK_SUCCESS) {
                        throw std::runtime_error("failed to map memory!");
                    }
                    memcpy(data, imageData, bufferSize);
                    vmaUnmapMemory(cachedAllocator, stagingAllocation);

                    // Creating the image
                    VkImageCreateInfo image_create_info{};
                    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                    image_create_info.imageType = VK_IMAGE_TYPE_2D;
                    image_create_info.format = format;
                    image_create_info.mipLevels = m_MipLevels;
                    image_create_info.arrayLayers = 1;
                    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
                    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
                    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
                    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    image_create_info.extent.width = m_Specs.p_Width;
                    image_create_info.extent.height = m_Specs.p_Height;
                    image_create_info.extent.depth = 1;
                    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

                    VmaAllocationCreateInfo imageAllocInfo{};
                    imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
                    if (vmaCreateImage(cachedAllocator, &image_create_info, &imageAllocInfo, &m_Image, &m_ImageAllocation, nullptr)) {
                        throw std::runtime_error("Failed to create image!");
                    }

                    // Copying data from staging buffer to image
                    VkCommandBufferBeginInfo beginInfo = {};
                    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                    beginInfo.pInheritanceInfo = nullptr;

                    if (vkBeginCommandBuffer(copyCmd, &beginInfo) != VK_SUCCESS) {
                        throw std::runtime_error("Failed to begin command buffer!");
                    }

                    VkImageSubresourceRange subresource_range = {};
                    subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    subresource_range.levelCount = 1;
                    subresource_range.layerCount = 1;

                    // Transition image to TransferDst layout
                    VkImageMemoryBarrier transferDstImageMemoryBarrier{};
                    transferDstImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    transferDstImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    transferDstImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                    transferDstImageMemoryBarrier.srcAccessMask = 0;
                    transferDstImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    transferDstImageMemoryBarrier.image = m_Image;
                    transferDstImageMemoryBarrier.subresourceRange = subresource_range;
                    vkCmdPipelineBarrier(copyCmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &transferDstImageMemoryBarrier);

                    VkBufferImageCopy bufferRegion = {};
                    bufferRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    bufferRegion.imageSubresource.mipLevel = 0;
                    bufferRegion.imageSubresource.baseArrayLayer = 0;
                    bufferRegion.imageSubresource.layerCount = 1;
                    bufferRegion.imageExtent.width = m_Specs.p_Width;
                    bufferRegion.imageExtent.height = m_Specs.p_Height;
                    bufferRegion.imageExtent.depth = 1;

                    vkCmdCopyBufferToImage(copyCmd, stagingBuffer, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferRegion);

                    // Transition image to TransferSrc for generating mipmaps
                    VkImageMemoryBarrier transferSrcImageMemoryBarrier{};
                    transferSrcImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    transferSrcImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                    transferSrcImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                    transferSrcImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    transferSrcImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                    transferSrcImageMemoryBarrier.image = m_Image;
                    transferSrcImageMemoryBarrier.subresourceRange = subresource_range;
                    vkCmdPipelineBarrier(copyCmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &transferSrcImageMemoryBarrier);

                    if (vkEndCommandBuffer(copyCmd) != VK_SUCCESS) {
                        throw std::runtime_error("Failed to end command buffer!");
                    }

                    VkCommandBufferBeginInfo beginBlitInfo = {};
                    beginBlitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                    beginBlitInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                    beginBlitInfo.pInheritanceInfo = nullptr;

                    if (vkBeginCommandBuffer(blitCmd, &beginBlitInfo) != VK_SUCCESS) {
                        throw std::runtime_error("Failed to begin command buffers!");
                    }

                    for (uint32_t i = 1; i < m_MipLevels; i++) {
                        VkImageBlit imageBlit{};

                        imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                        imageBlit.srcSubresource.layerCount = 1;
                        imageBlit.srcSubresource.mipLevel = i - 1;
                        imageBlit.srcOffsets[1].x = int32_t(m_Specs.p_Width >> (i - 1));
                        imageBlit.srcOffsets[1].y = int32_t(m_Specs.p_Height >> (i - 1));
                        imageBlit.srcOffsets[1].z = 1;

                        imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                        imageBlit.dstSubresource.layerCount = 1;
                        imageBlit.dstSubresource.mipLevel = i;
                        imageBlit.dstOffsets[1].x = int32_t(m_Specs.p_Width >> i);
                        imageBlit.dstOffsets[1].y = int32_t(m_Specs.p_Height >> i);
                        imageBlit.dstOffsets[1].z = 1;

                        VkImageSubresourceRange mipSubRange = {};
                        mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                        mipSubRange.baseMipLevel = i;
                        mipSubRange.levelCount = 1;
                        mipSubRange.layerCount = 1;

                        VkImageMemoryBarrier mipTransferDstImageMemoryBarrier{};
                        mipTransferDstImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                        mipTransferDstImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                        mipTransferDstImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                        mipTransferDstImageMemoryBarrier.srcAccessMask = 0;
                        mipTransferDstImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                        mipTransferDstImageMemoryBarrier.image = m_Image;
                        mipTransferDstImageMemoryBarrier.subresourceRange = mipSubRange;
                        vkCmdPipelineBarrier(blitCmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &mipTransferDstImageMemoryBarrier);

                        vkCmdBlitImage(blitCmd, m_Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);

                        VkImageMemoryBarrier mipTransferSrcImageMemoryBarrier{};
                        mipTransferSrcImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                        mipTransferSrcImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                        mipTransferSrcImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                        mipTransferSrcImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                        mipTransferSrcImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                        mipTransferSrcImageMemoryBarrier.image = m_Image;
                        mipTransferSrcImageMemoryBarrier.subresourceRange = mipSubRange;
                        vkCmdPipelineBarrier(blitCmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &mipTransferSrcImageMemoryBarrier);
                    }

                    subresource_range.levelCount = m_MipLevels;
                    VkImageLayout layout;

                    VkImageMemoryBarrier readOnlyImageMemoryBarrier{};
                    readOnlyImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    readOnlyImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                    readOnlyImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    readOnlyImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                    readOnlyImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                    readOnlyImageMemoryBarrier.image = m_Image;
                    readOnlyImageMemoryBarrier.subresourceRange = subresource_range;
                    vkCmdPipelineBarrier(blitCmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &readOnlyImageMemoryBarrier);

                    if (vkEndCommandBuffer(blitCmd) != VK_SUCCESS) {
                        throw std::runtime_error("Failed to end command buffer!");
                    }

                    // Submit copy cmd
                    {
                        VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
                        submitInfo.commandBufferCount = 1;
                        submitInfo.pCommandBuffers = &copyCmd;
                        submitInfo.signalSemaphoreCount = 1;
                        submitInfo.pSignalSemaphores = &copyFinishedSemaphore;

                        if (vkQueueSubmit(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetGraphicsQueue(), 1, &submitInfo, fence) != VK_SUCCESS) {
                            throw std::runtime_error("Failed to submit copy command!");
                        }

                        vkWaitForFences(deviceCached, 1, &fence, VK_TRUE, UINT64_MAX);
                        vkResetFences(deviceCached, 1, &fence);
                    }

                    // Submit blit cmd
                    {
                        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                        VkSubmitInfo submitInfo { VK_STRUCTURE_TYPE_SUBMIT_INFO };
                        submitInfo.commandBufferCount = 1;
                        submitInfo.pCommandBuffers = &blitCmd;
                        submitInfo.waitSemaphoreCount = 1;
                        submitInfo.pWaitSemaphores = &copyFinishedSemaphore;
                        submitInfo.pWaitDstStageMask = &waitStage;

                        if (vkQueueSubmit(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetGraphicsQueue(), 1, &submitInfo, fence) != VK_SUCCESS) {
                            throw std::runtime_error("Failed to submit blit command!");
                        }

                        vkWaitForFences(deviceCached, 1, &fence, VK_TRUE, UINT64_MAX);
                        vkResetFences(deviceCached, 1, &fence);
                    }
                        
                    vmaDestroyBuffer(cachedAllocator, stagingBuffer, stagingAllocation);
                    vkResetCommandBuffer(copyCmd, 0);
                    vkResetCommandBuffer(blitCmd, 0);
                    vkDestroySemaphore(deviceCached, copyFinishedSemaphore, nullptr);
                    vkDestroyFence(deviceCached, fence, nullptr);
                    vkQueueWaitIdle(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetGraphicsQueue());

                    VkImageViewCreateInfo viewInfo{};
                    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                    viewInfo.image = m_Image;
                    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                    viewInfo.format = format;
                    viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
                    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    viewInfo.subresourceRange.layerCount = 1;
                    viewInfo.subresourceRange.levelCount = 1;
                    if (vkCreateImageView(deviceCached, &viewInfo, nullptr, &m_ImageView)) {
                        throw std::runtime_error("Failed to create image view!");
                    }

                    stbi_image_free(imageData);
                }

                m_Descriptor.sampler = m_Sampler;
                m_Descriptor.imageView = m_ImageView;
                m_Descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                //std::cout << "Array::No image data.\n";
            },
            [&](const fastgltf::sources::Vector&) {
                std::cout << "Vector::No image data.\n";
            },
            [&](const fastgltf::sources::CustomBuffer&) {
                std::cout << "CustomBuffer::No image data.\n";
            },
            [&](const fastgltf::sources::ByteView&) {
                std::cout << "ByteView::No image data.\n";
            },
            [&](const fastgltf::sources::Fallback&) {
                std::cout << "Fallback::No image data.\n";
            },
            [&](const fastgltf::sources::BufferView& view) {
                auto& bufferView = asset.bufferViews[view.bufferViewIndex];
                auto& buffer = asset.buffers[bufferView.bufferIndex];
                std::visit(fastgltf::visitor{
                    [](auto& arg) {},
                    [&](fastgltf::sources::Vector& vector) {
                        //
                    }
                },buffer.data);
            },
        }, image.data);
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

    void TextureVulkan::Release() {
        if (m_Sampler != VK_NULL_HANDLE) {
            vkDestroySampler(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), m_Sampler, nullptr);
            m_Sampler = VK_NULL_HANDLE;
        }

        VmaAllocator cachedAllocator = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetVmaAllocator();
        vmaDestroyImage(cachedAllocator, m_Image, m_ImageAllocation);
        vkDestroyImageView(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), m_ImageView, nullptr);
    }
}