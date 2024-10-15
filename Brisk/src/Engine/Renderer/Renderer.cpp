#include "Renderer.hpp"
#include "Pipeline.hpp"
#include "RHI.hpp"
#include "RenderPass.hpp"
#include "Shader.hpp"
#include "Engine/Model.hpp"

namespace Brisk 
{
	void Renderer::Init() {
        Pipeline::PipelineSpecs pipelineSpecs{};
        RenderPass::RenderPassSpecs renderPassSpecs;
        renderPassSpecs.pAttachments.push_back({ 0, Core::Format::FORMAT_R8G8B8A8_SRGB, true, RenderPass::AttachmentType::Swapchain });
        renderPassSpecs.pAttachments.push_back({ 1, Core::Format::FORMAT_D32_SFLOAT, true, RenderPass::AttachmentType::Depth });

        Pipeline::VertexDataLayout vertexLayout;
        vertexLayout.pBinding = 0;
        vertexLayout.pStride = sizeof(Vertex);
        vertexLayout.pAttributes = {
            {0, 0,Core::Format::FORMAT_R32G32B32_SFLOAT,     offsetof(Vertex, Vertex::pos)},
            {0, 1,Core::Format::FORMAT_R32G32B32_SFLOAT,     offsetof(Vertex, Vertex::normal)},
            {0, 2,Core::Format::FORMAT_R32G32_SFLOAT,        offsetof(Vertex, Vertex::uv0)},
            {0, 3,Core::Format::FORMAT_R32G32_SFLOAT,        offsetof(Vertex, Vertex::uv1)},
            {0, 4,Core::Format::FORMAT_R32G32B32_SFLOAT,  offsetof(Vertex, Vertex::color)},
        };
        pipelineSpecs.Layout = vertexLayout;
        //renderPassSpecs.pAttachments =
        //{
        //    { 0, Core::Format::FORMAT_R8G8B8A8_SRGB, true, RenderPass::AttachmentType::Swapchain },
        //    { 1, Core::Format::FORMAT_D32_SFLOAT, true, RenderPass::AttachmentType::Depth }
        //};

        pipelineSpecs.pRenderPass = RenderPass::Create();
        pipelineSpecs.pRenderPass->Init(renderPassSpecs);

        std::shared_ptr<Shader> vertexShader = Shader::Create();
        vertexShader->Init(std::make_pair("Shaders/Vulkan/Compiled/TriangleVS.spv", Pipeline::ShaderStage::VERTEX));
        std::shared_ptr<Descriptor> descriptor = Descriptor::Create();
        descriptor->AddBindingLayout(0, 1, Descriptor::DescriptorType::UNIFORM);
        descriptor->Init();
        vertexShader->AddDescriptor(descriptor);

        std::shared_ptr<Shader> fragmentShader = Shader::Create();
        fragmentShader->Init(std::make_pair("Shaders/Vulkan/Compiled/TriangleFS.spv", Pipeline::ShaderStage::FRAGMENT));

        pipelineSpecs.pShaders.push_back(vertexShader);
        pipelineSpecs.pShaders.push_back(fragmentShader);

        pipelineSpecs.pLineWidth = 1.0f;
        std::shared_ptr<Pipeline> pipeline = Pipeline::Create();
        pipeline->Init(pipelineSpecs);
	}

    //void Renderer::Update() {

    //}

    //void Renderer::Destroy() {

    //}
}