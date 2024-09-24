#include "GraphicsPipelineVulkan.hpp"
#include "Core/Log.hpp"
#include "Engine/Engine.hpp"
#include "SwapchainVulkan.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "GpuContextVulkan.hpp"

namespace Brisk 
{
    GraphicsPipelineVulkan::GraphicsPipelineVulkan() {
        VkPipelineVertexInputStateCreateInfo m_VertexInputInfo{};
        m_VertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescription.stride = sizeof(Point);
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
        attributeDescriptions.resize(2);
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Point, Point::Position);
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Point, Point::Color);
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
        m_Rasterizer.depthClampEnable = VK_FALSE;
        m_Rasterizer.rasterizerDiscardEnable = VK_FALSE;
        m_Rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        m_Rasterizer.lineWidth = 1.0f;
        m_Rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        m_Rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        m_Rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo m_Multisampling{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
        m_Multisampling.sampleShadingEnable = VK_FALSE;
        m_Multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineDepthStencilStateCreateInfo m_DepthStencil{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
        m_DepthStencil.depthTestEnable = VK_TRUE;
        m_DepthStencil.depthWriteEnable = VK_TRUE;
        m_DepthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        m_DepthStencil.depthBoundsTestEnable = VK_FALSE;
        m_DepthStencil.stencilTestEnable = VK_FALSE;

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

        if (vkCreatePipelineLayout(GpuContextVulkan::s_GPUDevice->GetDevice(), &m_PipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void GraphicsPipelineVulkan::CreateVertexInputState(const std::vector<VkVertexInputBindingDescription>& bindingDescriptions, const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions) {
        m_VertexInputInfo.sType = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
        m_VertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        m_VertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
        m_VertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
        m_VertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    }

    void GraphicsPipelineVulkan::CreateInputAssembly(bool primitiveRestartEnable, VkPrimitiveTopology topology) {
        m_InputAssembly.sType = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
        m_InputAssembly.topology = topology;
        m_InputAssembly.primitiveRestartEnable = primitiveRestartEnable ? VK_TRUE : VK_FALSE;
    }

    void GraphicsPipelineVulkan::CreateViewportState(uint32_t viewportCount, uint32_t scissorCount) {
        m_ViewportState.sType = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
        m_ViewportState.viewportCount = viewportCount;
        m_ViewportState.scissorCount = scissorCount;
    }

    void GraphicsPipelineVulkan::CreateRasterizer(bool depthClamp, bool discard, VkPolygonMode polygonMode, float lineWidth, VkCullModeFlags cullMode, VkFrontFace frontFace, bool depthBias) {
        m_Rasterizer.sType = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
        m_Rasterizer.depthClampEnable = depthClamp ? VK_TRUE : VK_FALSE;
        m_Rasterizer.rasterizerDiscardEnable = discard ? VK_TRUE : VK_FALSE;
        m_Rasterizer.polygonMode = polygonMode;
        m_Rasterizer.lineWidth = lineWidth;
        m_Rasterizer.cullMode = cullMode;
        m_Rasterizer.frontFace = frontFace;
        m_Rasterizer.depthBiasEnable = depthBias ? VK_TRUE : VK_FALSE;
    }

    void GraphicsPipelineVulkan::CreateMultiSampling(bool sampleShading, VkSampleCountFlagBits samples) {
        m_Multisampling.sType = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
        m_Multisampling.sampleShadingEnable = sampleShading ? VK_TRUE : VK_FALSE;
        m_Multisampling.rasterizationSamples = samples;
    }

    void GraphicsPipelineVulkan::CreateDepthStencil(bool depthTest, bool depthWrite, VkCompareOp compareOp, bool depthBoundsTest, bool stencilTest) {
        m_DepthStencil.sType = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
        m_DepthStencil.depthTestEnable = depthTest ? VK_TRUE : VK_FALSE;
        m_DepthStencil.depthWriteEnable = depthTest ? VK_TRUE : VK_FALSE;
        m_DepthStencil.depthCompareOp = compareOp;
        m_DepthStencil.depthBoundsTestEnable = depthBoundsTest ? VK_TRUE : VK_FALSE;
        m_DepthStencil.stencilTestEnable = stencilTest ? VK_TRUE : VK_FALSE;
    }

    void GraphicsPipelineVulkan::CrateColorBlending(const std::vector<VkPipelineColorBlendAttachmentState>& colorBlendAttachments, bool isLogicOp, VkLogicOp logicOp) {
        m_ColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        m_ColorBlending.logicOpEnable = isLogicOp ? VK_TRUE : VK_FALSE;
        m_ColorBlending.logicOp = logicOp;
        m_ColorBlending.attachmentCount = colorBlendAttachments.size();
        m_ColorBlending.pAttachments = colorBlendAttachments.data();
        m_ColorBlending.blendConstants[0] = 0.0f;
        m_ColorBlending.blendConstants[1] = 0.0f;
        m_ColorBlending.blendConstants[2] = 0.0f;
        m_ColorBlending.blendConstants[3] = 0.0f;
    }

    void GraphicsPipelineVulkan::CreateDynamicState(const std::vector<VkDynamicState>& dynamicStates) {
        m_DynamicState.sType = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
        m_DynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        m_DynamicState.pDynamicStates = dynamicStates.data();
    }

    void GraphicsPipelineVulkan::CreatePipelineLayout(std::vector< VkDescriptorSetLayout>& layouts, uint32_t pushConstantRangeCount) {
        m_PipelineLayoutInfo.sType = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
        m_PipelineLayoutInfo.pushConstantRangeCount = pushConstantRangeCount;
        m_PipelineLayoutInfo.pPushConstantRanges = nullptr;
        m_PipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
        m_PipelineLayoutInfo.pSetLayouts = layouts.data();

        if (vkCreatePipelineLayout(GpuContextVulkan::s_GPUDevice->GetDevice(), &m_PipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void GraphicsPipelineVulkan::CreateShaderStage(VkShaderModule module, VkShaderStageFlagBits stage) {
        VkPipelineShaderStageCreateInfo shaderStageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
        shaderStageInfo.stage = stage;
        shaderStageInfo.module = module;
        shaderStageInfo.pName = "main";

        m_Modules.push_back(module);
        m_ShaderStages.push_back(shaderStageInfo);
    }

	void GraphicsPipelineVulkan::CreatePipeline(VkRenderPass renderpass) {
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

    void GraphicsPipelineVulkan::Release() {
        for (VkShaderModule module : m_Modules) {
            vkDestroyShaderModule(GpuContextVulkan::s_GPUDevice->GetDevice(), module, nullptr);
        }
        vkDestroyPipelineLayout(GpuContextVulkan::s_GPUDevice->GetDevice(), m_PipelineLayout, nullptr);
        vkDestroyPipeline(GpuContextVulkan::s_GPUDevice->GetDevice(), m_GraphicsPipeline, nullptr);
    }
}