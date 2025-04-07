#pragma once

#include "CommandBufferVulkan.hpp"
#include "Engine/Renderer/RenderPass.hpp"

#include <Volk/volk.h>
#include <iostream>

namespace Brisk 
{
    class RenderPassVulkan {
    public:
        RenderPassVulkan() = default;
        RenderPassVulkan(VkDevice device, VkRenderPass renderPass, VkFramebuffer framebuffer, VkCommandBuffer commandBuffer)
            : device(device), renderPass(renderPass), framebuffer(framebuffer), commandBuffer(commandBuffer) {
        }
        void Execute();
    private:
        VkDevice device;
        VkRenderPass renderPass;
        VkFramebuffer framebuffer;
        VkCommandBuffer commandBuffer;
    };
}