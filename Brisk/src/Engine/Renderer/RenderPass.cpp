// INCLUDES
#include "Graphics/Vulkan/RenderPassVulkan.hpp"
//--------------------------------------------

namespace Brisk 
{
    GBufferPass::~GBufferPass() {
    }

    std::unique_ptr<RenderPass> GBufferPass::Get() { 
        return std::make_unique<GBufferPassVulkan>(); 
    }

    //
    LightingPass::~LightingPass() {
    }

    std::unique_ptr<RenderPass> LightingPass::Get() {
        return std::make_unique<LightingPassVulkan>();
    }

    //
    CompositionPass::~CompositionPass() {
    }

    std::unique_ptr<RenderPass> CompositionPass::Get() {
        return std::make_unique<CompositionPassVulkan>();
    }

    //
    PostProcessingPass::~PostProcessingPass() {
    }

    std::unique_ptr<RenderPass> PostProcessingPass::Get() {
        return std::make_unique<PostProcessingPassVulkan>();
    }
}