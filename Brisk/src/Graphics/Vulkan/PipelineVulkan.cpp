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

//#include <shaderc/shaderc.hpp>
#include <spirv_reflect.h>

namespace Brisk
{
    static std::string ReadFileAsString(const std::string& fileName) {
        std::ifstream file(fileName, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open shader source: " + fileName);
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

//    std::vector<uint32_t> CompileGLSL(const std::string& source, shaderc_shader_kind kind, const std::string& filename) {
//        shaderc::Compiler compiler;
//        shaderc::CompileOptions options;
//
//#ifdef _DEBUG
//        options.SetGenerateDebugInfo();
//        options.SetOptimizationLevel(shaderc_optimization_level_zero);
//#else
//        options.SetOptimizationLevel(shaderc_optimization_level_performance);
//#endif
//
//        shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source, kind, filename.c_str(), options);
//
//        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
//            throw std::runtime_error(result.GetErrorMessage());
//        }
//
//        return { result.cbegin(), result.cend() };
//    }

    void PipelineVulkan::Init(const GraphicsPipelineSpecs& specs) {
        m_GraphicsSpecs = specs;

        m_DescriptorSetLayouts.resize(4);
        m_DescriptorSetLayouts[0] = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_DummyDescriptorLayout;
        m_DescriptorSetLayouts[1] = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_DummyDescriptorLayout;
        m_DescriptorSetLayouts[2] = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_DummyDescriptorLayout;
        m_DescriptorSetLayouts[3] = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_DummyDescriptorLayout;
        std::vector<VkPushConstantRange> pushConstants;
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        bool usingMeshShading = false;
        for (const std::string& path : specs.pShaderPathsVK) 
        {
            std::filesystem::path fsPath(path);
            std::string filename = fsPath.stem().string(); // "DepthPrePassMS"

            // Check shader stage
            if (filename.find("MS") != std::string::npos) {
                usingMeshShading = true;
                std::cout << "Mesh Shader detected: " + path << std::endl;
            }
            else if (filename.find("VS") != std::string::npos) {
                std::cout << "Vertex Shader detected\n";
            }
            else if (filename.find("FS") != std::string::npos) {
                std::cout << "Fragment Shader detected\n";
            }
            else {
                std::cout << "Unknown shader stage\n";
            }


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

            if (shaderStage.stage == VK_SHADER_STAGE_MESH_BIT_EXT) {
                usingMeshShading = true;
            }

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

                    ShaderResource shaderResource{};
                    shaderResource.p_Name = reflBinding->name;
                    shaderResource.p_Set = reflBinding->set;
                    shaderResource.p_Binding = reflBinding->binding;
                    shaderResource.p_Type = static_cast<VkDescriptorType>(reflBinding->descriptor_type);
                    shaderResource.p_Stages = module.shader_stage;
                    m_ShaderResources.push_back(shaderResource);

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
                if (set->set == 0) {
                    m_DescriptorSetLayouts[setIndex] = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_FrameGlobalDescriptorLayout;
                }
                else if (set->set == 2) {
                    m_DescriptorSetLayouts[setIndex] = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_PerMeshDescriptorLayout;
                }
                else if (set->set == 3) {
                    m_DescriptorSetLayouts[setIndex] = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_ClusteredLightingDescriptorLayout;
                }
                else if (isBindless) {
                    m_DescriptorSetLayouts[setIndex] = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_BindlessDescriptorLayout;
                }
                else {
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
        pipelineInfo.pVertexInputState = usingMeshShading ? nullptr : &vertexInputInfo;
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

        VkDebugUtilsObjectNameInfoEXT nameInfo = {};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = VK_OBJECT_TYPE_PIPELINE;
        nameInfo.objectHandle = (uint64_t)m_Pipeline;
        nameInfo.pObjectName = specs.pDebugName.c_str();
#if _DEBUG
        vkSetDebugUtilsObjectNameEXT(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &nameInfo);
#endif
    }

    void PipelineVulkan::Init(const ComputePipelineSpecs& specs) {
        m_ComputeSpecs = specs;
        m_IsCompute = true;

        m_DescriptorSetLayouts.resize(6);
        m_DescriptorSetLayouts[0] = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_DummyDescriptorLayout;
        m_DescriptorSetLayouts[1] = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_DummyDescriptorLayout;
        m_DescriptorSetLayouts[2] = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_DummyDescriptorLayout;
        m_DescriptorSetLayouts[3] = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_DummyDescriptorLayout;
        m_DescriptorSetLayouts[4] = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_DummyDescriptorLayout;
        m_DescriptorSetLayouts[5] = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_DummyDescriptorLayout;
        std::vector<VkPushConstantRange> pushConstants;
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

        const std::string& path = specs.pShaderPath;
        VkPipelineShaderStageCreateInfo shaderStage{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
        const std::vector<char>* shaderCode = UtilitiesVulkan::ReadShaderFile(path);

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = shaderCode->size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode->data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create shader module!");
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

                ShaderResource shaderResource{};
                shaderResource.p_Name = reflBinding->name;
                shaderResource.p_Set = reflBinding->set;
                shaderResource.p_Binding = reflBinding->binding;
                shaderResource.p_Type = static_cast<VkDescriptorType>(reflBinding->descriptor_type);
                shaderResource.p_Stages = module.shader_stage;
                m_ShaderResources.push_back(shaderResource);

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

            if (set->set == 3) {
                m_DescriptorSetLayouts[setIndex] = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_ClusteredLightingDescriptorLayout;
            }
            else if (set->set == 0) {
                m_DescriptorSetLayouts[setIndex] = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_FrameGlobalDescriptorLayout;
            }
            else if (isBindless) {
                m_DescriptorSetLayouts[setIndex] = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_BindlessDescriptorLayout;
            }
            else {
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

        VkPipelineLayoutCreateInfo computePipelineLayoutCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
        computePipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(m_DescriptorSetLayouts.size());
        computePipelineLayoutCreateInfo.pSetLayouts = m_DescriptorSetLayouts.data();
        computePipelineLayoutCreateInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
        computePipelineLayoutCreateInfo.pPushConstantRanges = pushConstants.data();

        if (vkCreatePipelineLayout(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &computePipelineLayoutCreateInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout!");
        }

        VkComputePipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
        pipelineInfo.stage = shaderStage;
        pipelineInfo.layout = m_PipelineLayout;

        if (vkCreateComputePipelines(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), nullptr, 1, &pipelineInfo, nullptr, &m_Pipeline) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create compute pipeline!");
        }
    }

    void PipelineVulkan::BindInternal(std::shared_ptr<CommandBuffer> cmd, VkDescriptorSet set, uint32_t setIndex) {
        vkCmdBindDescriptorSets(
            std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(),
            m_IsCompute ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS,
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
            m_IsCompute ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS, 
            m_Pipeline);

        // Totally production code
        bool once1 = false;
        bool once2 = false;
        bool once3 = false;
        bool once4 = false;
        bool once5 = false;
        bool once6 = false;

        for (const ShaderResource& resource : m_ShaderResources) {
            switch (resource.p_Set)
            {
                case SET_FRAME_GLOBAL:
                {
                    if (!once1) {
                        BindInternal(cmd, std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_GlobalSet, resource.p_Set);
                        once1 = true;
                    }
                    break;
                }
                case SET_BINDLESS_TEXTURES:
                {
                    if (!once2) {
                        BindInternal(cmd, std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_BindlessSet, resource.p_Set);
                        once2 = true;
                    }
                    break;
                }
                case SET_PER_MESH:
                {
                    if (!once3) {
                        BindInternal(cmd, std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_PerMeshSet, resource.p_Set);
                        once3 = true;
                    }
                    break;
                }
                case SET_CLUSTERED_LIGHTING:
                {
                    if (!once6) {
                        BindInternal(cmd, std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_ClusteredLightingSet, resource.p_Set);
                        once6 = true;
                    }
                    break;
                }
                default:
                {
                    BRISK_CORE_ERROR("Descriptor set not found for set index {}", resource.p_Set);
                    break;
                }
            }
        }
    }

    void PipelineVulkan::UpdateResources(const std::string& name, std::vector<std::shared_ptr<Texture>> textures, std::shared_ptr<Buffer> buffer) {
        bool resourceExists = false;
        for (const ShaderResource& resource : m_ShaderResources) {
            if (resource.p_Name == name) {
                resourceExists = true;
                switch (resource.p_Set) {
                    case SET_FRAME_GLOBAL:
                    {
                        VkWriteDescriptorSet write{};
                        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        write.descriptorType = static_cast<VkDescriptorType>(resource.p_Type);
                        write.dstSet = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_GlobalSet;
                        write.dstBinding = resource.p_Binding;
                        write.descriptorCount = 1;
                        write.pBufferInfo = std::static_pointer_cast<BufferVulkan>(buffer)->GetDescriptor();
                        vkUpdateDescriptorSets(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), 1, &write, 0, nullptr);
                        break;
                    }
                    case SET_BINDLESS_TEXTURES:
                    {
                        std::vector<VkWriteDescriptorSet> writes;
                        for (int i = 0; i < textures.size(); i++) {
                            VkWriteDescriptorSet write{};
                            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                            write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                            write.dstSet = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_BindlessSet;
                            write.dstBinding = 0;
                            write.dstArrayElement = Engine::s_TexturesOffset + i;
                            write.descriptorCount = 1;
                            write.pImageInfo = std::static_pointer_cast<TextureVulkan>(textures[i])->GetDescriptor();
                            writes.push_back(write);
                        }

                        vkUpdateDescriptorSets(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), writes.size(), writes.data(), 0, nullptr);
                        break;
                    }
                    case SET_PER_MESH:
                    {
                        VkWriteDescriptorSet write{};
                        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        write.dstSet = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_PerMeshSet;
                        write.dstBinding = resource.p_Binding;
                        write.descriptorCount = 1;
                        write.pImageInfo = std::static_pointer_cast<TextureVulkan>(textures[0])->GetDescriptor();

                        vkUpdateDescriptorSets(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), 1, &write, 0, nullptr);
                        break;
                    }
                    case SET_CLUSTERED_LIGHTING:
                    {
                        VkWriteDescriptorSet write{};
                        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        write.descriptorType = static_cast<VkDescriptorType>(resource.p_Type);
                        write.dstSet = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_ClusteredLightingSet;
                        write.dstBinding = resource.p_Binding;
                        write.descriptorCount = 1;
                        write.pBufferInfo = std::static_pointer_cast<BufferVulkan>(buffer)->GetDescriptor();

                        vkUpdateDescriptorSets(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), 1, &write, 0, nullptr);
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

    void PipelineVulkan::BindPushConstant(std::shared_ptr<CommandBuffer> cmd, uint32_t size, void* data, uint32_t offset, bool vertexShader) {
        if(vertexShader)
            vkCmdPushConstants(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, size, data);
    }

    void PipelineVulkan::Release() {
        for (VkShaderModule module : m_Modules) {
            vkDestroyShaderModule(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), module, nullptr);
        }
        //for (VkDescriptorSetLayout layout : m_DescriptorSetLayouts) {
        //    vkDestroyDescriptorSetLayout(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), layout, nullptr);
        //}
        vkDestroyPipelineLayout(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), m_PipelineLayout, nullptr);
        vkDestroyPipeline(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), m_Pipeline, nullptr);
    }
}