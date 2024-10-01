#pragma once
#include <cstdint>
#include <vector>

namespace Brisk 
{
    class RenderPass {
    public:
        enum class Format {
            Undefined,
            R8G8B8A8_UNORM,
            B8G8R8A8_UNORM,
            R32G32B32A32_SFLOAT,
            D24_UNORM_S8_UINT,
            D32_SFLOAT,
            RGBA,
            RGB,
            DepthStencil,
        };

        enum class LoadOp {
            Load,
            Clear,
            DontCare,
        };

        enum class StoreOp {
            Store,
            DontCare,
        };

        enum class Layout {
            Undefined,                    // Undefined layout (initial state)
            ColorAttachmentOptimal,        // Layout optimized for color attachment
            DepthStencilAttachmentOptimal, // Layout optimized for depth/stencil attachment
            ShaderReadOnlyOptimal,         // Layout optimized for shader reads
            TransferSrcOptimal,            // Layout optimized for transfer source
            TransferDstOptimal,            // Layout optimized for transfer destination
            PresentSrcKHR,                 // Layout for presenting the swapchain image
        };

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

        struct AttachmentRef {
            uint32_t attachmentIndex;
            Layout layout;
        };

        struct Attachment {
            uint32_t attachmentIndex;
            Format format;
            Samples samples;
            LoadOp loadOp;
            StoreOp storeOp;
            LoadOp stencilLoadOp;
            StoreOp stencilStoreOp;
            Layout initialLayout;
            Layout finalLayout;
        };

        struct AttachmentRef {
            uint32_t index;
            Layout layout;
        };

        struct SubPass {
            BindPoint bindPoint;
            std::vector<AttachmentRef> colorAttachments;
            std::vector<AttachmentRef> inputAttachments;
            AttachmentRef depthStencilAttachment;
        };

    private:
        std::vector<Attachment> attachments;
        std::vector<SubPass> subPasses;

    public:
        RenderPass() = default;
        virtual ~RenderPass() = default;

        virtual void initialize() = 0;
        virtual void begin() = 0;
        virtual void end() = 0;

        const std::vector<Attachment>& getAttachments() const { return attachments; }
        const std::vector<SubPass>& getSubPasses() const { return subPasses; }

    protected:
        void addAttachment(const Attachment& attachment) {
            attachments.push_back(attachment);
        }

        void addSubPass(const SubPass& subPass) {
            subPasses.push_back(subPass);
        }
    };

    // Template class for render pass specialization
    template <typename T>
    class RenderPassTemplate : public RenderPass {
    public:
        RenderPassTemplate() {
            // Custom initialization using the templated type
            T::initAttachments(*this);
            T::initSubPasses(*this);
        }

        // Call the templated methods for the pass lifecycle
        void initialize() override {
            T::initialize();
        }

        void begin() override {
            T::begin();
        }

        void end() override {
            T::end();
        }
    };

    // Example Vulkan-specific render pass
    class GBufferPassVulkan {
    public:
        // Define attachments specific to GBuffer in Vulkan
        static void initAttachments(RenderPass& pass) {
            pass.addAttachment({ 0, Format::RGBA, Samples::Sample1, LoadOp::Clear, StoreOp::Store, LoadOp::DontCare, StoreOp::DontCare, Layout::Undefined, Layout::ColorAttachmentOptimal });
            // Add more attachments as needed
        }

        // Define subpasses for GBuffer
        static void initSubPasses(RenderPass& pass) {
            RenderPass::SubPass subPass;
            subPass.bindPoint = BindPoint::Graphics;
            subPass.colorAttachments.push_back({ 0, Layout::ColorAttachmentOptimal });
            pass.addSubPass(subPass);
        }

        static void initialize() {
            // Specific Vulkan initialization for the GBuffer pass
        }

        static void begin() {
            // Commands to begin GBuffer render pass
        }

        static void end() {
            // Commands to end GBuffer render pass
        }
    };

    // Usage
    void createRenderPasses() {
        RenderPassTemplate<GBufferPassVulkan> gbufferRenderPass;
        gbufferRenderPass.initialize();
        gbufferRenderPass.begin();
        // Rendering work here
        gbufferRenderPass.end();
    }

};
