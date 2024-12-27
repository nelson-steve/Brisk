#include "Renderer.hpp"
#include "RHI.hpp"
#include "RenderPass.hpp"
#include "Shader.hpp"
#include "Engine/Model.hpp"
#include "RendererAPI.hpp"
#include "Graphics/Vulkan/CommandBufferVulkan.hpp"
#include "RenderCommand.hpp"
#include "Graphics/Factories/SwapchainFactory.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Component.hpp"

namespace Brisk
{
    void *m_UniformBufferData;
    VkSemaphore ImageAvailableSemaphore;
    VkSemaphore RenderFinishedSemaphore;
    VkFence fence;
    uint32_t imageIndex;
    std::shared_ptr<CommandBuffer> cmd;
    VkCommandPool m_CommandPool;
    RenderCommand command;

    std::shared_ptr<Pipeline> pipeline;
    std::shared_ptr<Shader> m_Shader;

    void Renderer::Init(const std::shared_ptr<Scene> scene)
    {
        m_Shader->SetPipeline(pipeline);

        auto view = scene->Reg().view<MaterialComponent, ModelComponent>();

        for (auto &entity : view)
        {
            auto &material = view.get<MaterialComponent>(entity);
            auto &model = view.get<ModelComponent>(entity);

            material.p_Material->p_GpuResource->Allocate(pipeline);
            {
                GPUResource::ResourceBinding binding{};
                binding.binding = 0;
                binding.texture = material.p_Material->baseColorTexture;
                binding.ResourceType = ResourceType::Texture;

                material.p_Material->p_GpuResource->AddBinding(binding);
            }
            {
                GPUResource::ResourceBinding binding{};
                binding.binding = 1;
                binding.texture = material.p_Material->metallicRoughnessTexture;
                binding.ResourceType = ResourceType::Texture;

                material.p_Material->p_GpuResource->AddBinding(binding);
            }
            {
                GPUResource::ResourceBinding binding{};
                binding.binding = 2;
                binding.texture = material.p_Material->normalTexture;
                binding.ResourceType = ResourceType::Texture;

                material.p_Material->p_GpuResource->AddBinding(binding);
            }
            {
                GPUResource::ResourceBinding binding{};
                binding.binding = 3;
                binding.texture = material.p_Material->occlusionTexture;
                binding.ResourceType = ResourceType::Texture;

                material.p_Material->p_GpuResource->AddBinding(binding);
            }
            {
                GPUResource::ResourceBinding binding{};
                binding.binding = 4;
                binding.texture = material.p_Material->emissiveTexture;
                binding.ResourceType = ResourceType::Texture;

                material.p_Material->p_GpuResource->AddBinding(binding);
            }

            material.p_Material->p_GpuResource->UpdateResource();
        }

        /*
                for (auto &entity : view)
                {
                    auto &shader = view.get<ShaderComponent>(entity);
                    for (size_t i = 0; i < scene_object->p_model.GetMaterials().size(); i++)
                    {
                        VkDescriptorSetAllocateInfo allocInfo{};
                        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                        allocInfo.descriptorPool = m_descriptor_pools.scene;
                        allocInfo.descriptorSetCount = 1;
                        allocInfo.pSetLayouts = &m_descriptorSetLayouts.model;

                        if (vkAllocateDescriptorSets(m_device, &allocInfo, &(scene_object->p_model.GetMaterial(i).descriptorSet)) != VK_SUCCESS)
                        {
                            throw std::runtime_error("failed to allocate descriptor sets!");
                        }

                        VkDescriptorBufferInfo bufferInfo{};
                        bufferInfo.buffer = scene_object->p_ubo.uniformBuffers[0];
                        bufferInfo.offset = 0;
                        bufferInfo.range = sizeof(UBO);

                        VkDescriptorBufferInfo shaderValuesBufferInfo{};
                        shaderValuesBufferInfo.buffer = scene_object->p_shader_values_ubo.uniformBuffers[0];
                        shaderValuesBufferInfo.offset = 0;
                        shaderValuesBufferInfo.range = sizeof(UBOShaderValues);

                        if (scene_object->p_model.GetMaterial(i).baseColorTexture == nullptr)
                        {
                            scene_object->p_model.GetMaterial(i).baseColorTexture = m_white_texture;
                            std::cout << "base color texture not found" << std::endl;
                        }
                        if (scene_object->p_model.GetMaterial(i).metallicRoughnessTexture == nullptr)
                        {
                            scene_object->p_model.GetMaterial(i).metallicRoughnessTexture = m_white_texture;
                            std::cout << "metal roughness texture not found" << std::endl;
                        }
                        if (scene_object->p_model.GetMaterial(i).normalTexture == nullptr)
                        {
                            scene_object->p_model.GetMaterial(i).normalTexture = m_white_texture;
                            std::cout << "normal texture not found" << std::endl;
                        }
                        if (scene_object->p_model.GetMaterial(i).occlusionTexture == nullptr)
                        {
                            scene_object->p_model.GetMaterial(i).occlusionTexture = m_white_texture;
                            std::cout << "occlusion texture not found" << std::endl;
                        }
                        if (scene_object->p_model.GetMaterial(i).emissiveTexture == nullptr)
                        {
                            scene_object->p_model.GetMaterial(i).emissiveTexture = m_white_texture;
                            std::cout << "emissive texture not found" << std::endl;
                        }

                        std::vector<VkDescriptorImageInfo> image_descriptors = {
                            scene_object->p_model.GetMaterial(i).baseColorTexture->m_descriptor,
                            scene_object->p_model.GetMaterial(i).metallicRoughnessTexture->m_descriptor,
                            scene_object->p_model.GetMaterial(i).normalTexture->m_descriptor,
                            scene_object->p_model.GetMaterial(i).occlusionTexture->m_descriptor,
                            scene_object->p_model.GetMaterial(i).emissiveTexture->m_descriptor,
                        };

                        std::vector<VkWriteDescriptorSet> descriptorWrites;
                        descriptorWrites.resize(7);
                        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                        descriptorWrites[0].dstSet = scene_object->p_model.GetMaterial(i).descriptorSet;
                        descriptorWrites[0].dstBinding = 0;
                        descriptorWrites[0].descriptorCount = 1;
                        descriptorWrites[0].pBufferInfo = &bufferInfo;

                        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                        descriptorWrites[1].dstSet = scene_object->p_model.GetMaterial(i).descriptorSet;
                        descriptorWrites[1].dstBinding = 1;
                        descriptorWrites[1].descriptorCount = 1;
                        descriptorWrites[1].pBufferInfo = &shaderValuesBufferInfo;

                        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[2].dstSet = scene_object->p_model.GetMaterial(i).descriptorSet;
                        descriptorWrites[2].dstBinding = 2;
                        descriptorWrites[2].descriptorCount = 1;
                        descriptorWrites[2].pImageInfo = &image_descriptors[0];

                        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[3].dstSet = scene_object->p_model.GetMaterial(i).descriptorSet;
                        descriptorWrites[3].dstBinding = 3;
                        descriptorWrites[3].descriptorCount = 1;
                        descriptorWrites[3].pImageInfo = &image_descriptors[1];

                        descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[4].dstSet = scene_object->p_model.GetMaterial(i).descriptorSet;
                        descriptorWrites[4].dstBinding = 4;
                        descriptorWrites[4].descriptorCount = 1;
                        descriptorWrites[4].pImageInfo = &image_descriptors[2];

                        descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[5].dstSet = scene_object->p_model.GetMaterial(i).descriptorSet;
                        descriptorWrites[5].dstBinding = 5;
                        descriptorWrites[5].descriptorCount = 1;
                        descriptorWrites[5].pImageInfo = &image_descriptors[3];

                        descriptorWrites[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[6].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[6].dstSet = scene_object->p_model.GetMaterial(i).descriptorSet;
                        descriptorWrites[6].dstBinding = 6;
                        descriptorWrites[6].descriptorCount = 1;
                        descriptorWrites[6].pImageInfo = &image_descriptors[4];

                        vkUpdateDescriptorSets(m_device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
                    }
                }
        */

        m_Swapchain = SwapchainFactory::CreateSwapchain(Engine::s_Application->GetWindow());
        m_Swapchain->Create(Swapchain::DOUBLE_BUFFERING);

        std::shared_ptr<DescriptorLayout> materialLayout = DescriptorLayout::Create();
        materialLayout->AddBindingLayout(0, 1, ResourceType::UNIFORM_BUFFER);
        materialLayout->AddBindingLayout(1, 1, ResourceType::UNIFORM_BUFFER);
        materialLayout->AddBindingLayout(2, 1, ResourceType::COMBINED_IMAGE_SAMPLER);
        materialLayout->AddBindingLayout(3, 1, ResourceType::COMBINED_IMAGE_SAMPLER);
        materialLayout->AddBindingLayout(4, 1, ResourceType::COMBINED_IMAGE_SAMPLER);
        materialLayout->AddBindingLayout(5, 1, ResourceType::COMBINED_IMAGE_SAMPLER);
        materialLayout->AddBindingLayout(6, 1, ResourceType::COMBINED_IMAGE_SAMPLER);
        materialLayout->Init();

        std::shared_ptr<DescriptorLayout> pbrLayout = DescriptorLayout::Create();
        pbrLayout->AddBindingLayout(0, 1, ResourceType::COMBINED_IMAGE_SAMPLER);
        pbrLayout->AddBindingLayout(1, 1, ResourceType::COMBINED_IMAGE_SAMPLER);
        pbrLayout->AddBindingLayout(2, 1, ResourceType::COMBINED_IMAGE_SAMPLER);
        pbrLayout->Init();

        std::shared_ptr<Shader> vertexShader = Shader::Create();
        vertexShader->Init(std::make_pair("Shaders/Vulkan/Compiled/TriangleVS.spv", Pipeline::ShaderStage::VERTEX));

        std::shared_ptr<Shader> fragmentShader = Shader::Create();
        fragmentShader->Init(std::make_pair("Shaders/Vulkan/Compiled/TriangleFS.spv", Pipeline::ShaderStage::FRAGMENT));

        vertexShader->AddDescriptorLayout(materialLayout);
        fragmentShader->AddDescriptorLayout(pbrLayout);

        Pipeline::PipelineSpecs pipelineSpecs{};
        RenderPass::RenderPassSpecs renderPassSpecs;
        renderPassSpecs.pAttachments =
            {
                {0, Core::Format::FORMAT_B8G8R8A8_UNORM, true, RenderPass::AttachmentType::Swapchain},
                {1, Core::Format::FORMAT_D32_SFLOAT, true, RenderPass::AttachmentType::Depth}};

        Pipeline::VertexDataLayout vertexLayout;
        vertexLayout.pBinding = 0;
        vertexLayout.pStride = sizeof(Point);
        vertexLayout.pAttributes = {
            {0, 0, Core::Format::FORMAT_R32G32B32_SFLOAT, offsetof(Point, Point::Position)},
            {0, 1, Core::Format::FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Vertex::normal)},
            {0, 2, Core::Format::FORMAT_R32G32_SFLOAT, offsetof(Vertex, Vertex::uv0)},
            {0, 3, Core::Format::FORMAT_R32G32_SFLOAT, offsetof(Vertex, Vertex::uv1)},
            {0, 1, Core::Format::FORMAT_R32G32B32_SFLOAT, offsetof(Point, Point::Color)},
        };
        pipelineSpecs.Layout = vertexLayout;
        pipelineSpecs.pRenderPass = RenderPass::Create();
        pipelineSpecs.pRenderPass->Init(renderPassSpecs);

