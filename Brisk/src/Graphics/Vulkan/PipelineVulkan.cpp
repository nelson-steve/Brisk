#include "PipelineVulkan.hpp"
#include "Core/Log.hpp"
#include "Engine/Engine.hpp"
#include "SwapchainVulkan.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "UtilitiesVulkan.hpp"

namespace Brisk
{
    void PipelineVulkan::Init(const Pipeline::PipelineSpecs& specs) {
        VkPipelineVertexInputStateCreateInfo m_VertexInputInfo{};
        m_VertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = specs.Layout.pBinding;
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescription.stride = specs.Layout.pStride;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
        for (int i = 0; i < specs.Layout.pAttributes.size(); i++) {
            VkVertexInputAttributeDescription attributeDescription;
            attributeDescription.binding = specs.Layout.pAttributes[i].pBinding;
            attributeDescription.location = specs.Layout.pAttributes[i].pLocation;
            attributeDescription.format = UtilitiesVulkan::FormatToVulkanType(specs.Layout.pAttributes[i].pFormat);
            attributeDescription.offset = specs.Layout.pAttributes[i].pOffset;
            attributeDescriptions.push_back(attributeDescription);
        }
        m_VertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        m_VertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        m_VertexInputInfo.vertexBindingDescriptionCount = 1;
        m_VertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());

        VkPipelineInputAssemblyStateCreateInfo m_InputAssembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
        m_InputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        m_InputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo m_ViewportState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
        m_ViewportState.viewportCount = 1;
        m_ViewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo m_Rasterizer{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
        m_Rasterizer.depthClampEnable = specs.pDepthClampEnable;
        m_Rasterizer.rasterizerDiscardEnable = specs.pRasterizationDiscardEnable;
        m_Rasterizer.polygonMode = UtilitiesVulkan::PolygonToVulkanType(specs.pPolygoneMode);
        m_Rasterizer.lineWidth = specs.pLineWidth;
        m_Rasterizer.cullMode = specs.pCullMode;
        m_Rasterizer.frontFace = UtilitiesVulkan::FrontFaceToVulkanType(specs.pFrontFace);
        m_Rasterizer.depthBiasEnable = specs.pDepthBiasEnable;

        VkPipelineMultisampleStateCreateInfo m_Multisampling{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
        m_Multisampling.sampleShadingEnable = VK_FALSE;
        m_Multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineDepthStencilStateCreateInfo m_DepthStencil{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
        m_DepthStencil.depthTestEnable = specs.pDepthTestEnable;
        m_DepthStencil.depthWriteEnable = specs.pDepthWriteEnable;
        m_DepthStencil.depthCompareOp = UtilitiesVulkan::CompareOpToVulkanType(specs.pCompareOp);
        m_DepthStencil.depthBoundsTestEnable = specs.pDepthBoundsTestEnable;
        m_DepthStencil.stencilTestEnable = specs.pStencilTestEnable;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        VkPipelineColorBlendStateCreateInfo m_ColorBlending{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
        m_ColorBlending.logicOpEnable = VK_FALSE;
        m_ColorBlending.logicOp = VK_LOGIC_OP_COPY;
        m_ColorBlending.attachmentCount = 1;
        m_ColorBlending.pAttachments = &colorBlendAttachment;
        m_ColorBlending.blendConstants[0] = 0.0f;
        m_ColorBlending.blendConstants[1] = 0.0f;
        m_ColorBlending.blendConstants[2] = 0.0f;
        m_ColorBlending.blendConstants[3] = 0.0f;

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo m_DynamicState{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
        m_DynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        m_DynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineLayoutCreateInfo m_PipelineLayoutInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
        m_PipelineLayoutInfo.setLayoutCount = 0;
        m_PipelineLayoutInfo.pushConstantRangeCount = 0;

        if (vkCreatePipelineLayout(Engine::s_GPUDevice->GetDevice(), &m_PipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkPipelineShaderStageCreateInfo shaderStageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
        shaderStageInfo.stage = stage;
        shaderStageInfo.module = module;
        shaderStageInfo.pName = "main";

        m_Modules.push_back(module);
        m_ShaderStages.push_back(shaderStageInfo);

        VkGraphicsPipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
        pipelineInfo.stageCount = static_cast<uint32_t>(m_ShaderStages.size());
        pipelineInfo.pStages = m_ShaderStages.data();
        pipelineInfo.pVertexInputState = &m_VertexInputInfo;
        pipelineInfo.pInputAssemblyState = &m_InputAssembly;
        pipelineInfo.pViewportState = &m_ViewportState;
        pipelineInfo.pRasterizationState = &m_Rasterizer;
        pipelineInfo.pMultisampleState = &m_Multisampling;
        pipelineInfo.pColorBlendState = &m_ColorBlending;
        pipelineInfo.pDepthStencilState = &m_DepthStencil;
        pipelineInfo.pDynamicState = &m_DynamicState;
        pipelineInfo.layout = m_PipelineLayout;
        pipelineInfo.renderPass = renderpass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(GpuContextVulkan::s_GPUDevice->GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }
    }

    void GraphicsPipelineVulkan::Destroy() {
        for (VkShaderModule module : m_Modules) {
            vkDestroyShaderModule(GpuContextVulkan::s_GPUDevice->GetDevice(), module, nullptr);
        }
        vkDestroyPipelineLayout(GpuContextVulkan::s_GPUDevice->GetDevice(), m_PipelineLayout, nullptr);
        vkDestroyPipeline(GpuContextVulkan::s_GPUDevice->GetDevice(), m_GraphicsPipeline, nullptr);
    }
}