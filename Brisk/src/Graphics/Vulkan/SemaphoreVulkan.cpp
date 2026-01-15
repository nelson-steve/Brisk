#include "pch.hpp"
#include "SemaphoreVulkan.hpp"

#include "Engine/Engine.hpp"
#include "GpuAdapterVulkan.hpp"

namespace Brisk 
{
    void SemaphoreVulkan::Init() {
        VkSemaphoreCreateInfo semaphoreCreateInfo = {};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if (vkCreateSemaphore(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), &semaphoreCreateInfo, nullptr, &m_Semaphore) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan semaphore!");
        }
    }

    void SemaphoreVulkan::Release() {
        vkDestroySemaphore(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), m_Semaphore, nullptr);
    }

    VkSemaphore SemaphoreVulkan::Get() {
        return m_Semaphore;
    }
}