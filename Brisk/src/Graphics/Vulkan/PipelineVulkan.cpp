#include "PipelineVulkan.hpp"
#include "Core/Log.hpp"
#include "ShaderVulkan.hpp"
#include "Engine/Engine.hpp"
#include "SwapchainVulkan.hpp"
#include "UtilitiesVulkan.hpp"
#include "RenderPassVulkan.hpp"
#include "ShaderModuleVulkan.hpp"
#include "DescriptorLayoutVulkan.hpp"
#include "Engine/Renderer/Renderer.hpp"

namespace Brisk
{
    void PipelineVulkan::Init(const GraphicsPipelineSpecs& specs) {
        m_GraphicsSpecs = specs;

        std::vector<VkDescriptorSetLayout> descriptorLayouts;
        for (const auto& layout : specs.pDescriptorLayouts) {
            descriptorLayouts.push_back(std::static_pointer_cast<DescriptorLayoutVulkan>(layout)->GetLayout());
            //descriptorLayouts.push_back(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_BindlessDescriptorLayout);
        }

        VkPushConstantRange pushConstantRange = {};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PushConstants);

        VkPipelineLayoutCreateInfo m_PipelineLayoutInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
        m_PipelineLayoutInfo.pushConstantRangeCount = 0;
        m_PipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorLayouts.size());
        m_PipelineLayoutInfo.pSetLayouts = descriptorLayouts.data();

        if (vkCreatePipelineLayout(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), &m_PipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = specs.pLayout.pBinding;
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescription.stride = specs.pLayout.pStride;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
        for (int i = 0; i < specs.pLayout.pAttributes.size(); i++) {
            VkVertexInputAttributeDescription attributeDescription;
            attributeDescription.binding = specs.pLayout.pAttributes[i].pBinding;
            attributeDescription.location = specs.pLayout.pAttributes[i].pLocation;
            attributeDescription.format = UtilitiesVulkan::FormatToVkFormat(specs.pLayout.pAttributes[i].pFormat);
            attributeDescription.offset = specs.pLayout.pAttributes[i].pOffset;
            attributeDescriptions.push_back(attributeDescription);
        }
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
        rasterizer.depthClampEnable = specs.pDepthClampEnable;
        rasterizer.rasterizerDiscardEnable = specs.pRasterizationDiscardEnable;
        rasterizer.polygonMode = UtilitiesVulkan::PolygonToVulkanType(specs.pPolygoneMode);
        rasterizer.lineWidth = specs.pLineWidth;
        rasterizer.cullMode = (specs.pCullMode == CullMode::BACK) ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_FRONT_BIT;
        rasterizer.frontFace = UtilitiesVulkan::FrontFaceToVulkanType(specs.pFrontFace);
        rasterizer.depthBiasEnable = specs.pDepthBiasEnable;

        VkPipelineMultisampleStateCreateInfo multisampling{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineDepthStencilStateCreateInfo depthStencil{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
        depthStencil.depthTestEnable = specs.pDepthTestEnable;
        depthStencil.depthWriteEnable = specs.pDepthWriteEnable;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = specs.pDepthBoundsTestEnable;
        depthStencil.stencilTestEnable = specs.pStencilTestEnable;
        depthStencil.back.compareOp = VK_COMPARE_OP_ALWAYS;
        depthStencil.front = depthStencil.back;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        VkPipelineColorBlendStateCreateInfo colorBlending{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages{};
        for (int i = 0; i < specs.pShaderModules.size(); i++) {
            shaderStages.push_back(std::static_pointer_cast<ShaderModuleVulkan>(specs.pShaderModules[i])->GetShaderStageInfo());
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
        pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = m_PipelineLayout;
        pipelineInfo.renderPass = std::static_pointer_cast<RenderPassVulkan>(specs.pRenderPass)->GetRenderPass();
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }
    }

    void PipelineVulkan::Init(const ComputePipelineSpecs& specs) {
        VkPipelineLayoutCreateInfo computePipelineLayoutCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
        computePipelineLayoutCreateInfo.pNext = nullptr;
        //computePipelineLayoutCreateInfo.flags;
        computePipelineLayoutCreateInfo.setLayoutCount = 1;
        computePipelineLayoutCreateInfo.pSetLayouts;
        //computePipelineLayoutCreateInfo.pushConstantRangeCount;
        //computePipelineLayoutCreateInfo.pPushConstantRanges;

        if (vkCreatePipelineLayout(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &computePipelineLayoutCreateInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        std::vector<VkDescriptorSetLayout> descriptorLayouts;
        for (const auto& layout : specs.pDescriptorLayouts) {
            descriptorLayouts.push_back(std::static_pointer_cast<DescriptorLayoutVulkan>(layout)->GetLayout());
            descriptorLayouts.push_back(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_BindlessDescriptorLayout);
        }

        VkPipelineShaderStageCreateInfo shaderStage{};
        shaderStage = std::static_pointer_cast<ShaderModuleVulkan>(specs.pShaderModule)->GetShaderStageInfo();
        VkComputePipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
        pipelineInfo.pNext = nullptr;
        //pipelineInfo.flags;
        pipelineInfo.stage = shaderStage;
        pipelineInfo.layout = m_PipelineLayout;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        //pipelineInfo.basePipelineIndex;
    }

    void PipelineVulkan::Bind(std::shared_ptr<CommandBuffer> cmd) {
        vkCmdBindPipeline(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
    }

    void PipelineVulkan::BindPushConstant(std::shared_ptr<CommandBuffer> cmd, uint32_t size, void* data) {
        vkCmdPushConstants(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), m_PipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, size, data);
    }

    void PipelineVulkan::Destroy() {
        for (VkShaderModule module : m_Modules) {
            vkDestroyShaderModule(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), module, nullptr);
        }
        vkDestroyPipelineLayout(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), m_PipelineLayout, nullptr);
        vkDestroyPipeline(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), m_Pipeline, nullptr);
    }
}