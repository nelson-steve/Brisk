// INCLUDES
#include "pch.hpp"
#include "FenceVulkan.hpp"
#include "GpuAdapterVulkan.hpp"
//-----------------------------

namespace Brisk 
{
    void FenceVulkan::Init() {
        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        //fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Optional: create as signaled if needed

        VkResult result = vkCreateFence(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), &fenceCreateInfo, nullptr, &m_Fence);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create fence");
        }
    }

    void FenceVulkan::Wait() {
        vkWaitForFences(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), 1, &m_Fence, VK_TRUE, UINT64_MAX);
    }

    void FenceVulkan::Reset() {
        vkResetFences(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), 1, &m_Fence);
    }

    void FenceVulkan::Release() {
        vkDestroyFence(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), m_Fence, nullptr);
    }
}