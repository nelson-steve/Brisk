#pragma once

#include "QueueVulkan.hpp"
#include "SwapchainVulkan.hpp"
#include "SemaphoreVulkan.hpp"
#include "CommandBufferVulkan.hpp"
#include "FenceVulkan.hpp"

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
            waitSemaphores[i] = std::static_pointer_cast<SemaphoreVulkan>(submitInfo.pWaitSemaphores[i])->Get();
        }
        for (size_t i = 0; i < submitInfo.pSignalSemaphores.size(); ++i) {
            signalSemaphores[i] = std::static_pointer_cast<SemaphoreVulkan>(submitInfo.pSignalSemaphores[i])->Get();
        }

        std::vector<VkCommandBuffer> commandBuffers(submitInfo.pCmdBuffers.size());
        for (size_t i = 0; i < submitInfo.pCmdBuffers.size(); ++i) {
            commandBuffers[i] = std::static_pointer_cast<CommandBufferVulkan>(submitInfo.pCmdBuffers[i])->Get();
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

        VkFence vkFence = fence ? std::static_pointer_cast<FenceVulkan>(fence)->Get() : VK_NULL_HANDLE;

        VkResult result = vkQueueSubmit(std::static_pointer_cast<GpuAdapterVulkan> (Engine::s_Application->GetGpuAdapter())->GetGraphicsQueue().Handle, 1, &submitInfoVk, vkFence);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to submit command buffers to Vulkan queue");
        }
    }

    void QueueVulkan::Present(Queue::PresentInfo info) {
        std::vector<VkSemaphore> waitSemaphores(info.pWaitSemaphores.size());

        for (size_t i = 0; i < info.pWaitSemaphores.size(); ++i) {
            waitSemaphores[i] = std::static_pointer_cast<SemaphoreVulkan>(info.pWaitSemaphores[i])->Get();
        }

        std::vector<VkSwapchainKHR> swapchains(info.pSwapchains.size());
        for (size_t i = 0; i < info.pSwapchains.size(); ++i) {
            swapchains[i] = std::static_pointer_cast<SwapchainVulkan>(info.pSwapchains[i])->GetSwapchain();
        }

        VkPresentInfoKHR presentInfoVk = {};
        presentInfoVk.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfoVk.pWaitSemaphores = waitSemaphores.data();
        VkSwapchainKHR swapChains[] = { std::static_pointer_cast<SwapchainVulkan>(info.pSwapchains[0])->GetSwapchain() };
        presentInfoVk.pSwapchains = { swapChains };
        presentInfoVk.swapchainCount = 1;
        uint32_t indices = info.pImageIndex;
        presentInfoVk.pImageIndices = &indices;

        VkResult result = vkQueuePresentKHR(std::static_pointer_cast<GpuAdapterVulkan> (Engine::s_Application->GetGpuAdapter())->GetGraphicsQueue().Handle, &presentInfoVk);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to submit command buffers to Vulkan queue");
        }        
    }
}
