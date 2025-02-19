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
    void PipelineVulkan::Init(const Pipeline::GraphicsPipelineSpecs& specs) {
        m_GraphicsSpecs = specs;
        //std::vector<VkDescriptorSetLayout> descriptorLayouts;

        //for (const auto& pair : specs.pDescriptorLayouts) {
        //    descriptorLayouts.push_back(std::static_pointer_cast<DescriptorLayoutVulkan>(pair.second)->GetLayout());
        //}

        //VkPipelineVertexInputStateCreateInfo m_VertexInputInfo{};
        //m_VertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        //VkVertexInputBindingDescription bindingDescription{};
        //bindingDescription.binding = specs.pLayout.pBinding;
        //bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        //bindingDescription.stride = specs.pLayout.pStride;
        //std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
        //for (int i = 0; i < specs.pLayout.pAttributes.size(); i++) {
        //    VkVertexInputAttributeDescription attributeDescription;
        //    attributeDescription.binding = specs.pLayout.pAttributes[i].pBinding;
        //    attributeDescription.location = specs.pLayout.pAttributes[i].pLocation;
        //    attributeDescription.format = UtilitiesVulkan::FormatToVkFormat(specs.pLayout.pAttributes[i].pFormat);
        //    attributeDescription.offset = specs.pLayout.pAttributes[i].pOffset;
        //    attributeDescriptions.push_back(attributeDescription);
        //}
        //m_VertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        //m_VertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        //m_VertexInputInfo.vertexBindingDescriptionCount = 1;
        //m_VertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());

        //VkPipelineInputAssemblyStateCreateInfo m_InputAssembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
        //m_InputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        //m_InputAssembly.primitiveRestartEnable = VK_FALSE;

        //VkPipelineViewportStateCreateInfo m_ViewportState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
        //m_ViewportState.viewportCount = 1;
        //m_ViewportState.scissorCount = 1;

        //VkPipelineRasterizationStateCreateInfo m_Rasterizer{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
        //m_Rasterizer.depthClampEnable = specs.pDepthClampEnable;
        //m_Rasterizer.rasterizerDiscardEnable = specs.pRasterizationDiscardEnable;
        //m_Rasterizer.polygonMode = UtilitiesVulkan::PolygonToVulkanType(specs.pPolygoneMode);
        //m_Rasterizer.lineWidth = specs.pLineWidth;
        //m_Rasterizer.cullMode = VK_CULL_MODE_NONE;
        //m_Rasterizer.frontFace = UtilitiesVulkan::FrontFaceToVulkanType(specs.pFrontFace);
        //m_Rasterizer.depthBiasEnable = specs.pDepthBiasEnable;

        //VkPipelineMultisampleStateCreateInfo m_Multisampling{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
        //m_Multisampling.sampleShadingEnable = VK_FALSE;
        //m_Multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        //VkPipelineDepthStencilStateCreateInfo m_DepthStencil{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
        //m_DepthStencil.depthTestEnable = specs.pDepthTestEnable;
        //m_DepthStencil.depthWriteEnable = specs.pDepthWriteEnable;
        //m_DepthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        //m_DepthStencil.depthBoundsTestEnable = specs.pDepthBoundsTestEnable;
        //m_DepthStencil.stencilTestEnable = specs.pStencilTestEnable;
        //m_DepthStencil.back.compareOp = VK_COMPARE_OP_ALWAYS;

        //m_DepthStencil.depthTestEnable = VK_FALSE;
        //m_DepthStencil.depthWriteEnable = VK_FALSE;
        //m_DepthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        ////m_DepthStencil.front = depthStencilStateCI.back;
        //m_DepthStencil.back.compareOp = VK_COMPARE_OP_ALWAYS;

        //VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        //colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        //colorBlendAttachment.blendEnable = VK_FALSE;
        //VkPipelineColorBlendStateCreateInfo m_ColorBlending{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
        //m_ColorBlending.logicOpEnable = VK_FALSE;
        //m_ColorBlending.logicOp = VK_LOGIC_OP_COPY;
        //m_ColorBlending.attachmentCount = 1;
        //m_ColorBlending.pAttachments = &colorBlendAttachment;
        //m_ColorBlending.blendConstants[0] = 0.0f;
        //m_ColorBlending.blendConstants[1] = 0.0f;
        //m_ColorBlending.blendConstants[2] = 0.0f;
        //m_ColorBlending.blendConstants[3] = 0.0f;

        //std::vector<VkDynamicState> dynamicStates = {
        //    VK_DYNAMIC_STATE_VIEWPORT,
        //    VK_DYNAMIC_STATE_SCISSOR
        //};
        //VkPipelineDynamicStateCreateInfo m_DynamicState{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
        //m_DynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        //m_DynamicState.pDynamicStates = dynamicStates.data();

        //VkPipelineLayoutCreateInfo m_PipelineLayoutInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
        //m_PipelineLayoutInfo.setLayoutCount = 0;
        //m_PipelineLayoutInfo.pushConstantRangeCount = 0;
        //m_PipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorLayouts.size());
        //m_PipelineLayoutInfo.pSetLayouts = descriptorLayouts.data();

        //if (vkCreatePipelineLayout(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), &m_PipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
        //    throw std::runtime_error("failed to create pipeline layout!");
        //}

        //std::vector<VkPipelineShaderStageCreateInfo> shaderStages{};
        //for (int i = 0; i < specs.pShaderModules.size(); i++) {
        //    shaderStages.push_back(std::static_pointer_cast<ShaderModuleVulkan>(specs.pShaderModules[i])->GetShaderStageInfo());
        //}

        //VkGraphicsPipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
        //pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        //pipelineInfo.pStages = shaderStages.data();
        //pipelineInfo.pVertexInputState = &m_VertexInputInfo;
        //pipelineInfo.pInputAssemblyState = &m_InputAssembly;
        //pipelineInfo.pViewportState = &m_ViewportState;
        //pipelineInfo.pRasterizationState = &m_Rasterizer;
        //pipelineInfo.pMultisampleState = &m_Multisampling;
        //pipelineInfo.pColorBlendState = &m_ColorBlending;
        //pipelineInfo.pDepthStencilState = &m_DepthStencil;
        //pipelineInfo.pDynamicState = &m_DynamicState;
        //pipelineInfo.layout = m_PipelineLayout;
        //pipelineInfo.renderPass = std::static_pointer_cast<RenderPassVulkan>(specs.pRenderPass)->GetRenderPass();
        //pipelineInfo.subpass = 0;
        //pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        //if (vkCreateGraphicsPipelines(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline) != VK_SUCCESS) {
        //    throw std::runtime_error("failed to create graphics pipeline!");
        //}

        {
            //
            std::vector<VkDescriptorSetLayout> descriptorLayouts;
            for (const auto& pair : specs.pDescriptorLayouts) {
                descriptorLayouts.push_back(std::static_pointer_cast<DescriptorLayoutVulkan>(pair.second)->GetLayout());
                descriptorLayouts.push_back(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_BindlessDescriptorLayout);
            }

            VkPushConstantRange pushConstantRange = {};
            pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;  // Assuming you'll use this in the fragment shader
            pushConstantRange.offset = 0;  // Starting offset in push constant space
            pushConstantRange.size = sizeof(PushConstants);  // Size of the push constant data

            VkPipelineLayoutCreateInfo m_PipelineLayoutInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
            m_PipelineLayoutInfo.setLayoutCount = 0;
            m_PipelineLayoutInfo.pushConstantRangeCount = 1;
            m_PipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
            m_PipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorLayouts.size());
            m_PipelineLayoutInfo.pSetLayouts = descriptorLayouts.data();

            if (vkCreatePipelineLayout(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), &m_PipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create pipeline layout!");
            }

            //

            VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
            vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

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

            vertexInputInfo.vertexBindingDescriptionCount = 1;
            vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
            vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
            vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

            VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
            inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            inputAssembly.primitiveRestartEnable = VK_FALSE;

            VkPipelineViewportStateCreateInfo viewportState{};
            viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportState.viewportCount = 1;
            viewportState.scissorCount = 1;

            VkPipelineRasterizationStateCreateInfo rasterizer{};
            rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizer.depthClampEnable = VK_FALSE;
            rasterizer.rasterizerDiscardEnable = VK_FALSE;
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizer.lineWidth = 1.0f;
            rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
            rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
            rasterizer.depthBiasEnable = VK_FALSE;

            VkPipelineMultisampleStateCreateInfo multisampling{};
            multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampling.sampleShadingEnable = VK_FALSE;
            multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkPipelineDepthStencilStateCreateInfo depthStencil{};
            depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depthStencil.depthTestEnable = VK_TRUE;
            depthStencil.depthWriteEnable = VK_TRUE;
            depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
            depthStencil.depthBoundsTestEnable = VK_FALSE;
            depthStencil.stencilTestEnable = VK_FALSE;

            VkPipelineColorBlendAttachmentState colorBlendAttachment{};
            colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            colorBlendAttachment.blendEnable = VK_FALSE;

            VkPipelineColorBlendStateCreateInfo colorBlending{};
            colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
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
            VkPipelineDynamicStateCreateInfo dynamicState{};
            dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
            dynamicState.pDynamicStates = dynamicStates.data();

            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = 1;
            pipelineLayoutInfo.pSetLayouts = descriptorLayouts.data();

            if (vkCreatePipelineLayout(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), &m_PipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create pipeline layout!");
            }

            std::vector<VkPipelineShaderStageCreateInfo> shaderStages{};
            for (int i = 0; i < specs.pShaderModules.size(); i++) {
                shaderStages.push_back(std::static_pointer_cast<ShaderModuleVulkan>(specs.pShaderModules[i])->GetShaderStageInfo());
            }

            VkGraphicsPipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.stageCount = 2;
            pipelineInfo.pStages = shaderStages.data();
            pipelineInfo.pVertexInputState = &vertexInputInfo;
            pipelineInfo.pInputAssemblyState = &inputAssembly;
            pipelineInfo.pViewportState = &viewportState;
            pipelineInfo.pRasterizationState = &rasterizer;
            pipelineInfo.pMultisampleState = &multisampling;
            pipelineInfo.pDepthStencilState = &depthStencil;
            pipelineInfo.pColorBlendState = &colorBlending;
            pipelineInfo.pDynamicState = &dynamicState;
            pipelineInfo.layout = m_PipelineLayout;
            pipelineInfo.renderPass = std::static_pointer_cast<RenderPassVulkan>(specs.pRenderPass)->GetRenderPass();
            pipelineInfo.subpass = 0;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

            if (vkCreateGraphicsPipelines(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline) != VK_SUCCESS) {
                throw std::runtime_error("failed to create graphics pipeline!");
            }
        }
    }

    void PipelineVulkan::Bind(std::shared_ptr<CommandBuffer> cmd) {
        vkCmdBindPipeline(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
    }

    void PipelineVulkan::BindPushConstant(std::shared_ptr<CommandBuffer> cmd, uint32_t size, void* data) {
        vkCmdPushConstants(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), m_PipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, size, data);
    }

    void PipelineVulkan::Destroy() {
        //for (VkShaderModule module : m_Modules) {
        //    vkDestroyShaderModule(GpuContextVulkan::s_GPUDevice->GetDevice(), module, nullptr);
        //}
        //vkDestroyPipelineLayout(GpuContextVulkan::s_GPUDevice->GetDevice(), m_PipelineLayout, nullptr);
        //vkDestroyPipeline(GpuContextVulkan::s_GPUDevice->GetDevice(), m_GraphicsPipeline, nullptr);
    }
}