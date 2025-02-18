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

        Pipeline::PipelineSpecs pipelineSpecs{};
		pipelineSpecs.pDescriptorLayouts["textures"] = descriptorLayout;

        pipelineSpecs.pShaderModules.push_back(computeShaderModule);

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

        m_ComputePipeline = Pipeline::Create();
        m_ComputePipeline->Init(pipelineSpecs);
	}

	void IBLTextureMaker::EquirectangularMapPath(std::string path) {
		m_EquirectangularMap = std::make_shared<Texture>();
		m_EquirectangularMap->Init(path);
	}
}