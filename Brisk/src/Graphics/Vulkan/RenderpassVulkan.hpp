#pragma once

#include "CommandBufferVulkan.hpp"
#include "Engine/Renderer/RenderPass.hpp"

#include <Volk/volk.h>
#include <iostream>

namespace Brisk 
{
    class RenderPassVulkan : public RenderPass {
    public:
        RenderPassVulkan() = default;
        virtual void Init(std::vector<std::shared_ptr<Texture>> inputs, std::vector<std::shared_ptr<Texture>> outputs) override;
        virtual void Bind() override;
    private:
        VkRenderPass renderPass;
        VkDevice device;
        VkFramebuffer framebuffer;
        VkCommandBuffer commandBuffer;
    };
}