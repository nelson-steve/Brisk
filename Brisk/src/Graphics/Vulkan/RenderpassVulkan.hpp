#pragma once

#include "CommandBufferVulkan.hpp"
#include "Engine/Renderer/RenderPass.hpp"

#include <Volk/volk.h>

namespace Brisk 
{
    // API-Specific Implementations
    class RenderPassVulkan : public RenderPass {
    public:
        virtual void Execute() override {
            std::cout << "Executing VulkanRenderPass\n";
        }
    };

    // Deferred Renderer Passes for Vulkan
    class GBufferPass : public RenderPassVulkan {
    public:
        GBufferPass(VkDevice device, VkRenderPass renderPass, VkFramebuffer framebuffer, VkCommandBuffer commandBuffer)
            : device(device), renderPass(renderPass), framebuffer(framebuffer), commandBuffer(commandBuffer) {
        }

        void Execute() override;
    private:
        VkDevice device;
        VkRenderPass renderPass;
        VkFramebuffer framebuffer;
        VkCommandBuffer commandBuffer;
    };

    class LightingPass : public RenderPassVulkan {
    public:
        void Execute() override {
        }
    };

    class CompositionPass : public RenderPassVulkan {
    public:
        void Execute() override {
        }
    };

    class PostProcessingPass : public RenderPassVulkan {
    public:
        void Execute() override {
        }
    };
}