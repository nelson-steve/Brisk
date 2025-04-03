#pragma once

#include "CommandBufferVulkan.hpp"
#include "Engine/Renderer/RenderPass.hpp"

#include <Volk/volk.h>
#include <iostream>

namespace Brisk 
{
    class GBufferPassVulkan : public GBufferPass {
    public:
        GBufferPassVulkan(VkDevice device, VkRenderPass renderPass, VkFramebuffer framebuffer, VkCommandBuffer commandBuffer)
            : device(device), renderPass(renderPass), framebuffer(framebuffer), commandBuffer(commandBuffer) {
        }
        virtual ~GBufferPassVulkan() = 0;
        void Execute() override;
    private:
        VkDevice device;
        VkRenderPass renderPass;
        VkFramebuffer framebuffer;
        VkCommandBuffer commandBuffer;
    };

    class LightingPassVulkan : public LightingPass {
    public:
        virtual ~LightingPassVulkan() = 0;
    };

    class CompositionPassVulkan : public CompositionPass {
    public:
        virtual ~CompositionPassVulkan() = 0;
    };

    class PostProcessingPassVulkan : public PostProcessingPass {
    public:
        virtual ~PostProcessingPassVulkan() = 0;
    };
}