#pragma once

#include "CommandBufferDirectX12.hpp"
#include "Engine/Renderer/RenderPass.hpp"

namespace Brisk 
{
    class RenderPassDirectX12 : public RenderPass {
    public:
        virtual void Execute() override {
            std::cout << "Executing RenderPassDirectX12\n";
        }
    };

    class GBufferPass : public RenderPassDirectX12 {
    public:
        void Execute() override;
    };

    class LightingPass : public RenderPassDirectX12 {
    public:
        void Execute() override {
        }
    };

    class CompositionPass : public RenderPassDirectX12 {
    public:
        void Execute() override {
        }
    };

    class PostProcessingPass : public RenderPassDirectX12 {
    public:
        void Execute() override {
        }
    };
}
