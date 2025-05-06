#pragma once

#include "IBLTextureMaker.hpp"
#include "Engine/Model.hpp"
#include "Shader.hpp"
#include "ComputeCommand.hpp"

namespace Brisk
{
	void IBLTextureMaker::Init() {
        m_ComputePipeline = Pipeline::Create();

        Texture::TextureSpecification cubemapSpecs;
        cubemapSpecs.p_Width = 1024;
        cubemapSpecs.p_Height = 1024;
        cubemapSpecs.p_Depth = 1;
        cubemapSpecs.p_ArrayLayers = 6;
        std::shared_ptr<Texture> cubemap = Texture::Create();
        cubemap->Init(cubemapSpecs);
        std::shared_ptr<Texture> hdr = Texture::Create();
        hdr->Init("path to hdr skybox");

        int mapLevel = 1;
		std::shared_ptr<DescriptorLayout> descriptorLayout = DescriptorLayout::Create();
        descriptorLayout->AddBinding(0, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { GPUResource::ShaderStageAccess::SHADER_STAGE_COMPUTE_BIT });
        descriptorLayout->AddBinding(1, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_STORAGE_IMAGE, { GPUResource::ShaderStageAccess::SHADER_STAGE_COMPUTE_BIT });
        descriptorLayout->AddBinding(2, mapLevel - 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_STORAGE_IMAGE, { GPUResource::ShaderStageAccess::SHADER_STAGE_COMPUTE_BIT });
        descriptorLayout->Init();

		std::shared_ptr<ShaderModule> computeShaderModule = ShaderModule::Create();
        computeShaderModule->Init("Shaders/Vulkan/Compiled/EquirectangularToCubemap.spv", Pipeline::ShaderStage::COMPUTE);

        Pipeline::ComputePipelineSpecs pipelineSpecs{};
        pipelineSpecs.pDescriptorLayouts.push_back(descriptorLayout);
        pipelineSpecs.pShaderModule = computeShaderModule;

        m_ComputePipeline = Pipeline::Create();
        m_ComputePipeline->Init(pipelineSpecs);

        {
            std::shared_ptr<CommandBuffer> cmd = CommandBuffer::Create();
            cmd->Bind();

            //std::shared_ptr<Shader> m_ComputeShader = Shader::Create();
            //std::shared_ptr<Shader> m_ComputeShader = std::make_shared<Shader>();
            Texture::ImageBarrierParams preComputeBarrier = {
                cubemap,
                Texture::ImageLayout::Undefined,
                Texture::ImageLayout::General,
                Texture::AccessType::None,
                Texture::AccessType::ShaderWrite,
                Texture::PipelineStage::TopOfPipe,
                Texture::PipelineStage::ComputeShader,
                Texture::ImageAspectFlags::Color,
                0,                         // Starting mip level
                1,                         // Only one mip level
                0,                         // All layers
                0// All layers
            };
            cubemap->TransitionImageLayout(cmd, { preComputeBarrier });

            m_ComputePipeline->Bind(cmd);
            //m_ComputeShader->Bind(cmd, m_ComputePipeline);
            uint32_t texSize = 1024;
            ComputeCommand::CmdDispatch(cmd, texSize / 32, texSize / 32, 6);

            Texture::ImageBarrierParams postComputeBarrier = {
                cubemap,
                Texture::ImageLayout::General,
                Texture::ImageLayout::ShaderReadOnlyOptimal,
                Texture::AccessType::ShaderWrite,
                Texture::AccessType::None,
                Texture::PipelineStage::ComputeShader,
                Texture::PipelineStage::BottomOfPipe,
                Texture::ImageAspectFlags::Color,
                0,                         // Starting mip level
                1,                         // Only one mip level
                0,                         // All layers
                0// All layers
            };
            cubemap->TransitionImageLayout(cmd, { postComputeBarrier });

            cmd->UnBind();
            m_ComputePipeline->Destroy();

        }

        {
            Texture::TextureSpecification mainEnvTextureSpecs;
            mainEnvTextureSpecs.p_Width = 1024;
            mainEnvTextureSpecs.p_Height = 1024;
            mainEnvTextureSpecs.p_Depth = 1;
            mainEnvTextureSpecs.p_ArrayLayers = 6;
            std::shared_ptr<Texture> mainEnvTexture;
            mainEnvTexture->Init(mainEnvTextureSpecs);

            std::shared_ptr<CommandBuffer> cmd;
            cmd->Bind();

            Texture::ImageBarrierParams preCopyBarrierCubemap = {
                cubemap,
                Texture::ImageLayout::ShaderReadOnlyOptimal,
                Texture::ImageLayout::TransferSrc,
                Texture::AccessType::None,
                Texture::AccessType::TransferRead,
                Texture::PipelineStage::TopOfPipe,
                Texture::PipelineStage::TransferStage,
                Texture::ImageAspectFlags::Color,
                0,                         // Starting mip level
                1,                         // Only one mip level
                0,                         // All layers
                0// All layers
            };

            Texture::ImageBarrierParams preCopyBarrierEnvTexture = {
                mainEnvTexture,
                Texture::ImageLayout::Undefined,
                Texture::ImageLayout::TransferDst,
                Texture::AccessType::None,
                Texture::AccessType::TransferRead,
                Texture::PipelineStage::TopOfPipe,
                Texture::PipelineStage::TransferStage,
                Texture::ImageAspectFlags::Color,
                0,                         // Starting mip level
                1,                         // Only one mip level
                0,                         // All layers
                0// All layers
            };
            cubemap->TransitionImageLayout(cmd, { preCopyBarrierCubemap, preCopyBarrierEnvTexture });

            cubemap->CopyImage(cmd, cubemap, mainEnvTexture, 1024, 1024);

            Texture::ImageBarrierParams postCopyBarrierCubemap = {
                cubemap,
                Texture::ImageLayout::TransferSrc,
                Texture::ImageLayout::ShaderReadOnlyOptimal,
                Texture::AccessType::TransferRead,
                Texture::AccessType::None,
                Texture::PipelineStage::TransferStage,
                Texture::PipelineStage::BottomOfPipe,
                Texture::ImageAspectFlags::Color,
                0,                         // Starting mip level
                1,                         // Only one mip level
                0,                         // All layers
                0// All layers
            };

            Texture::ImageBarrierParams postCopyBarrierEnvTexture = {
                mainEnvTexture,
                Texture::ImageLayout::TransferDst,
                Texture::ImageLayout::ShaderReadOnlyOptimal,
                Texture::AccessType::TransferWrite,
                Texture::AccessType::None,
                Texture::PipelineStage::TransferStage,
                Texture::PipelineStage::BottomOfPipe,
                Texture::ImageAspectFlags::Color,
                0,                         // Starting mip level
                1,                         // Only one mip level
                0,                         // All layers
                0// All layers
            };
            cubemap->TransitionImageLayout(cmd, { postCopyBarrierCubemap, postCopyBarrierEnvTexture });

            cmd->UnBind();
        }
        //  
        {
            std::shared_ptr<Texture> _Cubemap;

            std::shared_ptr<DescriptorLayout> materialLayout = DescriptorLayout::Create();
            materialLayout->AddBinding(0, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_UNIFORM_BUFFER, { GPUResource::ShaderStageAccess::SHADER_STAGE_VERTEX_BIT });
            materialLayout->Init();

            std::shared_ptr<DescriptorLayout> pbrLayout = DescriptorLayout::Create();
            pbrLayout->AddBinding(0, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
            pbrLayout->Init();

            std::shared_ptr<ShaderModule> vertexShaderModule = ShaderModule::Create();
            vertexShaderModule->Init("Shaders/Vulkan/Compiled/TriangleVS.spv", Pipeline::ShaderStage::VERTEX);

            std::shared_ptr<ShaderModule> fragmentShaderModule = ShaderModule::Create();
            fragmentShaderModule->Init("Shaders/Vulkan/Compiled/TriangleFS.spv", Pipeline::ShaderStage::FRAGMENT);

            Pipeline::GraphicsPipelineSpecs pipelineSpecs{};
            //RenderPass::RenderPassSpecs renderPassSpecs;
            //renderPassSpecs.pAttachments =
            //{
            //    {0, Core::Format::FORMAT_B8G8R8A8_UNORM, true, RenderPass::AttachmentType::Swapchain},
            //    {1, Core::Format::FORMAT_D32_SFLOAT, true, RenderPass::AttachmentType::Depth}
            //};

            Pipeline::VertexDataLayout vertexLayout;
            vertexLayout.pBinding = 0;
            vertexLayout.pStride = sizeof(MeshData);
            vertexLayout.pAttributes = {
                {0, 0, Core::Format::FORMAT_R32G32B32_SFLOAT, offsetof(MeshData, MeshData::Position)},
            };
            pipelineSpecs.pLayout = vertexLayout;
            //pipelineSpecs.pRenderPass = RenderPass::Create();
            //pipelineSpecs.pRenderPass->Init(renderPassSpecs);

            pipelineSpecs.pDescriptorLayouts.push_back(materialLayout);

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

            std::shared_ptr<Pipeline> pipeline = Pipeline::Create();
            pipeline->Init(pipelineSpecs);
        }
	}

	void IBLTextureMaker::EquirectangularMapPath(std::string path) {
		m_EquirectangularMap = Texture::Create();
		m_EquirectangularMap->Init(path);
	}
}