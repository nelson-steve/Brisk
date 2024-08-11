#pragma once

#include <Volk/volk.h>

class VulkanQueue {
public:
    VulkanQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex = 0);
    ~VulkanQueue();

    // Submits a command buffer to the queue
    void Submit(VkCommandBuffer commandBuffer, VkFence fence = VK_NULL_HANDLE);

    // Waits for the queue to finish executing
    void WaitIdle();

    // Returns the underlying VkQueue handle
    VkQueue GetQueueHandle() const { return queue_; }

private:
    VkDevice device_;
    VkQueue queue_;
};