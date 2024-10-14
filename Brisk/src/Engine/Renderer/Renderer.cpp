#include "Renderer.hpp"
#include "Pipeline.hpp"
#include "RHI.hpp"
#include "RenderPass.hpp"
#include "Shader.hpp"

namespace Brisk 
{
	std::unique_ptr<Renderer> Renderer::Create() {
		//return std::make_unique<RendererVulkan>();
	}

	void Renderer::Init() {
        Pipeline::PipelineSpecs pipelineSpecs{};
        RenderPass::RenderPassSpecs renderPassSpecs;
        renderPassSpecs.pAttachments.push_back({ Core::Format::FORMAT_R8G8B8A8_SRGB, true, RenderPass::AttachmentType::Swapchain, false});
        renderPassSpecs.pAttachments.push_back({ Core::Format::FORMAT_D32_SFLOAT, true, RenderPass::AttachmentType::Depth, true });
        pipelineSpecs.pRenderPass = RenderPass::Create();

        std::shared_ptr<Shader> vertexShader = Shader::Create();
        vertexShader->Init(std::make_pair("Shaders/Vulkan/Compiled/TriangleVS.spv", Pipeline::ShaderStage::VERTEX));
        std::shared_ptr<Descriptor> descriptor = Descriptor::Create();
        descriptor->AddBindingLayout(0, 1, Descriptor::DescriptorType::UNIFORM);
        vertexShader->AddDescriptor(descriptor);

        std::shared_ptr<Shader> fragmentShader = Shader::Create();
        fragmentShader->Init(std::make_pair("Shaders/Vulkan/Compiled/TriangleFS.spv", Pipeline::ShaderStage::FRAGMENT));

        pipelineSpecs.pShaders.push_back(fragmentShader);

        std::shared_ptr<Pipeline> pipeline = Pipeline::Create();
        pipeline->Init(pipelineSpecs);
	}

    //void Renderer::Update() {

    //}

    //void Renderer::Destroy() {

    //}
}