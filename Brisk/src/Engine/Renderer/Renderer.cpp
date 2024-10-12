#include "Renderer.hpp"
#include "Pipeline.hpp"
#include "RHI.hpp"
#include "RenderPass.hpp"

namespace Brisk 
{
	std::unique_ptr<Renderer> Renderer::Create() {
		//return std::make_unique<RendererVulkan>();
	}

	void Renderer::Init() {
        Pipeline::PipelineSpecs specs{};
        RenderPass::RenderPassSpecs renderPassSpecs;
        renderPassSpecs.pAttachments.push_back({ Core::Format::FORMAT_R8G8B8A8_SRGB, true, RenderPass::AttachmentType::Swapchain, false});
        renderPassSpecs.pAttachments.push_back({ Core::Format::FORMAT_D32_SFLOAT, true, RenderPass::AttachmentType::Depth, true });
        specs.pRenderPass = RenderPass::Create(renderPassSpecs);
        specs.pShaders.push_back(std::make_pair("Shaders/Vulkan/Compiled/TriangleVS.spv", Pipeline::ShaderStage::VERTEX));
        specs.pShaders.push_back(std::make_pair("Shaders/Vulkan/Compiled/TriangleFS.spv", Pipeline::ShaderStage::FRAGMENT));

        std::shared_ptr<Pipeline> pipeline = Pipeline::Create(specs);
	}

    //void Renderer::Update() {

    //}

    //void Renderer::Destroy() {

    //}
}