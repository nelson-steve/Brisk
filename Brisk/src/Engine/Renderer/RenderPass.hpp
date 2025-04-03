#pragma once

namespace Brisk 
{
    class RenderPass {
    public:
        virtual void Execute() = 0;
        virtual ~RenderPass() = default;
    };

    //-----------------------------------------------------

    class GBufferPass : public RenderPass {
    public:
        virtual ~GBufferPass() = 0;
    };

    class LightingPass : public RenderPass {
    public:
        virtual ~LightingPass() = 0;
    };

    class CompositionPass : public RenderPass {
    public:
        virtual ~CompositionPass() = 0;
    };

    class PostProcessingPass : public RenderPass {
    public:
        virtual ~PostProcessingPass() = 0;
    };
};