        pipelineSpecs.pDescriptorLayouts.push_back(vertexLayoutSet1);
        pipelineSpecs.pDescriptorLayouts.push_back(vertexLayoutSet2);

        pipelineSpecs.pShaders.push_back(vertexShader);
        pipelineSpecs.pShaders.push_back(fragmentShader);

        pipelineSpecs.pDepthClampEnable = false;
        pipelineSpecs.pRasterizationDiscardEnable = false;
        pipelineSpecs.pPolygoneMode = Pipeline::POLYGON_MODE_FILL;
        pipelineSpecs.pLineWidth = 1.0f;
        pipelineSpecs.pCullMode = Pipeline::CullMode::BACK;
        pipelineSpecs.pFrontFace = Pipeline::FrontFace::CLOCKWISE;
        pipelineSpecs.pDepthBiasEnable = false;
        pipelineSpecs.pDepthTestEnable = true;
        pipelineSpecs.pDepthWriteEnable = true;
        pipelineSpecs.pCompareOp = Pipeline::COMPARE_OP_LESS;
        pipelineSpecs.pDepthBoundsTestEnable = false;
        pipelineSpecs.pStencilTestEnable = false;

        pipeline = Pipeline::Create();
        pipeline->Init(pipelineSpecs);

        cmd = CommandBuffer::Create();

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateSemaphore(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), &semaphoreInfo, nullptr, &ImageAvailableSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), &semaphoreInfo, nullptr, &RenderFinishedSemaphore) != VK_SUCCESS ||
            vkCreateFence(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), &fenceInfo, nullptr, &fence) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetGraphicsQueue().FamilyIndex;

        if (vkCreateCommandPool(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create command pool!");
        }

        std::static_pointer_cast<CommandBufferVulkan>(cmd)->Allocate(m_CommandPool);
    }

    void Renderer::RenderScene(float deltaTime)
    {
        vkWaitForFences(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), 1, &fence, VK_TRUE, UINT64_MAX);

        std::static_pointer_cast<SwapchainVulkan>(m_Swapchain)->AquireNextImage(UINT64_MAX, ImageAvailableSemaphore, fence, &imageIndex);
        // vkResetCommandBuffer(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), /*VkCommandBufferResetFlagBits*/ 0);
        cmd->Reset();
        cmd->Bind();
        pipeline->m_Specs.pRenderPass->Begin(cmd, imageIndex);
        pipeline->Bind(cmd);

        cmd->RecordCommand([=]()
                           {
            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(m_Swapchain->GetExtentWidth());
            viewport.height = static_cast<float>(m_Swapchain->GetExtentHeight());
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), 0, 1, &viewport); });

        cmd->RecordCommand([&]()
                           {
            VkRect2D scissor{};
            scissor.offset = { 0, 0 };
            scissor.extent = std::static_pointer_cast<SwapchainVulkan>(m_Swapchain)->GetExtent();
            vkCmdSetScissor(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), 0, 1, &scissor); });

        vkResetFences(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), 1, &fence);

        // const VkBuffer vertexBuffers[] = { std::static_pointer_cast<BufferVulkan>(m_VertexBuffer)->Get() };
        VkDeviceSize offsets[] = {0};
        // vkCmdBindVertexBuffers(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), 0, 1, vertexBuffers, offsets);
        // vkCmdBindDescriptorSets(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), VK_PIPELINE_BIND_POINT_GRAPHICS, std::static_pointer_cast<PipelineVulkan>(pipeline)->GetLayout(), 0, 1, &m_DescriptorSet, 0, nullptr);
        // RenderCommand::Draw(cmd, static_cast<uint32_t>(vertices.size()), 0);

        pipeline->m_Specs.pRenderPass->End(cmd);
        cmd->UnBind();

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkSemaphore waitSemaphores[] = {ImageAvailableSemaphore};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        std::vector<VkCommandBuffer> cmdBufers = {std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get()};
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = cmdBufers.data();

        VkSemaphore signalSemaphores[] = {RenderFinishedSemaphore};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetGraphicsQueue().Handle, 1, &submitInfo, fence) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {std::static_pointer_cast<SwapchainVulkan>(m_Swapchain)->GetSwapchain()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        vkQueuePresentKHR(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetGraphicsQueue().Handle, &presentInfo);
    }

    std::unique_ptr<Renderer> Renderer::Create()
    {
        return std::make_unique<Renderer>();
    }
}