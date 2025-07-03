#include "SemaphoreVulkan.hpp"

#include "Engine/Engine.hpp"
#include "GpuAdapterVulkan.hpp"

namespace Brisk 
{
    void SemaphoreVulkan::Init() {
        VkSemaphoreCreateInfo semaphoreCreateInfo = {};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if (vkCreateSemaphore(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &semaphoreCreateInfo, nullptr, &m_Semaphore) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan semaphore!");
        }
    }

    void SemaphoreVulkan::Release() {
        vkDestroySemaphore(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), m_Semaphore, nullptr);
    }

    VkSemaphore SemaphoreVulkan::Get() {
        return m_Semaphore;
    }
}