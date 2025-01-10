#include "SemaphoreVulkan.hpp"

#include "Engine/Engine.hpp"
#include "GpuAdapterVulkan.hpp"

namespace Brisk 
{
    void SemaphoreVulkan::Init() {
        VkSemaphoreCreateInfo semaphoreCreateInfo = {};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if (vkCreateSemaphore(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), &semaphoreCreateInfo, nullptr, &mSemaphore) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan semaphore!");
        }
    }

    VkSemaphore SemaphoreVulkan::Get() {
        return mSemaphore;
    }
}