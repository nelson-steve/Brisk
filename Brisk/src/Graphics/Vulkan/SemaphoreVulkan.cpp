#include "SemaphoreVulkan.hpp"

namespace Brisk 
{
    void SemaphoreVulkan::Init() {
        VkSemaphoreCreateInfo semaphoreCreateInfo = {};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if (vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &mSemaphore) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan semaphore!");
        }
    }

    VkSemaphore SemaphoreVulkan::Get() {
        return mSemaphore;
    }
}