#pragma once

#include "CommandBufferVulkan.hpp"
#include "Engine/Renderer/RenderPass.hpp"

#include <Volk/volk.h>
#include <iostream>

namespace Brisk 
{
    class GBufferPassVulkan : public GBufferPass {
    public:
        GBufferPassVulkan() = default;
        GBufferPassVulkan(VkDevice device, VkRenderPass renderPass, VkFramebuffer framebuffer, VkCommandBuffer commandBuffer)
            : device(device), renderPass(renderPass), framebuffer(framebuffer), commandBuffer(commandBuffer) {
        }
        void Execute() override;
    private:
        VkDevice device;
        VkRenderPass renderPass;
        VkFramebuffer framebuffer;
        VkCommandBuffer commandBuffer;
    };

    class LightingPassVulkan : public LightingPass {
    public:
        void Execute() override;
    };

    class CompositionPassVulkan : public CompositionPass {
    public:
        void Execute() override;
    };

    class PostProcessingPassVulkan : public PostProcessingPass {
    public:
        void Execute() override;
    };
}