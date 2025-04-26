// INCLUDES
#include "RHI.hpp"
#include "Shader.hpp"
#include "Renderer.hpp"
#include "RenderPass.hpp"
#include "RendererAPI.hpp"
#include "Engine/Model.hpp"
#include "ShaderModule.hpp"
#include "Engine/Entity.hpp"
#include "RenderCommand.hpp"
#include "ComputeCommand.hpp"
#include "Engine/Component.hpp"
#include "Engine/SceneManager.hpp"
#include "Graphics/Factories/SwapchainFactory.hpp"

namespace Brisk
{
    std::shared_ptr<Swapchain> Renderer::m_Swapchain;

    void Renderer::Init()
    {
        RenderCommand::s_RendererAPI = RendererAPI::Create();
        ComputeCommand::s_ComputeAPI = ComputeAPI::Create();

        m_Swapchain = SwapchainFactory::CreateSwapchain(Engine::s_Application->GetWindow());
        m_Swapchain->Create(Swapchain::DOUBLE_BUFFERING);

        // Renderpasses
        {
            //----------------------------------------------------------------------------------------------------
            g_Pos = Texture::Create();
            g_Normal = Texture::Create();
            g_Albedo = Texture::Create();
            g_Depth = Texture::Create();

            {
                Texture::TextureSpecification specs{};
                specs.p_Width = 1920;
                specs.p_Height = 1080;
                specs.p_Type = Texture::TextureType::TEXTURE2D;
                specs.p_Format = Core::Format::FORMAT_R16G16B16A16_SFLOAT;
                g_Pos->Init(specs);

                g_Normal->Init(specs);

                specs.p_Format = Core::Format::FORMAT_R8G8B8A8_UNORM;
                g_Albedo->Init(specs);

                specs.p_Format = Core::Format::FORMAT_D16_UNORM;
                specs.p_IsDepth = true;
                g_Depth->Init(specs);
            }

            // Geometry pass
            //----------------------------------------------------------------------------------------------------
            m_GeometryBufferPass = RenderPass::Create();
            m_GeometryBufferPass->Init(
                {},
                {   RenderPassAttachment{ 0, AttachmentType::Color, g_Pos    },
                    RenderPassAttachment{ 1, AttachmentType::Color, g_Normal },
                    RenderPassAttachment{ 2, AttachmentType::Color, g_Albedo },
                    RenderPassAttachment{ 3, AttachmentType::Depth, g_Depth  } }
            );

            // Lighting pass
            //----------------------------------------------------------------------------------------------------
            g_lightingOutput = Texture::Create();

            {
                Texture::TextureSpecification specs{};
                specs.p_Width = 1920;
                specs.p_Height = 1080;
                specs.p_Format = Core::Format::FORMAT_R16G16B16A16_SFLOAT;
                g_lightingOutput->Init(specs);
            }

            m_LightingPass = RenderPass::Create();
            m_LightingPass->Init(
                {},
                {   RenderPassAttachment{ 0, AttachmentType::Color, g_lightingOutput } }
            );
        }

        // Pipelines
        {
            // Geometry pass pipeline
            //----------------------------------------------------------------------------------------------------
            {
                std::shared_ptr<ShaderModule> vertexShaderModule = ShaderModule::Create();
                vertexShaderModule->Init("Shaders/Vulkan/DeferredRenderer/Compiled/GeometryVS.spv", Pipeline::ShaderStage::VERTEX);
                std::shared_ptr<ShaderModule> fragmentShaderModule = ShaderModule::Create();
                fragmentShaderModule->Init("Shaders/Vulkan/DeferredRenderer/Compiled/GeometryFS.spv", Pipeline::ShaderStage::FRAGMENT);

                Pipeline::GraphicsPipelineSpecs pipelineSpecs{};
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
                pipelineSpecs.pRenderPass = m_GeometryBufferPass;

                {
                    std::shared_ptr<DescriptorLayout> layout = DescriptorLayout::Create();

                    layout->SetGlobal(true);
                    pipelineSpecs.pDescriptorLayouts.push_back(layout);
                }

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

                m_GBufferPipeline = Pipeline::Create();
                m_GBufferPipeline->Init(pipelineSpecs);
            }
            //----------------------------------------------------------------------------------------------------

            // Lighting pass pipeline
            //----------------------------------------------------------------------------------------------------
            {
                std::shared_ptr<ShaderModule> vertexShaderModule = ShaderModule::Create();
                vertexShaderModule->Init("Shaders/Vulkan/DeferredRenderer/Compiled/LightingVS.spv", Pipeline::ShaderStage::VERTEX);
                std::shared_ptr<ShaderModule> fragmentShaderModule = ShaderModule::Create();
                fragmentShaderModule->Init("Shaders/Vulkan/DeferredRenderer/Compiled/LightingFS.spv", Pipeline::ShaderStage::FRAGMENT);

                Pipeline::GraphicsPipelineSpecs pipelineSpecs{};
                Pipeline::VertexDataLayout vertexLayout;
                vertexLayout.pBinding = 0;
                vertexLayout.pStride = sizeof(MeshData);
                //vertexLayout.pAttributes = {};
                pipelineSpecs.pLayout = vertexLayout;
                pipelineSpecs.pRenderPass = m_LightingPass;

                {
                    std::shared_ptr<DescriptorLayout> layout = DescriptorLayout::Create();

                    layout->AddBinding(0, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_SAMPLED_IMAGE, { GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
                    layout->AddBinding(1, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_SAMPLED_IMAGE, { GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
                    layout->AddBinding(2, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_SAMPLED_IMAGE, { GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
                    layout->AddBinding(3, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_SAMPLED_IMAGE, { GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });

                    layout->AddBinding(0, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
                    layout->AddBinding(1, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
                    layout->AddBinding(2, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
                    layout->SetGlobal(true);
                    pipelineSpecs.pDescriptorLayouts.push_back(layout);
                }
                // TODO: Add bindless descriptor

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

                m_LightingPipeline = Pipeline::Create();
                m_LightingPipeline->Init(pipelineSpecs);
            }
            //----------------------------------------------------------------------------------------------------

        }

        m_MainCmdBuffer = CommandBuffer::Create();

        m_Fence = Fence::Create();
        m_Fence->Init();

        ImageAvailableSemaphore = Semaphore::Create();
        ImageAvailableSemaphore->Init();

        RenderFinishedSemaphore = Semaphore::Create();
        RenderFinishedSemaphore->Init();

        m_Queue = Queue::Create();

        m_MainCmdBufferAllocator = CommandBufferAllocator::Create();
        m_MainCmdBufferAllocator->Init();
        m_MainCmdBufferAllocator->Allocate(m_MainCmdBuffer);
    }

    void Renderer::SetupEntity(Entity e) {
        if (e.HasComponent<MeshComponent>()) {
            for (auto& subMesh : e.GetComponent<MeshComponent>().subMeshes) {
                uint32_t index = subMesh.material_index != -1 ? subMesh.material_index : 0;
                //materials[index]->Bind(m_MainCmdBuffer, m_Pipeline);

                //PushConstants pushConstantsData = {
                //    SceneManager::pActiveScene->mMaterials[index].baseColorTextureIndex,   // Index for albedo texture 0
                //    SceneManager::pActiveScene->mMaterials[index].metallicRoughnessTextureIndex, // Index for metallic texture1
                //    SceneManager::pActiveScene->mMaterials[index].normalTextureIndex,   // Index for normal texture 4
                //    SceneManager::pActiveScene->mMaterials[index].emissiveTextureIndex,// Index for roughness texture 2
                //    SceneManager::pActiveScene->mMaterials[index].occlusionTextureIndex// Index for emissive texture 3
                //};

                //pushConstantsData.camPos = Engine::s_Application->GetCamera()->GetPosition();

                //m_Pipeline->BindPushConstant(m_MainCmdBuffer, sizeof(PushConstants), &pushConstantsData);

                RenderCommand::DrawIndexed(m_MainCmdBuffer, subMesh.index_count, 1, subMesh.first_index, 0, 0);
            }
        }
        for (auto& child : e.GetComponent<TransformComponent>().children) {
            SetupEntity(child);
        }
    }

    void Renderer::PreRenderScene() {
        // Bind unbound resources
        if (!SceneManager::pActiveScene) return;

        auto parent = SceneManager::pActiveScene->Reg().view<RootComponent>();

        for (auto e : parent) {
            Entity entity = { e, SceneManager::pActiveScene.get() };
            SetupEntity(entity);
        }
    }

    void Renderer::RenderScene(float deltaTime)
    {
        if (!SceneManager::pActiveScene) return;

        //auto view = SceneManager::pActiveScene->Reg().view<MeshComponent, MaterialComponent>();
        auto parent = SceneManager::pActiveScene->Reg().view<RootComponent>();

        m_Fence->Wait();
        m_Fence->Reset();

        m_Swapchain->AquireNextImage(UINT64_MAX, ImageAvailableSemaphore, nullptr, &m_ImageIndex);
        m_MainCmdBuffer->Reset();
        m_MainCmdBuffer->Bind();
        //m_Pipeline->m_GraphicsSpecs.pRenderPass->Begin(m_MainCmdBuffer, m_ImageIndex);
        m_GBufferPipeline->Bind(m_MainCmdBuffer);

        m_GeometryBufferPass->Begin(m_MainCmdBuffer);

        RenderCommand::SetViewport(m_MainCmdBuffer, 0, 0, m_Swapchain->GetExtentWidth(), m_Swapchain->GetExtentHeight(), 0, 1);
        RenderCommand::SetScissor(m_MainCmdBuffer, 0, 0, m_Swapchain->GetExtentWidth(), m_Swapchain->GetExtentHeight());

        for (auto e : parent) {
            Entity entity = { e, SceneManager::pActiveScene.get() };
            
            //std::cout << entity.GetComponent<TagComponent>().Tag << std::endl;
            //auto& mesh = entity.GetComponent<MeshComponent>();
            auto& root = entity.GetComponent<RootComponent>();
            auto& mat = entity.GetComponent<MaterialComponent>();

            RenderCommand::BindVertexBuffer(m_MainCmdBuffer, { root.m_VertexBuffer }, 0);
            RenderCommand::BindIndexBuffer(m_MainCmdBuffer, root.m_IndexBuffer, 0);

            //mat.pMaterials[0]->Bind(m_MainCmdBuffer, m_Pipeline);

            //m_Pipeline->Bind(m_MainCmdBuffer);

            RenderEntity(entity);
        }

        m_GeometryBufferPass->End(m_MainCmdBuffer);
        //m_Pipeline->m_GraphicsSpecs.pRenderPass->End(m_MainCmdBuffer);
        m_MainCmdBuffer->UnBind();

        Queue::SubmitInfo submitInfo{};
        submitInfo.pWaitSemaphores.push_back(ImageAvailableSemaphore);
        submitInfo.pSignalSemaphores.push_back(RenderFinishedSemaphore);
        submitInfo.pWaitStages.push_back(Queue::WaitStage::PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        submitInfo.pCmdBuffers.push_back(m_MainCmdBuffer);

        // Submit m_Queue
        m_Queue->Submit(submitInfo, m_Fence);

        Queue::PresentInfo presentInfo{};
        presentInfo.pWaitSemaphores.push_back(RenderFinishedSemaphore);
        presentInfo.pSwapchains.push_back(m_Swapchain);
        presentInfo.pImageIndex = m_ImageIndex;

        // Present
        m_Queue->Present(presentInfo);
    }

    void Renderer::RenderEntity(Entity e) {
        if (e.HasComponent<MeshComponent>()) {
            for (auto& subMesh : e.GetComponent<MeshComponent>().subMeshes) {
                uint32_t index = subMesh.material_index != -1 ? subMesh.material_index : 0;
                //materials[index]->Bind(m_MainCmdBuffer, m_Pipeline);

                //PushConstants pushConstantsData = {
                //    SceneManager::pActiveScene->mMaterials[index].baseColorTextureIndex,   // Index for albedo texture 0
                //    SceneManager::pActiveScene->mMaterials[index].metallicRoughnessTextureIndex, // Index for metallic texture1
                //    SceneManager::pActiveScene->mMaterials[index].normalTextureIndex,   // Index for normal texture 4
                //    SceneManager::pActiveScene->mMaterials[index].emissiveTextureIndex,// Index for roughness texture 2
                //    SceneManager::pActiveScene->mMaterials[index].occlusionTextureIndex// Index for emissive texture 3
                //};

                //pushConstantsData.camPos = Engine::s_Application->GetCamera()->GetPosition();

                //m_Pipeline->BindPushConstant(m_MainCmdBuffer, sizeof(PushConstants), &pushConstantsData);

                RenderCommand::DrawIndexed(m_MainCmdBuffer, subMesh.index_count, 1, subMesh.first_index, 0, 0);
            }
        }
        for (auto& child : e.GetComponent<TransformComponent>().children) {
            RenderEntity(child);
        }
    }

    void Renderer::DrawNode(const std::shared_ptr<Mesh> model, std::vector<std::shared_ptr<Shader>> materials, GLTF_Node* node) {}

    std::unique_ptr<Renderer> Renderer::Create()
    {
        return std::make_unique<Renderer>();
    }
}