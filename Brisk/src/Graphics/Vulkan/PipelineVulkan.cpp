#include "PipelineVulkan.hpp"
#include "Core/Log.hpp"
#include "Engine/Engine.hpp"
#include "SwapchainVulkan.hpp"
#include "UtilitiesVulkan.hpp"
#include "RenderPassVulkan.hpp"
#include "DescriptorLayoutVulkan.hpp"
#include "Engine/Renderer/Renderer.hpp"

#include <spirv_reflect.h>

namespace Brisk
{
    void PipelineVulkan::Init(const GraphicsPipelineSpecs& specs) {
        m_GraphicsSpecs = specs;

        m_DescriptorSetLayouts.resize(5);
        m_DescriptorSetLayouts[0] = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_DummyDescriptorLayout;
        m_DescriptorSetLayouts[1] = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_DummyDescriptorLayout;
        m_DescriptorSetLayouts[2] = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_DummyDescriptorLayout;
        m_DescriptorSetLayouts[3] = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_DummyDescriptorLayout;
        m_DescriptorSetLayouts[4] = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_DummyDescriptorLayout;
        std::vector<VkPushConstantRange> pushConstants;
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        for (const std::string& path : specs.pShaderPaths) 
        {
            VkPipelineShaderStageCreateInfo shaderStage{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
            const std::vector<char>* shaderCode = UtilitiesVulkan::ReadShaderFile(path);

            VkShaderModuleCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = shaderCode->size();
            createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode->data());

            VkShaderModule shaderModule;
            if (vkCreateShaderModule(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
                throw std::runtime_error("failed to create shader module!");
            }
            shaderStage.module = shaderModule;
            shaderStage.pName = "main";

            m_Modules.push_back(shaderModule);

            SpvReflectShaderModule module;
            SpvReflectResult result = spvReflectCreateShaderModule(shaderCode->size(), shaderCode->data(), &module);

            // Dont need it anymore
            delete shaderCode;

            shaderStage.pName = module.entry_point_name;
            shaderStage.stage = static_cast<VkShaderStageFlagBits>(module.shader_stage);

            shaderStages.push_back(shaderStage);

            uint32_t setCount = 0;
            result = spvReflectEnumerateDescriptorSets(&module, &setCount, nullptr);
            std::vector<SpvReflectDescriptorSet*> sets(setCount);
            result = spvReflectEnumerateDescriptorSets(&module, &setCount, sets.data());

            for (auto set : sets) {
                uint32_t setIndex = 0;
                bool isBindless = false;
                std::vector<VkDescriptorSetLayoutBinding> bindings;
                for (uint32_t i = 0; i < set->binding_count; ++i) {
                    const SpvReflectDescriptorBinding* reflBinding = set->bindings[i];
                    setIndex = reflBinding->set;
                    if (reflBinding->count <= 0) {
                        isBindless = true;
                        break;
                    }

                    VkDescriptorSetLayoutBinding binding{};
                    binding.binding = reflBinding->binding;
                    binding.descriptorType = static_cast<VkDescriptorType>(reflBinding->descriptor_type);
                    binding.descriptorCount = reflBinding->count;
                    binding.stageFlags = module.shader_stage;
                    binding.pImmutableSamplers = nullptr;
                    bindings.push_back(binding);
                }
                if (!isBindless) {
                    VkDescriptorSetLayoutCreateInfo layoutInfo{};
                    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
                    layoutInfo.pBindings = bindings.data();

                    VkDescriptorSetLayout descriptorSetLayout;
                    if (vkCreateDescriptorSetLayout(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
                        throw std::runtime_error("Failed to create descriptor set layout!");
                    }
                    m_DescriptorSetLayouts[setIndex] = descriptorSetLayout;
                }
                else {
                    m_DescriptorSetLayouts[setIndex] = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_BindlessDescriptorLayout;
                }

                p_ResourceTypes.push_back((GpuDescriptorResourceType)setIndex);
            }

            uint32_t pc_count = 0;
            result = spvReflectEnumeratePushConstantBlocks(&module, &pc_count, nullptr);
            std::vector<SpvReflectBlockVariable*> pcs(pc_count);
            result = spvReflectEnumeratePushConstantBlocks(&module, &pc_count, pcs.data());

            for (auto pc : pcs) {
                VkPushConstantRange range{};
                range.stageFlags = module.shader_stage;
                range.offset = pc->offset;
                range.size = pc->size;
                pushConstants.push_back(range);
            }
        }

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(m_DescriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = m_DescriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
        pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();

        if (vkCreatePipelineLayout(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
        VkVertexInputBindingDescription bindingDescription{};
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
        if (specs.pLayout.has_value()) {
            bindingDescription.binding = specs.pLayout.value().pBinding;
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            bindingDescription.stride = specs.pLayout.value().pStride;
            for (int i = 0; i < specs.pLayout.value().pAttributes.size(); i++) {
                VkVertexInputAttributeDescription attributeDescription;
                attributeDescription.binding = specs.pLayout.value().pAttributes[i].pBinding;
                attributeDescription.location = specs.pLayout.value().pAttributes[i].pLocation;
                attributeDescription.format = UtilitiesVulkan::FormatToVkFormat(specs.pLayout.value().pAttributes[i].pFormat);
                attributeDescription.offset = specs.pLayout.value().pAttributes[i].pOffset;
                attributeDescriptions.push_back(attributeDescription);
            }

            vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
            vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
            vertexInputInfo.vertexBindingDescriptionCount = 1;
            vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        }
        else {
            vertexInputInfo.vertexAttributeDescriptionCount = 0;
            vertexInputInfo.vertexBindingDescriptionCount = 0;
        }

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

        std::vector<VkPipelineColorBlendAttachmentState> colorAttachments;
        for (int i = 0; i < specs.pRenderPass->GetColorAttachmentCount(); i++) {
            VkPipelineColorBlendAttachmentState colorBlendAttachment{};
            if (!specs.pTransparent) {
                colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
                colorBlendAttachment.blendEnable = VK_FALSE;
            }
            else {
                colorBlendAttachment.blendEnable = VK_TRUE;
                colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
                colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
                colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
                colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
                colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
            }
            colorAttachments.push_back(colorBlendAttachment);
        }

        VkPipelineColorBlendStateCreateInfo colorBlending{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = colorAttachments.size();
        colorBlending.pAttachments = colorAttachments.data();
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

        //VkPipelineShaderStageCreateInfo shaderStage{};
        VkComputePipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
        pipelineInfo.pNext = nullptr;
        //pipelineInfo.flags;
        //pipelineInfo.stage = shaderStage;
        pipelineInfo.layout = m_PipelineLayout;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        //pipelineInfo.basePipelineIndex;
    }

    void PipelineVulkan::Bind(std::shared_ptr<CommandBuffer> cmd) {
        vkCmdBindPipeline(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);

        for (auto& type : p_ResourceTypes)
        {
            switch (type)
            {
                case GpuDescriptorResourceType::MVPUBO:
                {
                    vkCmdBindDescriptorSets(
                        std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(),
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        m_PipelineLayout,
                        SET_MVP,
                        1,
                        &std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_MVPUBOSet,
                        0,
                        nullptr
                    );
                    break;
                }
                case GpuDescriptorResourceType::SceneLightsUBO:
                {
                    vkCmdBindDescriptorSets(
                        std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(),
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        m_PipelineLayout,
                        SET_LIGHTS,
                        1,
                        &std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_SceneLightsSet,
                        0,
                        nullptr
                    );
                    break;
                }
                case GpuDescriptorResourceType::DeferredTextures:
                {
                    vkCmdBindDescriptorSets(
                        std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(),
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        m_PipelineLayout,
                        SET_DEFERRED_TEXTURES,
                        1,
                        &std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_DeferredTexturesSet,
                        0,
                        nullptr
                    );
                    break;
                }
                case GpuDescriptorResourceType::BindlessTextures:
                {
                    vkCmdBindDescriptorSets(
                        std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(),
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        m_PipelineLayout,
                        SET_BINDLESS,
                        1,
                        &std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_BindlessTexturesSet,
                        0,
                        nullptr
                    );
                    break;
                }
                case GpuDescriptorResourceType::Materials:
                {
                    vkCmdBindDescriptorSets(
                        std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(),
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        m_PipelineLayout,
                        SET_MATERIALS,
                        1,
                        &std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_MaterialsSet,
                        0,
                        nullptr
                    );
                    break;
                }
                default:
                    break;
            }
        }
    }

    void PipelineVulkan::BindPushConstant(std::shared_ptr<CommandBuffer> cmd, uint32_t size, void* data, uint32_t offset, bool vertexShader) {
        if(vertexShader)
            vkCmdPushConstants(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, size, data);
    }

    void PipelineVulkan::Release() {
        for (VkShaderModule module : m_Modules) {
            vkDestroyShaderModule(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), module, nullptr);
        }        
        for (VkDescriptorSetLayout layout : m_DescriptorSetLayouts) {
            vkDestroyDescriptorSetLayout(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), layout, nullptr);
        }
        vkDestroyPipelineLayout(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), m_PipelineLayout, nullptr);
        vkDestroyPipeline(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), m_Pipeline, nullptr);
    }
}