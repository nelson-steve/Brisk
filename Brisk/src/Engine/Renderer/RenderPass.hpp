#pragma once

// INCLUDES
#include "CommandBuffer.hpp"
#include "Graphics/Swapchain.hpp"
#include "RHI.hpp"
#include "Graphics/Texture.hpp"
//---------------
#include <cstdint>
#include <vector>
#include <memory>
//--------------

namespace Brisk 
{
    class RenderPass {
        DEFINE_BASE_CLASS_CONSTRUCTOR(RenderPass)
    public:
        enum AttachmentType {
            //Swapchain,
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
        virtual void AddRenderTarget(std::shared_ptr<Swapchain> swapchain) = 0;
        virtual void AddRenderTarget(std::shared_ptr<Texture> texture) = 0;
        //void AddRenderTarget();
        virtual void Init(const RenderPassSpecs& specs) = 0;
        virtual void Begin(std::shared_ptr<CommandBuffer> cmd, uint32_t imageIndex) = 0;
        virtual void End(std::shared_ptr<CommandBuffer> cmd) = 0;

        static std::shared_ptr<RenderPass> Create();


    };
};
