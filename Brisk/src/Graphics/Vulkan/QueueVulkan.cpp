#pragma once

#include "QueueVulkan.hpp"

namespace Brisk 
{
    void QueueVulkan::Submit(SubmitInfo submitInfo, std::shared_ptr<Fence> fence) {
        std::vector<VkSemaphore> waitSemaphores(submitInfo.pWaitSemaphores.size());
        std::vector<VkSemaphore> signalSemaphores(submitInfo.pSignalSemaphores.size());
        std::vector<VkPipelineStageFlags> waitStages(submitInfo.pWaitStages.size());

        for (size_t i = 0; i < submitInfo.pWaitStages.size(); ++i) {
            waitStages[i] = static_cast<VkPipelineStageFlags>(submitInfo.pWaitStages[i]);
        }

        for (size_t i = 0; i < submitInfo.pWaitSemaphores.size(); ++i) {
            waitSemaphores[i] = submitInfo.pWaitSemaphores[i]->GetHandle();
        }
        for (size_t i = 0; i < submitInfo.pSignalSemaphores.size(); ++i) {
            signalSemaphores[i] = submitInfo.pSignalSemaphores[i]->GetHandle();
        }

        std::vector<VkCommandBuffer> commandBuffers(submitInfo.pCmdBuffers.size());
        for (size_t i = 0; i < submitInfo.pCmdBuffers.size(); ++i) {
            commandBuffers[i] = submitInfo.pCmdBuffers[i]->GetHandle();
        }

        // Create submit info struct for Vulkan
        VkSubmitInfo submitInfoVk = {};
        submitInfoVk.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfoVk.waitSemaphoreCount = static_cast<uint32_t>(submitInfo.pWaitSemaphores.size());
        submitInfoVk.pWaitSemaphores = waitSemaphores.data();
        submitInfoVk.pWaitDstStageMask = waitStages.data();
        submitInfoVk.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
        submitInfoVk.pCommandBuffers = commandBuffers.data();
        submitInfoVk.signalSemaphoreCount = static_cast<uint32_t>(submitInfo.pSignalSemaphores.size());
        submitInfoVk.pSignalSemaphores = signalSemaphores.data();

        VkFence vkFence = fence ? fence->GetHandle() : VK_NULL_HANDLE;

        VkResult result = vkQueueSubmit(mQueue, 1, &submitInfoVk, vkFence);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to submit command buffers to Vulkan queue");
        }
    }

    void QueueVulkan::Present(Queue::PresentInfo info) {
        VkPresentInfoKHR presentInfoVk = {};
        presentInfoVk.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfoVk.pWaitSemaphores = waitSemaphores.data();
        VkSwapchainKHR swapChains[] = {std::static_pointer_cast<SwapchainVulkan>(m_Swapchain)->GetSwapchain()};
        presentInfoVk.swapchains = {swapchains};
        presentInfoVk.image = &info.imageIndex;

        VkResult result = vkQueuePresentKHR(mQueue, &presentInfoVk);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to submit command buffers to Vulkan queue");
        }        
    }
}
