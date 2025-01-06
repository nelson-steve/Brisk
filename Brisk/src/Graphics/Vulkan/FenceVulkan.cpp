#include "FenceVulkan.hpp"

namespace Brisk 
{
    void FenceVulkan::Init() {
        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Optional: create as signaled if needed

        VkResult result = vkCreateFence(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), &fenceCreateInfo, nullptr, &mFence);
        if (result != VK_SUCCESS) {
            // Handle error
        }
    }

    void FenceVulkan::Wait() {
        vkWaitForFences(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), 1, &mFence, VK_TRUE, UINT64_MAX);
    }

    void FenceVulkan::Reset() {
        vkResetFences(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), 1, &mFence);
    }
}