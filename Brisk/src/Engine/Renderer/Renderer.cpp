// INCLUDES
#include "RHI.hpp"
#include "Shader.hpp"
#include "Renderer.hpp"
#include "RenderPass.hpp"
#include "RendererAPI.hpp"
#include "Engine/Model.hpp"
#include "ShaderModule.hpp"
#include "Engine/Entity.hpp"
#include "Graphics/Vulkan/PipelineVulkan.hpp"
#include "Engine/Component.hpp"
#include "Engine/SceneManager.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Graphics/Factories/SwapchainFactory.hpp"
//-----------------------------------------------

namespace Brisk
{
    std::shared_ptr<Swapchain> Renderer::m_Swapchain;

    void Renderer::Init()
    {
        m_Swapchain = SwapchainFactory::CreateSwapchain(Engine::s_Application->GetWindow());
        m_Swapchain->Create(Swapchain::DOUBLE_BUFFERING);

        std::shared_ptr<DescriptorLayout> materialLayout = DescriptorLayout::Create();
        materialLayout->pName = "material";
        materialLayout->AddBindingLayout(0, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_UNIFORM_BUFFER, { GPUResource::ShaderStageAccess::SHADER_STAGE_VERTEX_BIT });
        materialLayout->Init();

        std::shared_ptr<DescriptorLayout> pbrLayout = DescriptorLayout::Create();
        pbrLayout->pName = "pbr";
        pbrLayout->AddBindingLayout(0, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
        pbrLayout->AddBindingLayout(1, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
        pbrLayout->AddBindingLayout(2, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
        pbrLayout->Init();

        std::shared_ptr<ShaderModule> vertexShaderModule = ShaderModule::Create();
        vertexShaderModule->Init(std::make_pair("Shaders/Vulkan/Compiled/TriangleVS.spv", Pipeline::ShaderStage::VERTEX));

        std::shared_ptr<ShaderModule> fragmentShaderModule = ShaderModule::Create();
        fragmentShaderModule->Init(std::make_pair("Shaders/Vulkan/Compiled/TriangleFS.spv", Pipeline::ShaderStage::FRAGMENT));

        Pipeline::PipelineSpecs pipelineSpecs{};
        RenderPass::RenderPassSpecs renderPassSpecs;
        renderPassSpecs.pAttachments =
            {
                {0, Core::Format::FORMAT_B8G8R8A8_UNORM, true, RenderPass::AttachmentType::Swapchain},
                {1, Core::Format::FORMAT_D32_SFLOAT, true, RenderPass::AttachmentType::Depth}
            };

        Pipeline::VertexDataLayout vertexLayout;
        vertexLayout.pBinding = 0;
        vertexLayout.pStride = sizeof(MeshData);
        vertexLayout.pAttributes = {
            {0, 0, Core::Format::FORMAT_R32G32B32_SFLOAT, offsetof(MeshData, MeshData::Position)},
            {0, 1, Core::Format::FORMAT_R32G32B32_SFLOAT, offsetof(MeshData, MeshData::Normal)},
            {0, 2, Core::Format::FORMAT_R32G32_SFLOAT,    offsetof(MeshData, MeshData::UV0)},
            {0, 3, Core::Format::FORMAT_R32G32_SFLOAT,    offsetof(MeshData, MeshData::UV1)},
            {0, 4, Core::Format::FORMAT_R32G32B32_SFLOAT, offsetof(MeshData, MeshData::Color)},
        };
        pipelineSpecs.pLayout = vertexLayout;
        pipelineSpecs.pRenderPass = RenderPass::Create();
        pipelineSpecs.pRenderPass->Init(renderPassSpecs);

        pipelineSpecs.pDescriptorLayouts["material"] = materialLayout;
        //pipelineSpecs.pDescriptorLayouts["pbr"] = pbrLayout;

        pipelineSpecs.pShaderModules.push_back(vertexShaderModule);
        pipelineSpecs.pShaderModules.push_back(fragmentShaderModule);

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

        pipeline = Pipeline::Create();
        pipeline->Init(pipelineSpecs);

        cmd = CommandBuffer::Create();

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        fence = Fence::Create();
        fence->Init();

        ImageAvailableSemaphore = Semaphore::Create();
        ImageAvailableSemaphore->Init();

        RenderFinishedSemaphore = Semaphore::Create();
        RenderFinishedSemaphore->Init();

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetGraphicsQueue().FamilyIndex;

        if (vkCreateCommandPool(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create command pool!");
        }

        queue = Queue::Create();

        std::static_pointer_cast<CommandBufferVulkan>(cmd)->Allocate(m_CommandPool);
    }

    void Renderer::RenderScene(float deltaTime)
    {
        if (!SceneManager::pActiveScene) return;

        //auto view = SceneManager::pActiveScene->Reg().view<MeshComponent, MaterialComponent>();
        auto parent = SceneManager::pActiveScene->Reg().view<RootComponent>();

        fence->Wait();
        fence->Reset();

        m_Swapchain->AquireNextImage(UINT64_MAX, ImageAvailableSemaphore, nullptr, &imageIndex);
        cmd->Reset();
        cmd->Bind();
        pipeline->m_Specs.pRenderPass->Begin(cmd, imageIndex);
        pipeline->Bind(cmd);

        RenderCommand::SetViewport(cmd, 0, 0, m_Swapchain->GetExtentWidth(), m_Swapchain->GetExtentHeight(), 0, 1);
        RenderCommand::SetScissor(cmd, 0, 0, m_Swapchain->GetExtentWidth(), m_Swapchain->GetExtentHeight());

        for (auto e : parent) {
            Entity entity = { e, SceneManager::pActiveScene.get() };
            
            //std::cout << entity.GetComponent<TagComponent>().Tag << std::endl;
            //auto& mesh = entity.GetComponent<MeshComponent>();
            auto& root = entity.GetComponent<RootComponent>();
            auto& mat = entity.GetComponent<MaterialComponent>();

            RenderCommand::BindVertexBuffer(cmd, { root.m_VertexBuffer }, 0);
            RenderCommand::BindIndexBuffer(cmd, root.m_IndexBuffer, 0);

            mat.pMaterials[0]->Bind(cmd, pipeline);

            pipeline->Bind(cmd);

            HandleEntity(entity);
        }

        //std::cout << "loop ends" << std::endl;

        //for (auto e : view)
        //{
        //    Entity entity = { e, SceneManager::pActiveScene.get() };
        //    auto& mesh = entity.GetComponent<MeshComponent>();
        //    auto& mat = entity.GetComponent<MaterialComponent>();
        //    
        //    RenderCommand::BindVertexBuffer(cmd, { mesh.pModel->GetVertexBuffer() }, 0);
        //    RenderCommand::BindIndexBuffer(cmd, mesh.pModel->GetIndexBuffer(), 0);

        //    mat.pMaterials[0]->Bind(cmd, pipeline);

        //    pipeline->Bind(cmd);

        //    for (auto& node : mesh.pModel->GetNodes()) {
        //        DrawNode(mesh.pModel, mat.pMaterials, node);
        //    }
        //}

        pipeline->m_Specs.pRenderPass->End(cmd);
        cmd->UnBind();

        Queue::SubmitInfo submitInfo{};
        submitInfo.pWaitSemaphores.push_back(ImageAvailableSemaphore);
        submitInfo.pSignalSemaphores.push_back(RenderFinishedSemaphore);
        submitInfo.pWaitStages.push_back(Queue::WaitStage::PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        submitInfo.pCmdBuffers.push_back(cmd);

        // Submit queue
        queue->Submit(submitInfo, fence);

        Queue::PresentInfo presentInfo{};
        presentInfo.pWaitSemaphores.push_back(RenderFinishedSemaphore);
        presentInfo.pSwapchains.push_back(m_Swapchain);
        presentInfo.pImageIndex = imageIndex;

        // Present
        queue->Present(presentInfo);
    }

    void Renderer::HandleEntity(Entity e) {
        if (e.HasComponent<MeshComponent>()) {
            for (auto& subMesh : e.GetComponent<MeshComponent>().subMeshes) {
                uint32_t index = subMesh.material_index != -1 ? subMesh.material_index : 0;
                //materials[index]->Bind(cmd, pipeline);

                PushConstants pushConstantsData = {
                    SceneManager::pActiveScene->mMaterials[index].baseColorTextureIndex,   // Index for albedo texture 0
                    SceneManager::pActiveScene->mMaterials[index].metallicRoughnessTextureIndex, // Index for metallic texture1
                    SceneManager::pActiveScene->mMaterials[index].normalTextureIndex,   // Index for normal texture 4
                    SceneManager::pActiveScene->mMaterials[index].emissiveTextureIndex,// Index for roughness texture 2
                    SceneManager::pActiveScene->mMaterials[index].occlusionTextureIndex// Index for emissive texture 3
                };

                //pushConstantsData.camPos = Engine::s_Application->GetCamera()->GetPosition();

                pipeline->BindPushConstant(cmd, sizeof(PushConstants), &pushConstantsData);


                RenderCommand::DrawIndexed(cmd, subMesh.index_count, 1, subMesh.first_index, 0, 0);
            }
        }
        for (auto& child : e.GetComponent<TransformComponent>().children) {
            HandleEntity(child);
        }
    }

    void Renderer::DrawNode(const std::shared_ptr<Mesh> model, std::vector<std::shared_ptr<Shader>> materials, GLTF_Node* node) {
        //if (node->mesh) {
        //    for (Primitive* primitive : node->mesh->primitives) {
        //        //pipeline->Bind(cmd);

        //        RenderCommand::DrawIndexed(cmd, primitive->index_count, 1, primitive->first_index, 0, 0);
        //        //RenderCommand::Draw(cmd, primitive->vertex_count, 0);
        //    }
        //}
        //for (auto& child : node->children) {
        //    DrawNode(model, materials, child);
        //}
    }

    std::unique_ptr<Renderer> Renderer::Create()
    {
        return std::make_unique<Renderer>();
    }
}