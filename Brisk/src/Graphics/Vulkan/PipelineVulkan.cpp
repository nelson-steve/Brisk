#include "pch.hpp"
#include "PipelineVulkan.hpp"
#include "Core/Log.hpp"
#include "Engine/Engine.hpp"
#include "SwapchainVulkan.hpp"
#include "UtilitiesVulkan.hpp"
#include "RenderPassVulkan.hpp"
#include "DescriptorLayoutVulkan.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "BufferVulkan.hpp"
#include "TextureVulkan.hpp"

#include <spirv_reflect.h>
#include "CSMRenderpassVulkan.hpp"
#include "TLASVulkan.hpp"

namespace Brisk
{
// Number of descriptor sets used in all the shaders
#define NO_OF_DESCRIPTOR_SETS 4

    static std::string ReadFileAsString(const std::string& fileName) {
        std::ifstream file(fileName, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open shader source: " + fileName);
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    void GetPipelineData(const std::vector<std::string>& shaderPaths, 
        std::vector<VkShaderModule>& modules, 
        std::vector<VkDescriptorSetLayout>& descriptorSetLayouts, 
        std::vector<VkPipelineShaderStageCreateInfo>& shaderStages,
        std::vector<PipelineVulkan::ShaderResource>& shaderResources,
        std::vector<VkPushConstantRange>& pushConstants,
        std::vector<uint32_t>& setIndices,
        bool& isUsingMeshShading) {

        // Setting a dummy descriptor set layout for all layouts initially in case a pipeline doesnt use a layout and it needs to be skipped
        // Since Vulkan doesn't allow null descriptor set layouts
        descriptorSetLayouts.resize(NO_OF_DESCRIPTOR_SETS);
        for (int i = 0; i < NO_OF_DESCRIPTOR_SETS; i++) {
            descriptorSetLayouts[i] = std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->m_DummyDescriptorLayout;
        }

        for (const std::string& path : shaderPaths)
        {
            std::filesystem::path fsPath(path);
            std::string filename = fsPath.stem().string();

            VkPipelineShaderStageCreateInfo shaderStage{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
            const std::vector<char>* shaderCode = UtilitiesVulkan::ReadShaderFile(path);

            VkShaderModuleCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = shaderCode->size();
            createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode->data());

            SpvReflectShaderModule module;
            SpvReflectResult result = spvReflectCreateShaderModule(shaderCode->size(), shaderCode->data(), &module);

            VkShaderModule shaderModule;
            if (vkCreateShaderModule(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
                throw std::runtime_error("failed to create shader module!");
            }
            shaderStage.module = shaderModule;
            shaderStage.pName = module.entry_point_name;
            shaderStage.stage = static_cast<VkShaderStageFlagBits>(module.shader_stage);

            if (shaderStage.stage == VK_SHADER_STAGE_MESH_BIT_EXT) {
                isUsingMeshShading = true;
            }

            modules.push_back(shaderModule);
            shaderStages.push_back(shaderStage);

            uint32_t setCount = 0;
            result = spvReflectEnumerateDescriptorSets(&module, &setCount, nullptr);
            std::vector<SpvReflectDescriptorSet*> sets(setCount);
            result = spvReflectEnumerateDescriptorSets(&module, &setCount, sets.data());

            for (auto set : sets) {
                uint32_t setIndex = set->set;

                setIndices.push_back(setIndex);

                for (uint32_t i = 0; i < set->binding_count; ++i) {
                    const SpvReflectDescriptorBinding* reflBinding = set->bindings[i];
                    PipelineVulkan::ShaderResource shaderResource{};
                    shaderResource.p_Name = reflBinding->name;
                    shaderResource.p_Set = reflBinding->set;
                    shaderResource.p_Binding = reflBinding->binding;
                    shaderResource.p_Type = static_cast<VkDescriptorType>(reflBinding->descriptor_type);
                    shaderResource.p_Stages = module.shader_stage;
                    shaderResources.push_back(shaderResource);
                }

                if (setIndex == 0) {
                    descriptorSetLayouts[setIndex] = std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->m_FrameGlobalDescriptorLayout;
                }
                else if (setIndex == 1) {
                    descriptorSetLayouts[setIndex] = std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->m_BindlessDescriptorLayout;
                }
                else if (setIndex == 2) {
                    descriptorSetLayouts[setIndex] = std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->m_PerMeshDescriptorLayout;
                }
                else if (setIndex == 3) {
                    descriptorSetLayouts[setIndex] = std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->m_ClusteredLightingDescriptorLayout;
                }
                else {
                    BRISK_CORE_ERROR("Invalid descriptor set index {}", setIndex);
                }
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
            delete shaderCode;
        }
    }

    void PipelineVulkan::Init(const GraphicsPipelineSpecs& specs) {
        m_GraphicsSpecs = specs;

        bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        std::vector<VkShaderModule> shaderModules;
        std::vector<VkPushConstantRange> pushConstants;
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        bool usingMeshShading = false;

        GetPipelineData(specs.pShaderPathsVK, shaderModules, m_DescriptorSetLayouts, shaderStages, m_ShaderResources, pushConstants, m_SetIndices, usingMeshShading);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(m_DescriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = m_DescriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
        pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();

        if (vkCreatePipelineLayout(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
        VkVertexInputBindingDescription bindingDescription{};
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
        if (specs.pLayout.has_value() && !usingMeshShading) {
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
        depthStencil.depthCompareOp = specs.pCompareOp == CompareOp::COMPARE_OP_LESS ? VK_COMPARE_OP_LESS : VK_COMPARE_OP_LESS_OR_EQUAL;
        depthStencil.depthBoundsTestEnable = specs.pDepthBoundsTestEnable;
        depthStencil.stencilTestEnable = specs.pStencilTestEnable;
        depthStencil.back.compareOp = VK_COMPARE_OP_ALWAYS;
        depthStencil.front = depthStencil.back;

        std::vector<VkPipelineColorBlendAttachmentState> colorAttachments;
        if (specs.pRenderPass) {
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
        pipelineInfo.pVertexInputState = usingMeshShading ? nullptr : &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = m_PipelineLayout;
        if (specs.pRenderPass)
            pipelineInfo.renderPass = std::static_pointer_cast<RenderPassVulkan>(specs.pRenderPass)->GetRenderPass();
        else
            pipelineInfo.renderPass = std::static_pointer_cast<CSMRenderPassVulkan>(specs.pCSMRenderPass)->GetRenderPass();
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        for (int i = 0; i < shaderModules.size(); i++) {
            vkDestroyShaderModule(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), shaderModules[i], nullptr);
        }

        VkDebugUtilsObjectNameInfoEXT nameInfo = {};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = VK_OBJECT_TYPE_PIPELINE;
        nameInfo.objectHandle = (uint64_t)m_Pipeline;
        nameInfo.pObjectName = specs.pDebugName.c_str();
#if _DEBUG
        vkSetDebugUtilsObjectNameEXT(Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &nameInfo);
#endif
    }

    void PipelineVulkan::Init(const ComputePipelineSpecs& specs) {
        m_ComputeSpecs = specs;

        bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;

        std::vector<VkShaderModule> shaderModules;
        std::vector<VkPushConstantRange> pushConstants;
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

        bool usingMeshShading = false;
        GetPipelineData({ specs.pShaderPath }, shaderModules, m_DescriptorSetLayouts, shaderStages, m_ShaderResources, pushConstants, m_SetIndices, usingMeshShading);

        VkPipelineLayoutCreateInfo computePipelineLayoutCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
        computePipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(m_DescriptorSetLayouts.size());
        computePipelineLayoutCreateInfo.pSetLayouts = m_DescriptorSetLayouts.data();
        computePipelineLayoutCreateInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
        computePipelineLayoutCreateInfo.pPushConstantRanges = pushConstants.data();

        if (vkCreatePipelineLayout(Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &computePipelineLayoutCreateInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout!");
        }

        VkComputePipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
        pipelineInfo.stage = shaderStages[0];
        pipelineInfo.layout = m_PipelineLayout;

        if (vkCreateComputePipelines(Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), nullptr, 1, &pipelineInfo, nullptr, &m_Pipeline) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create compute pipeline!");
        }

        vkDestroyShaderModule(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), shaderModules[0], nullptr);
    }

    void PipelineVulkan::Init(const RayTracingPipelineSpecs& specs) {
        bindPoint = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;

        std::vector<VkShaderModule> shaderModules;
        std::vector<VkPushConstantRange> pushConstants;
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        bool usingMeshShading = false;

        GetPipelineData(specs.pShaderPathsVK, shaderModules, m_DescriptorSetLayouts, shaderStages, m_ShaderResources, pushConstants, m_SetIndices, usingMeshShading);

        // Raygen
        // Miss 
        // Closest Hit

        std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups{};

        VkRayTracingShaderGroupCreateInfoKHR shaderGroup{ VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR };

        shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        shaderGroup.generalShader = 0;
        shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
        shaderGroups.push_back(shaderGroup);

        shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        shaderGroup.generalShader = 1;
        shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
        shaderGroups.push_back(shaderGroup);

        shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        shaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.closestHitShader = 2;
        shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
        shaderGroups.push_back(shaderGroup);

        VkPipelineLayoutCreateInfo rayTracinglineLayoutCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
        rayTracinglineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(m_DescriptorSetLayouts.size());
        rayTracinglineLayoutCreateInfo.pSetLayouts = m_DescriptorSetLayouts.data();
        rayTracinglineLayoutCreateInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
        rayTracinglineLayoutCreateInfo.pPushConstantRanges = pushConstants.data();

        if (vkCreatePipelineLayout(Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &rayTracinglineLayoutCreateInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout!");
        }

        VkRayTracingPipelineCreateInfoKHR createinfo{ VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR };
        createinfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        createinfo.pStages = shaderStages.data();
        createinfo.groupCount = static_cast<uint32_t>(shaderGroups.size());
        createinfo.pGroups = shaderGroups.data();
        createinfo.maxPipelineRayRecursionDepth = 2; //std::min(uint32_t(2), /*rayTracingPipelineProperties.maxRayRecursionDepth*/31);
        createinfo.layout = m_PipelineLayout;

        if (vkCreateRayTracingPipelinesKHR(
            Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &createinfo, VK_NULL_HANDLE, &m_Pipeline) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create raytracing pipeline!");
        }
    }

    void PipelineVulkan::BindInternal(std::shared_ptr<CommandBuffer> cmd, VkDescriptorSet set, uint32_t setIndex) {
        vkCmdBindDescriptorSets(
            std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(),
            bindPoint,
            m_PipelineLayout,
            setIndex,
            1,
            &set,
            0,
            nullptr
        );
    }

    void PipelineVulkan::Bind(std::shared_ptr<CommandBuffer> cmd) {
        vkCmdBindPipeline(
            std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(),
            bindPoint,
            m_Pipeline);

        for (uint32_t index : m_SetIndices) {
            VkDescriptorSet set;
            if (index == SET_FRAME_GLOBAL) {
                set = std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->m_GlobalSet;
            }
            else if (index == SET_BINDLESS_TEXTURES) {
                set = std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->m_BindlessSet;
            }
            else if (index == SET_PER_MESH) {
                set = std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->m_PerMeshSet;
            }
            else if (index == SET_CLUSTERED_LIGHTING) {
                set = std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->m_ClusteredLightingSet;
            }
            else {
                BRISK_CORE_ERROR("Invalid set index {}", index);
            }

            vkCmdBindDescriptorSets(
                std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(),
                bindPoint,
                m_PipelineLayout,
                index,
                1,
                &set,
                0,
                nullptr
            );
        }
    }

    void PipelineVulkan::UpdateResources(const std::string& name, std::vector<std::shared_ptr<Texture>> textures, std::shared_ptr<Buffer> buffer, std::shared_ptr<TLAS> tlas) {
        bool resourceExists = false;
        for (const ShaderResource& resource : m_ShaderResources) {
            if (resource.p_Name == name) {
                resourceExists = true;
                switch (resource.p_Set) {
                case SET_FRAME_GLOBAL:
                {
                    if (buffer) {
                        VkWriteDescriptorSet write{};
                        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        write.descriptorType = static_cast<VkDescriptorType>(resource.p_Type);
                        write.dstSet = std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->m_GlobalSet;
                        write.dstBinding = resource.p_Binding;
                        write.descriptorCount = 1;
                        write.pBufferInfo = std::static_pointer_cast<BufferVulkan>(buffer)->GetDescriptor();
                        vkUpdateDescriptorSets(Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), 1, &write, 0, nullptr);
                    }
                    else if (tlas) {
                        VkWriteDescriptorSetAccelerationStructureKHR descriptorAccelerationStructureInfo{};
                        descriptorAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
                        descriptorAccelerationStructureInfo.accelerationStructureCount = 1;
                        descriptorAccelerationStructureInfo.pAccelerationStructures = std::static_pointer_cast<TLASVulkan>(tlas)->GetHandle();

                        VkWriteDescriptorSet accelerationStructureWrite{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
                        accelerationStructureWrite.pNext = &descriptorAccelerationStructureInfo;
                        accelerationStructureWrite.dstSet = std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->m_GlobalSet;
                        accelerationStructureWrite.dstBinding = resource.p_Binding;
                        accelerationStructureWrite.descriptorCount = 1;
                        accelerationStructureWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
                        vkUpdateDescriptorSets(Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), 1, &accelerationStructureWrite, 0, nullptr);
                    }
                    else {
                        std::vector<VkDescriptorImageInfo> imageInfos;
                        for (int i = 0; i < textures.size(); i++) {
                            VkDescriptorImageInfo* imageInfo = std::static_pointer_cast<TextureVulkan>(textures[i])->GetDescriptor();
                            imageInfos.push_back(*imageInfo);
                        }
                        VkWriteDescriptorSet write{};
                        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        write.descriptorType = static_cast<VkDescriptorType>(resource.p_Type);
                        write.dstSet = std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->m_GlobalSet;
                        write.dstBinding = resource.p_Binding;
                        write.descriptorCount = imageInfos.size();
                        write.pImageInfo = imageInfos.data();
                        vkUpdateDescriptorSets(Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), 1, &write, 0, nullptr);
                    }
                    break;
                }
                case SET_BINDLESS_TEXTURES:
                {
                    std::vector<VkWriteDescriptorSet> writes;
                    for (int i = 0; i < textures.size(); i++) {
                        VkWriteDescriptorSet write{};
                        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        write.dstSet = std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->m_BindlessSet;
                        write.dstBinding = 0;
                        write.dstArrayElement = i;
                        write.descriptorCount = 1;
                        write.pImageInfo = std::static_pointer_cast<TextureVulkan>(textures[i])->GetDescriptor();
                        writes.push_back(write);
                    }

                    vkUpdateDescriptorSets(Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), writes.size(), writes.data(), 0, nullptr);
                    break;
                }
                case SET_PER_MESH:
                {
                    VkWriteDescriptorSet write{};
                    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    write.dstSet = std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->m_PerMeshSet;
                    write.dstBinding = resource.p_Binding;
                    write.descriptorCount = 1;
                    write.pImageInfo = std::static_pointer_cast<TextureVulkan>(textures[0])->GetDescriptor();

                    vkUpdateDescriptorSets(Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), 1, &write, 0, nullptr);
                    break;
                }
                case SET_CLUSTERED_LIGHTING:
                {
                    VkWriteDescriptorSet write{};
                    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    write.descriptorType = static_cast<VkDescriptorType>(resource.p_Type);
                    write.dstSet = std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->m_ClusteredLightingSet;
                    write.dstBinding = resource.p_Binding;
                    write.descriptorCount = 1;
                    write.pBufferInfo = std::static_pointer_cast<BufferVulkan>(buffer)->GetDescriptor();

                    vkUpdateDescriptorSets(Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), 1, &write, 0, nullptr);
                    break;
                }
                default:
                {
                    BRISK_CORE_ERROR("Invalid descriptor set index");
                    break;
                }
                }

                break;
            }
        }
        if (!resourceExists) {
            BRISK_CORE_ERROR("Shader resrouce {} does not exist", name);
        }
    }

    void PipelineVulkan::BindPushConstant(std::shared_ptr<CommandBuffer> cmd, uint32_t size, void* data, uint32_t offset, Core::ShaderStageFlags stages) {
        VkShaderStageFlags flags = UtilitiesVulkan::ShaderStageToVkShaderStageFlags(stages);
        vkCmdPushConstants(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), m_PipelineLayout, flags, 0, size, data);
    }

    void PipelineVulkan::Release() {
        //for (VkDescriptorSetLayout layout : m_DescriptorSetLayouts) {
        //    vkDestroyDescriptorSetLayout(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), layout, nullptr);
        //}
        vkDestroyPipelineLayout(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), m_PipelineLayout, nullptr);
        vkDestroyPipeline(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), m_Pipeline, nullptr);
    }
}