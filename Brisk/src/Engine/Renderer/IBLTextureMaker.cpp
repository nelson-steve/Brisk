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
        EnvTexture->BindAs();
	}

	void IBLTextureMaker::EquirectangularMapPath(std::string path) {
		m_EquirectangularMap = std::make_shared<Texture>();
		m_EquirectangularMap->Init(path);
	}
}