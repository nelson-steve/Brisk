#pragma once

#include "IBLTextureMaker.hpp"

namespace Brisk
{
	void IBLTextureMaker::Init() {
		m_ComputePipeline = std::make_shared<Pipeline>();

        int mapLevel = 1;
		std::shared_ptr<DescriptorLayout> descriptorLayout = DescriptorLayout::Create();
        descriptorLayout->pName = "material";
        descriptorLayout->AddBindingLayout(0, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { GPUResource::ShaderStageAccess::SHADER_STAGE_COMPUTE_BIT });
        descriptorLayout->AddBindingLayout(1, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_STORAGE_IMAGE, { GPUResource::ShaderStageAccess::SHADER_STAGE_COMPUTE_BIT });
        descriptorLayout->AddBindingLayout(2, mapLevel - 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_STORAGE_IMAGE, { GPUResource::ShaderStageAccess::SHADER_STAGE_COMPUTE_BIT });
        descriptorLayout->Init();

		std::shared_ptr<ShaderModule> computeShaderModule = ShaderModule::Create();
        computeShaderModule->Init(std::make_pair("Shaders/Vulkan/Compiled/EquirectangularToCubemap.spv", Pipeline::ShaderStage::COMPUTE));

        Pipeline::ComputePipelineSpecs pipelineSpecs{};
		pipelineSpecs.pDescriptorLayouts["textures"] = descriptorLayout;
        pipelineSpecs.pShaderModule = computeShaderModule;

        m_ComputePipeline = Pipeline::Create();
        m_ComputePipeline->Init(pipelineSpecs);

        std::shared_ptr<Texture> InputTexture;
        std::shared_ptr<Texture> OutputTexture;

        std::shared_ptr<CommandBuffer> SingleTimeCommandBuffer;
        SingleTimeCommandBuffer->Bind();

        m_ComputePipeline->Bind(InputTexture, 0);
        m_ComputePipeline->Bind(OutputTexture, 1);

        InputTexture->BindAs();

        m_ComputePipeline->Bind();
        m_Shader->BindResources();
        m_ComputePipeline->DisaptchCompute();

        OutputTexture->BindAs();

        SingleTimeCommandBuffer->Execute();

        m_ComputePipeline->Destroy();

        OutputTexture->BindAs();
        EnvTexture->BindAs();

        CopyImage(OutputTexture, EnvTexture);

        OutputTexture->BindAs();
        EnvTexture->BindAs(); // final cubemap


        //
        std::shared_ptr<DescriptorLayout> materialLayout = DescriptorLayout::Create();
        materialLayout->pName = "material";
        materialLayout->AddBindingLayout(0, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_UNIFORM_BUFFER, { GPUResource::ShaderStageAccess::SHADER_STAGE_VERTEX_BIT });
        materialLayout->Init();

        std::shared_ptr<DescriptorLayout> pbrLayout = DescriptorLayout::Create();
        pbrLayout->pName = "pbr";
        pbrLayout->AddBindingLayout(0, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
        pbrLayout->AddBindingLayout(1, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
        pbrLayout->AddBindingLayout(2, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
        pbrLayout->Init();

        std::shared_ptr<ShaderModule> vertexShaderModule = ShaderModule::Create();
        vertexShaderModule->Init(std::make_pair("Shaders/Vulkan/Compiled/TriangleVS.spv", Pipeline::ShaderStage::VERTEX));

        std::shared_ptr<ShaderModule> fragmentShaderModule = ShaderModule::Create();
        fragmentShaderModule->Init(std::make_pair("Shaders/Vulkan/Compiled/TriangleFS.spv", Pipeline::ShaderStage::FRAGMENT));

        Pipeline::GraphicsPipelineSpecs pipelineSpecs{};
        RenderPass::RenderPassSpecs renderPassSpecs;
        renderPassSpecs.pAttachments =
        {
            {0, Core::Format::FORMAT_B8G8R8A8_UNORM, true, RenderPass::AttachmentType::Swapchain},
            {1, Core::Format::FORMAT_D32_SFLOAT, true, RenderPass::AttachmentType::Depth}
        };

        Pipeline::VertexDataLayout vertexLayout;
        vertexLayout.pBinding = 0;
        vertexLayout.pStride = sizeof(MeshData);
        vertexLayout.pAttributes = {
            {0, 0, Core::Format::FORMAT_R32G32B32_SFLOAT, offsetof(MeshData, MeshData::Position)},
            {0, 1, Core::Format::FORMAT_R32G32B32_SFLOAT, offsetof(MeshData, MeshData::Normal)},
            {0, 2, Core::Format::FORMAT_R32G32_SFLOAT,    offsetof(MeshData, MeshData::UV0)},
            {0, 3, Core::Format::FORMAT_R32G32_SFLOAT,    offsetof(MeshData, MeshData::UV1)},
            {0, 4, Core::Format::FORMAT_R32G32B32_SFLOAT, offsetof(MeshData, MeshData::Color)},
        };
        pipelineSpecs.pLayout = vertexLayout;
        pipelineSpecs.pRenderPass = RenderPass::Create();
        pipelineSpecs.pRenderPass->Init(renderPassSpecs);

        pipelineSpecs.pDescriptorLayouts["material"] = materialLayout;
        pipelineSpecs.pDescriptorLayouts["pbr"] = pbrLayout;

        pipelineSpecs.pShaderModules.push_back(vertexShaderModule);
        pipelineSpecs.pShaderModules.push_back(fragmentShaderModule);

        pipelineSpecs.pDepthClampEnable = false;
        pipelineSpecs.pRasterizationDiscardEnable = false;
        pipelineSpecs.pPolygoneMode = Pipeline::POLYGON_MODE_FILL;
        pipelineSpecs.pLineWidth = 1.0f;
        pipelineSpecs.pCullMode = Pipeline::CullMode::BACK;
        pipelineSpecs.pFrontFace = Pipeline::FrontFace::COUTNER_CLOCKWISE;
        pipelineSpecs.pDepthBiasEnable = false;
        pipelineSpecs.pDepthTestEnable = true;
        pipelineSpecs.pDepthWriteEnable = true;
        pipelineSpecs.pCompareOp = Pipeline::COMPARE_OP_LESS;
        pipelineSpecs.pDepthBoundsTestEnable = false;
        pipelineSpecs.pStencilTestEnable = false;

        pipeline = Pipeline::Create();
        pipeline->Init(pipelineSpecs);
	}

	void IBLTextureMaker::EquirectangularMapPath(std::string path) {
		m_EquirectangularMap = std::make_shared<Texture>();
		m_EquirectangularMap->Init(path);
	}
}