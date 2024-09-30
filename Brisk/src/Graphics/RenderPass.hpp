#pragma once
#include <cstdint>

namespace Brisk {
    class RenderPass {
    public:

    public:
        struct Attachment {
            uint32_t pAttachmentIndex;
            Format pFormat;
            Samples pSamples;
            LoadOp pLoadOp;
            StoreOp pStoreOp;
            LoadOp pStencilLoadOp;
            StoreOp pStencilStoreOp;
            Layout pInitialLayout;
            Layout pFinalLayout;
        }
        struct AttachmentRef {
            uint32_t Index;
            Layout pLayout;
        }

        struct SubPass {
            BindPoint pBindPoint;
            AttachmentRef pAttachmentRef;
        }
    private:
        std::vector<Attachment> mAttachments;
        std::vector<SubPass> mSubPasses;
    }
}

template<typename T>
class Renderpass {
    init() { T::init(); }
    being() { T::being(); }
    end() { T::end(); }
}

class gbufferPasVulkan {

}