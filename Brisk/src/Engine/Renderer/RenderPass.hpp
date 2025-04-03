#pragma once

#include <memory>

namespace Brisk 
{
    class RenderPass {
    public:
        virtual void Execute() = 0;
        virtual ~RenderPass() = default;
    };

    //-------------------------------------------------

    class GBufferPass : public RenderPass {
    public:
        virtual ~GBufferPass() = 0;

        static std::unique_ptr<RenderPass> Get();
    };

    class LightingPass : public RenderPass {
    public:
        virtual ~LightingPass() = 0;

        static std::unique_ptr<RenderPass> Get();
    };

    class CompositionPass : public RenderPass {
    public:
        virtual ~CompositionPass() = 0;

        static std::unique_ptr<RenderPass> Get();
    };

    class PostProcessingPass : public RenderPass {
    public:
        virtual ~PostProcessingPass() = 0;

        static std::unique_ptr<RenderPass> Get();
    };
};
