#pragma once

// INCLUDES
#include "QueueVulkan.hpp"
#include "SwapchainVulkan.hpp"
#include "SemaphoreVulkan.hpp"
#include "CommandBufferVulkan.hpp"
#include "FenceVulkan.hpp"
#include "UtilitiesVulkan.hpp"
//-----------------------

namespace Brisk 
{
    void QueueVulkan::Init(Queue::QueueType type) {
        switch (type)
        {
            case Brisk::Queue::QueueType::Graphics:
                m_Queue = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetGraphicsQueue();
                break;
            case Brisk::Queue::QueueType::Compute:
                m_Queue = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetComputeQueue();
                break;
            case Brisk::Queue::QueueType::Transfer:
                m_Queue = Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetTransferQueue();
                break;
            default:
                BRISK_CORE_ERROR("Invalid Queue Type");
                break;
        }
    }

    void QueueVulkan::Submit(SubmitInfo submitInfo, std::shared_ptr<Fence> fence) {
        std::vector<VkSemaphore> waitSemaphores(submitInfo.pWaitSemaphores.size());
        std::vector<VkSemaphore> signalSemaphores(submitInfo.pSignalSemaphores.size());
        std::vector<VkPipelineStageFlags> waitStages(submitInfo.pWaitStages.size());

        BRISK_CORE_ASSERT(waitSemaphores.size() == waitStages.size());

        for (size_t i = 0; i < submitInfo.pWaitStages.size(); ++i) {
            waitStages[i] = UtilitiesVulkan::PipelineStageToVkPipelineStageFlags(submitInfo.pWaitStages[i]);
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
        submitInfoVk.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
        submitInfoVk.pWaitSemaphores = waitSemaphores.data();
        submitInfoVk.pWaitDstStageMask = waitStages.data();
        submitInfoVk.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
        submitInfoVk.pCommandBuffers = commandBuffers.data();
        submitInfoVk.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
        submitInfoVk.pSignalSemaphores = signalSemaphores.data();

        VkFence vkFence = fence ? std::static_pointer_cast<FenceVulkan>(fence)->Get() : VK_NULL_HANDLE;

        VkResult result = vkQueueSubmit(m_Queue, 1, &submitInfoVk, vkFence);
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
        presentInfoVk.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
        presentInfoVk.pWaitSemaphores = waitSemaphores.data();
        VkSwapchainKHR swapChains[] = { std::static_pointer_cast<SwapchainVulkan>(info.pSwapchains[0])->GetSwapchain() };
        presentInfoVk.pSwapchains = { swapChains };
        presentInfoVk.swapchainCount = 1;
        uint32_t indices = info.pImageIndex;
        presentInfoVk.pImageIndices = &indices;

        VkResult result = vkQueuePresentKHR(m_Queue, &presentInfoVk);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to submit command buffers to Vulkan queue");
        }        
    }
}
