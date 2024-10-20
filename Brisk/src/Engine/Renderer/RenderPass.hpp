#pragma once

#include "CommandBuffer.hpp"
#include "RHI.hpp"

#include <cstdint>
#include <vector>
#include <memory>

namespace Brisk 
{
    class RenderPass {
    public:
        enum AttachmentType {
            Swapchain,
            Depth,
        };

        struct Attachment {
            uint32_t pAttachmentIndex;
            Core::Format pFormat;
            bool pClear;
            AttachmentType pType;
        };

        struct RenderPassSpecs {
            std::vector<Attachment> pAttachments;
        };

    public:
        enum class Samples {
            Sample1 = 1,  // No multisampling
            Sample2 = 2,
            Sample4 = 4,
            Sample8 = 8,
            Sample16 = 16,
        };

        enum class BindPoint {
            Graphics,
            Compute,
        };
    public:
        virtual ~RenderPass() = default;

        virtual void Init(const RenderPassSpecs& specs) = 0;
        virtual void Begin(std::shared_ptr<CommandBuffer> cmd, uint32_t imageIndex) = 0;
        virtual void End(std::shared_ptr<CommandBuffer> cmd) = 0;

        static std::shared_ptr<RenderPass> Create();
    };
};
