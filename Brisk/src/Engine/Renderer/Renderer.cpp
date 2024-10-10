#include "Renderer.hpp"
#include "Graphics/Vulkan/RenderPassVulkan.hpp"

namespace Brisk {
	std::unique_ptr<Renderer> Renderer::Create() {
		//return std::make_unique<RendererVulkan>();
	}

	void Renderer::Init() {
        Pipeline::PipelineSpecs specs{};
        Renderpass renderpass;
        renderpass->AddAttachment(Format::rgba, true, AttachmentType::Swapchain);
        renderpass->AddAttachment(Format::depth, true, AttachmentType::Depth);
        specs.renderpass = renderpass;
        specs.shader.add("Shaders/Vulkan/Compiled/TriangleVS.spv", VERTEX_STAGE);
        specs.shader.add("Shaders/Vulkan/Compiled/TriangleFS.spv", FRAGMENT_STAGE);
	}
}