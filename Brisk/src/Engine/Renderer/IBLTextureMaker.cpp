#pragma once

#include "IBLTextureMaker.hpp"
#include "Engine/Model.hpp"
#include "Shader.hpp"
#include "ComputeCommand.hpp"

namespace Brisk
{
	void IBLTextureMaker::Init() {
		m_ComputePipeline = std::make_shared<Pipeline>();

        Texture::TextureSpecification cubemapSpecs;
        cubemapSpecs.pWidth = 1024;
        cubemapSpecs.pHeight = 1024;
        cubemapSpecs.pDepth = 1;
        cubemapSpecs.pArrayLayers = 6;
        std::shared_ptr<Texture> cubemap;
        cubemap->Init(cubemapSpecs);
        std::shared_ptr<Texture> hdr;
        hdr->Init("path to hdr skybox");

        int mapLevel = 1;
		std::shared_ptr<DescriptorLayout> descriptorLayout = DescriptorLayout::Create();
        descriptorLayout->AddBinding(0, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { GPUResource::ShaderStageAccess::SHADER_STAGE_COMPUTE_BIT });
        descriptorLayout->AddBinding(1, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_STORAGE_IMAGE, { GPUResource::ShaderStageAccess::SHADER_STAGE_COMPUTE_BIT });
        descriptorLayout->AddBinding(2, mapLevel - 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_STORAGE_IMAGE, { GPUResource::ShaderStageAccess::SHADER_STAGE_COMPUTE_BIT });
        descriptorLayout->Init();

		std::shared_ptr<ShaderModule> computeShaderModule = ShaderModule::Create();
        computeShaderModule->Init(std::make_pair("Shaders/Vulkan/Compiled/EquirectangularToCubemap.spv", Pipeline::ShaderStage::COMPUTE));

        Pipeline::ComputePipelineSpecs pipelineSpecs{};
        pipelineSpecs.pDescriptorLayouts.push_back(descriptorLayout);
        pipelineSpecs.pShaderModule = computeShaderModule;

        m_ComputePipeline = Pipeline::Create();
        m_ComputePipeline->Init(pipelineSpecs);

        {
            std::shared_ptr<CommandBuffer> cmd;
            cmd->Bind();

            std::shared_ptr<Shader> m_ComputeShader = std::make_shared<Shader>();
            m_ComputeShader->Init(m_ComputePipeline);

            m_ComputeShader->BindTexture(hdr, 0, 0);
            m_ComputeShader->BindTexture(cubemap, 1, 0);

            cubemap->TransitionImageLayout(cmd, );

            m_ComputePipeline->Bind();
            m_ComputeShader->Bind(cmd, m_ComputePipeline);
            uint32_t texSize = 1024;
            ComputeCommand::CmdDispatch(cmd, texSize / 32, texSize / 32, 6);

            cubemap->TransitionImageLayout(cmd, );

            cmd->UnBind();
            m_ComputePipeline->Destroy();

        }

        Texture::TextureSpecification mainEnvTextureSpecs;
        mainEnvTextureSpecs.pWidth = 1024;
        mainEnvTextureSpecs.pHeight = 1024;
        mainEnvTextureSpecs.pDepth = 1;
        mainEnvTextureSpecs.pArrayLayers = 6;
        std::shared_ptr<Texture> mainEnvTexture;
        mainEnvTexture->Init(mainEnvTextureSpecs);

        std::shared_ptr<CommandBuffer> cmd;
        cmd->Bind();

        cubemap->TransitionImageLayout(cmd);
        mainEnvTexture->TransitionImageLayout(cmd);

        Texture::CopyImage(cmd, cubemap, mainEnvTexture, 1024);

        cubemap->TransitionImageLayout();
        mainEnvTexture->TransitionImageLayout();

        cmd->Bind();

        //
        {
            //std::shared_ptr<Texture> _Cubemap;

            //std::shared_ptr<DescriptorLayout> materialLayout = DescriptorLayout::Create();
            //materialLayout->pName = "material";
            //materialLayout->AddBindingLayout(0, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_UNIFORM_BUFFER, { GPUResource::ShaderStageAccess::SHADER_STAGE_VERTEX_BIT });
            //materialLayout->Init();

            //std::shared_ptr<DescriptorLayout> pbrLayout = DescriptorLayout::Create();
            //pbrLayout->pName = "pbr";
            //pbrLayout->AddBindingLayout(0, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
            //pbrLayout->AddBindingLayout(1, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
            //pbrLayout->AddBindingLayout(2, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
            //pbrLayout->Init();

            //std::shared_ptr<ShaderModule> vertexShaderModule = ShaderModule::Create();
            //vertexShaderModule->Init(std::make_pair("Shaders/Vulkan/Compiled/TriangleVS.spv", Pipeline::ShaderStage::VERTEX));

            //std::shared_ptr<ShaderModule> fragmentShaderModule = ShaderModule::Create();
            //fragmentShaderModule->Init(std::make_pair("Shaders/Vulkan/Compiled/TriangleFS.spv", Pipeline::ShaderStage::FRAGMENT));

            //Pipeline::GraphicsPipelineSpecs pipelineSpecs{};
            //RenderPass::RenderPassSpecs renderPassSpecs;
            //renderPassSpecs.pAttachments =
            //{
            //    {0, Core::Format::FORMAT_B8G8R8A8_UNORM, true, RenderPass::AttachmentType::Swapchain},
            //    {1, Core::Format::FORMAT_D32_SFLOAT, true, RenderPass::AttachmentType::Depth}
            //};

            //Pipeline::VertexDataLayout vertexLayout;
            //vertexLayout.pBinding = 0;
            //vertexLayout.pStride = sizeof(MeshData);
            //vertexLayout.pAttributes = {
            //    {0, 0, Core::Format::FORMAT_R32G32B32_SFLOAT, offsetof(MeshData, MeshData::Position)},
            //};
            //pipelineSpecs.pLayout = vertexLayout;
            //pipelineSpecs.pRenderPass = RenderPass::Create();
            //pipelineSpecs.pRenderPass->Init(renderPassSpecs);

            //pipelineSpecs.pDescriptorLayouts["material"] = materialLayout;
            //pipelineSpecs.pDescriptorLayouts["pbr"] = pbrLayout;

            //pipelineSpecs.pShaderModules.push_back(vertexShaderModule);
            //pipelineSpecs.pShaderModules.push_back(fragmentShaderModule);

            //pipelineSpecs.pDepthClampEnable = false;
            //pipelineSpecs.pRasterizationDiscardEnable = false;
            //pipelineSpecs.pPolygoneMode = Pipeline::POLYGON_MODE_FILL;
            //pipelineSpecs.pLineWidth = 1.0f;
            //pipelineSpecs.pCullMode = Pipeline::CullMode::BACK;
            //pipelineSpecs.pFrontFace = Pipeline::FrontFace::COUTNER_CLOCKWISE;
            //pipelineSpecs.pDepthBiasEnable = false;
            //pipelineSpecs.pDepthTestEnable = true;
            //pipelineSpecs.pDepthWriteEnable = true;
            //pipelineSpecs.pCompareOp = Pipeline::COMPARE_OP_LESS;
            //pipelineSpecs.pDepthBoundsTestEnable = false;
            //pipelineSpecs.pStencilTestEnable = false;

            //std::shared_ptr<Pipeline> pipeline = Pipeline::Create();
            //pipeline->Init(pipelineSpecs);
        }
	}

	void IBLTextureMaker::EquirectangularMapPath(std::string path) {
		m_EquirectangularMap = std::make_shared<Texture>();
		m_EquirectangularMap->Init(path);
	}
}