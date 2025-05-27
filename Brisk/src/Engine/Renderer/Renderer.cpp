// INCLUDES
#include "Renderer.hpp"
#include "RenderPass.hpp"
#include "RenderCommand.hpp"
#include "ComputeCommand.hpp"
#include "Engine/Component.hpp"
#include "Graphics/Factories/SwapchainFactory.hpp"
//------------------------------------------------

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
            // Depth Pre pass
            //----------------------------------------------------------------------------------------------------
            g_Depth = Texture::Create();

            {
                Texture::TextureSpecification specs{};
                specs.p_Width = 1920;
                specs.p_Height = 1080;
                specs.p_Type = Texture::TextureType::TEXTURE2D;
                specs.p_DebugName = "g_Depth";
                specs.p_Usage = Core::TextureUsage::ImageUsageDepthStencilAttachment | Core::TextureUsage::ImageUsageSampled;
                specs.p_Format = Core::Format::FORMAT_D16_UNORM;
                specs.p_IsDepth = true;
                g_Depth->Init(specs);
            }

            m_DepthPrePass = RenderPass::Create();
            m_DepthPrePass->Init(
                {
                    RenderPassDependency {
                        true,
                        Core::AccessType::None, // src access
                        Core::AccessType::ColorAttachmentWrite | Core::AccessType::DepthStencilWrite, // dst access
                        Core::PipelineStage::FragmentShader | Core::PipelineStage::EarlyFragmentTest, // src stage
                        Core::PipelineStage::ColorAttachment | Core::PipelineStage::EarlyFragmentTest // dst stage
                    },
                    RenderPassDependency {
                        false,
                        Core::AccessType::DepthStencilWrite,
                        Core::AccessType::ShaderRead,
                        Core::PipelineStage::LateFragmentTest,
                        Core::PipelineStage::FragmentShader
                    }
                },
                {   
                    RenderPassAttachment{ 0, AttachmentType::Depth, g_Depth  } 
                }
            );

            // Geometry pass
            //----------------------------------------------------------------------------------------------------
            g_Pos = Texture::Create();
            g_Normal = Texture::Create();
            g_Albedo = Texture::Create();

            {
                Texture::TextureSpecification specs{};
                specs.p_Width = 1920;
                specs.p_Height = 1080;
                specs.p_IsDepth = false;
                specs.p_DebugName = "g_Pos";
                specs.p_Type = Texture::TextureType::TEXTURE2D;
                specs.p_Usage = Core::TextureUsage::ImageUsageColorAttachment | Core::TextureUsage::ImageUsageSampled;
                specs.p_Format = Core::Format::FORMAT_R16G16B16A16_SFLOAT;
                g_Pos->Init(specs);

                specs.p_DebugName = "g_Normal";
                g_Normal->Init(specs);

                specs.p_DebugName = "g_Albedo";
                specs.p_Format = Core::Format::FORMAT_R8G8B8A8_UNORM;
                g_Albedo->Init(specs);
            }

            m_GeometryBufferPass = RenderPass::Create();
            m_GeometryBufferPass->Init(
                {
                    RenderPassDependency {
                        true,
                        Core::AccessType::None, // src access
                        Core::AccessType::ColorAttachmentWrite | Core::AccessType::DepthStencilWrite, // dst access
                        Core::PipelineStage::FragmentShader | Core::PipelineStage::EarlyFragmentTest, // src stage
                        Core::PipelineStage::ColorAttachment | Core::PipelineStage::EarlyFragmentTest // dst stage
                    },
                    RenderPassDependency {
                        false,
                        Core::AccessType::ColorAttachmentWrite,
                        Core::AccessType::ShaderRead,
                        Core::PipelineStage::ColorAttachment,
                        Core::PipelineStage::FragmentShader
                    },                
                },
                {   
                    RenderPassAttachment{ 0, AttachmentType::Color, g_Pos    },
                    RenderPassAttachment{ 1, AttachmentType::Color, g_Normal },
                    RenderPassAttachment{ 2, AttachmentType::Color, g_Albedo },
                }
            );

            // Lighting pass
            //----------------------------------------------------------------------------------------------------
            g_lightingOutput = Texture::Create();

            {
                Texture::TextureSpecification specs{};
                specs.p_Width = 1920;
                specs.p_Height = 1080;
                specs.p_DebugName = "g_Lighting";
                specs.p_Usage = Core::TextureUsage::ImageUsageColorAttachment | Core::TextureUsage::ImageUsageTransferSrc | Core::TextureUsage::ImageUsageSampled;
                specs.p_Format = Core::Format::FORMAT_R8G8B8A8_UNORM;
                g_lightingOutput->Init(specs);
            }

            m_LightingPass = RenderPass::Create();
            m_LightingPass->Init(
                {
                    RenderPassDependency {
                        true,
                        Core::AccessType::None, // src access
                        Core::AccessType::ColorAttachmentWrite | Core::AccessType::DepthStencilWrite, // dst access
                        Core::PipelineStage::FragmentShader | Core::PipelineStage::EarlyFragmentTest, // src stage
                        Core::PipelineStage::ColorAttachment | Core::PipelineStage::EarlyFragmentTest // dst stage
                    },
                    RenderPassDependency {
                        true,
                        Core::AccessType::ColorAttachmentWrite, // src access
                        Core::AccessType::ShaderRead, // dst access
                        Core::PipelineStage::ColorAttachment, // src stage
                        Core::PipelineStage::FragmentShader// dst stage
                    },
                    RenderPassDependency {
                        false,
                        Core::AccessType::ColorAttachmentWrite,
                        Core::AccessType::TransferRead,
                        Core::PipelineStage::ColorAttachment,
                        Core::PipelineStage::TransferStage
                    },
                },
                {  
                    RenderPassAttachment{ 0, AttachmentType::Color, g_lightingOutput } 
                }
            );
        }

        // Pipelines
        {
            // Depth pre pass pipeline
            //----------------------------------------------------------------------------------------------------
            {
                Pipeline::GraphicsPipelineSpecs pipelineSpecs{};
                Pipeline::VertexDataLayout vertexLayout;
                vertexLayout.pBinding = 0;
                vertexLayout.pStride = sizeof(MeshData);
                vertexLayout.pAttributes = {
                    {0, 0, Core::Format::FORMAT_R32G32B32_SFLOAT, offsetof(MeshData, MeshData::Position)},
                };
                pipelineSpecs.pLayout = vertexLayout;
                pipelineSpecs.pRenderPass = m_DepthPrePass;

                pipelineSpecs.pShaderPaths.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/DepthPrePassVS.spv");
                pipelineSpecs.pShaderPaths.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/DepthPrePassFS.spv");

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

                m_DepthPrePassPipeline = Pipeline::Create();
                m_DepthPrePassPipeline->Init(pipelineSpecs);
            }
            //----------------------------------------------------------------------------------------------------

            // Geometry pass pipeline
            //----------------------------------------------------------------------------------------------------
            {
                Pipeline::GraphicsPipelineSpecs pipelineSpecs{};
                Pipeline::VertexDataLayout vertexLayout;
                vertexLayout.pBinding = 0;
                vertexLayout.pStride = sizeof(MeshData);
                vertexLayout.pAttributes = {
                    {0, 0, Core::Format::FORMAT_R32G32B32_SFLOAT, offsetof(MeshData, MeshData::Position)},
                    {0, 1, Core::Format::FORMAT_R32G32B32_SFLOAT, offsetof(MeshData, MeshData::Normal)},
                    {0, 2, Core::Format::FORMAT_R32G32_SFLOAT,    offsetof(MeshData, MeshData::UV0)},
                    //{0, 3, Core::Format::FORMAT_R32G32_SFLOAT,    offsetof(MeshData, MeshData::UV1)},
                    //{0, 4, Core::Format::FORMAT_R32G32B32_SFLOAT, offsetof(MeshData, MeshData::Color)},
                };
                pipelineSpecs.pLayout = vertexLayout;
                pipelineSpecs.pRenderPass = m_GeometryBufferPass;

                pipelineSpecs.pShaderPaths.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/GeometryPassVS.spv");
                pipelineSpecs.pShaderPaths.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/GeometryPassFS.spv");

                pipelineSpecs.pDepthClampEnable = false;
                pipelineSpecs.pRasterizationDiscardEnable = false;
                pipelineSpecs.pPolygoneMode = Pipeline::POLYGON_MODE_FILL;
                pipelineSpecs.pLineWidth = 1.0f;
                pipelineSpecs.pCullMode = Pipeline::CullMode::BACK;
                pipelineSpecs.pFrontFace = Pipeline::FrontFace::CLOCKWISE;
                pipelineSpecs.pDepthBiasEnable = false;
                pipelineSpecs.pDepthTestEnable = false;
                pipelineSpecs.pDepthWriteEnable = false;
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
                Pipeline::GraphicsPipelineSpecs pipelineSpecs{};
                pipelineSpecs.pRenderPass = m_LightingPass;
                pipelineSpecs.pShaderPaths.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/LightingVS.spv");
                pipelineSpecs.pShaderPaths.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/LightingFS.spv");
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

                m_LightingPipeline = Pipeline::Create();
                m_LightingPipeline->Init(pipelineSpecs);
            }
            //----------------------------------------------------------------------------------------------------
        }

        Engine::s_Application->GetGpuAdapter()->AddResource(GpuDescriptorResourceType::DeferredTextures, g_Pos, nullptr, 0);
        Engine::s_Application->GetGpuAdapter()->AddResource(GpuDescriptorResourceType::DeferredTextures, g_Normal, nullptr, 1);
        Engine::s_Application->GetGpuAdapter()->AddResource(GpuDescriptorResourceType::DeferredTextures, g_Albedo, nullptr, 2);

        m_Fence = Fence::Create();
        m_Fence->Init();

        ImageAvailableSemaphore = Semaphore::Create();
        ImageAvailableSemaphore->Init();

        RenderFinishedSemaphore = Semaphore::Create();
        RenderFinishedSemaphore->Init();

        m_GraphicsQueue = Queue::Create();
        m_GraphicsQueue->Init(Queue::QueueType::Graphics);

        m_CmdBuffer = CommandBuffer::Create();
        m_CmdBuffer->Allocate();

        m_Editor = std::make_shared<Editor>();
        m_Editor->Create(m_LightingPass, m_CmdBuffer);
    }

    void Renderer::SetupEntity(Entity e) {
        if (e.HasComponent<MeshComponent>()) {
            for (auto& subMesh : e.GetComponent<MeshComponent>().subMeshes) {
                uint32_t index = subMesh.material_index != -1 ? subMesh.material_index : 0;

                RenderCommand::DrawIndexed(m_CmdBuffer, subMesh.index_count, 1, subMesh.first_index, 0, 0);
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

    int times = 0;
    void Renderer::RenderScene(float deltaTime)
    {
        if (!SceneManager::pActiveScene) return;

        auto parent = SceneManager::pActiveScene->Reg().view<RootComponent>();

        m_Fence->Wait();
        m_Fence->Reset();

        m_Swapchain->AquireNextImage(UINT64_MAX, ImageAvailableSemaphore, nullptr, &m_ImageIndex);
        m_CmdBuffer->Reset();
        m_CmdBuffer->Bind();

        {
            Brisk::Texture::ImageBarrierParams params{};
            params.oldLayout = Core::ImageLayout::Undefined;
            params.newLayout = Core::ImageLayout::DepthStencilAttachmentOptimal;
            params.srcAccess = Core::AccessType::None;
            params.dstAccess = Core::AccessType::DepthStencilWrite;
            params.srcStage = Core::PipelineStage::TopOfPipe;
            params.dstStage = Core::PipelineStage::EarlyFragmentTest;

            g_Depth->TransitionImageLayout(m_CmdBuffer, { params });
        }

        m_DepthPrePassPipeline->Bind(m_CmdBuffer);
        m_DepthPrePass->Begin(m_CmdBuffer);

        RenderCommand::SetViewport(m_CmdBuffer, 0, 0, m_Swapchain->GetExtentWidth(), m_Swapchain->GetExtentHeight(), 0, 1);
        RenderCommand::SetScissor(m_CmdBuffer, 0, 0, m_Swapchain->GetExtentWidth(), m_Swapchain->GetExtentHeight());

        for (auto e : parent) {
            Entity entity = { e, SceneManager::pActiveScene.get() };

            auto& mesh = entity.GetComponent<MeshComponent>();
            auto& root = entity.GetComponent<RootComponent>();

            RenderCommand::BindVertexBuffer(m_CmdBuffer, { root.m_VertexBuffer }, 0);
            RenderCommand::BindIndexBuffer(m_CmdBuffer, root.m_IndexBuffer, 0);

            RenderEntity(entity);
        }

        m_DepthPrePass->End(m_CmdBuffer);

        // --------------------------------------------
        {
            Brisk::Texture::ImageBarrierParams params{};
            params.oldLayout = Core::ImageLayout::Undefined;
            params.newLayout = Core::ImageLayout::ColorAttachmentOptimal;
            params.srcAccess = Core::AccessType::None;
            params.dstAccess = Core::AccessType::ColorAttachmentWrite;
            params.srcStage = Core::PipelineStage::TopOfPipe;
            params.dstStage = Core::PipelineStage::ColorAttachment;

            g_Pos->TransitionImageLayout(m_CmdBuffer, { params });
        }

        {
            Brisk::Texture::ImageBarrierParams params{};
            params.oldLayout = Core::ImageLayout::Undefined;
            params.newLayout = Core::ImageLayout::ColorAttachmentOptimal;
            params.srcAccess = Core::AccessType::None;
            params.dstAccess = Core::AccessType::ColorAttachmentWrite;
            params.srcStage = Core::PipelineStage::TopOfPipe;
            params.dstStage = Core::PipelineStage::ColorAttachment;

            g_Normal->TransitionImageLayout(m_CmdBuffer, { params });
        }

        {
            Brisk::Texture::ImageBarrierParams params{};
            params.oldLayout = Core::ImageLayout::Undefined;
            params.newLayout = Core::ImageLayout::ColorAttachmentOptimal;
            params.srcAccess = Core::AccessType::None;
            params.dstAccess = Core::AccessType::ColorAttachmentWrite;
            params.srcStage = Core::PipelineStage::TopOfPipe;
            params.dstStage = Core::PipelineStage::ColorAttachment;

            g_Albedo->TransitionImageLayout(m_CmdBuffer, { params });
        }

        m_GBufferPipeline->Bind(m_CmdBuffer);
        m_GeometryBufferPass->Begin(m_CmdBuffer);

        RenderCommand::SetViewport(m_CmdBuffer, 0, 0, m_Swapchain->GetExtentWidth(), m_Swapchain->GetExtentHeight(), 0, 1);
        RenderCommand::SetScissor(m_CmdBuffer, 0, 0, m_Swapchain->GetExtentWidth(), m_Swapchain->GetExtentHeight());

        for (auto e : parent) {
            Entity entity = { e, SceneManager::pActiveScene.get() };
            
            auto& mesh = entity.GetComponent<MeshComponent>();
            auto& root = entity.GetComponent<RootComponent>();

            RenderCommand::BindVertexBuffer(m_CmdBuffer, { root.m_VertexBuffer }, 0);
            RenderCommand::BindIndexBuffer(m_CmdBuffer, root.m_IndexBuffer, 0);

            RenderEntity(entity);
        }

        m_GeometryBufferPass->End(m_CmdBuffer);


        m_Editor->Update();

        // --- LIGHTING PASS ---------------------------

        //// --------------------------------------------
        {
            Brisk::Texture::ImageBarrierParams params{};
            params.oldLayout = Core::ImageLayout::Undefined;
            params.newLayout = Core::ImageLayout::ColorAttachmentOptimal;
            params.srcAccess = Core::AccessType::None;
            params.dstAccess = Core::AccessType::ColorAttachmentWrite;
            params.srcStage = Core::PipelineStage::TopOfPipe;
            params.dstStage = Core::PipelineStage::ColorAttachment;

            g_lightingOutput->TransitionImageLayout(m_CmdBuffer, { params });
        }

        m_LightingPass->Begin(m_CmdBuffer);
        m_LightingPipeline->Bind(m_CmdBuffer);

        RenderCommand::SetViewport(m_CmdBuffer, 0, 0, m_Swapchain->GetExtentWidth(), m_Swapchain->GetExtentHeight(), 0, 1);
        RenderCommand::SetScissor(m_CmdBuffer, 0, 0, m_Swapchain->GetExtentWidth(), m_Swapchain->GetExtentHeight());

        RenderCommand::Draw(m_CmdBuffer, 3, 0);

        m_Editor->Render(m_CmdBuffer);

        m_LightingPass->End(m_CmdBuffer);

        if (times < 2) {
            times++;
            Brisk::Texture::ImageBarrierParams params{};
            params.oldLayout = Core::ImageLayout::Undefined;
            params.newLayout = Core::ImageLayout::TransferDst;
            params.srcAccess = Core::AccessType::None;
            params.dstAccess = Core::AccessType::TransferWrite;
            params.srcStage = Core::PipelineStage::BottomOfPipe;
            params.dstStage = Core::PipelineStage::TransferStage;

            m_Swapchain->TransitionCurrentImage(m_CmdBuffer, params, m_ImageIndex);
        }
        else {
            // Prepare to be writeable
            {
                Brisk::Texture::ImageBarrierParams params{};
                params.oldLayout = Core::ImageLayout::PresentSrc;
                params.newLayout = Core::ImageLayout::TransferDst;
                params.srcAccess = Core::AccessType::MemoryRead;
                params.dstAccess = Core::AccessType::TransferWrite;
                params.srcStage = Core::PipelineStage::BottomOfPipe;
                params.dstStage = Core::PipelineStage::TransferStage;

                m_Swapchain->TransitionCurrentImage(m_CmdBuffer, params, m_ImageIndex);
            }
        }

        // Blit the lighting output to swapchain image
        m_Swapchain->Blit(m_CmdBuffer, g_lightingOutput, m_ImageIndex);

        // Transition lighting output back to color attachment for rendering
        {
            Brisk::Texture::ImageBarrierParams params{};
            params.oldLayout = Core::ImageLayout::TransferSrc;
            params.newLayout = Core::ImageLayout::ColorAttachmentOptimal;
            params.srcAccess = Core::AccessType::TransferRead;
            params.dstAccess = Core::AccessType::None;
            params.srcStage = Core::PipelineStage::TransferStage;
            params.dstStage = Core::PipelineStage::BottomOfPipe;

            g_lightingOutput->TransitionImageLayout(m_CmdBuffer, { params });
        }

        // Prepare to be presentable
        {
            Brisk::Texture::ImageBarrierParams params{};
            params.oldLayout = Core::ImageLayout::TransferDst;
            params.newLayout = Core::ImageLayout::PresentSrc;
            params.srcAccess = Core::AccessType::TransferWrite;
            params.dstAccess = Core::AccessType::MemoryRead;
            params.srcStage = Core::PipelineStage::TransferStage;
            params.dstStage = Core::PipelineStage::BottomOfPipe;

            m_Swapchain->TransitionCurrentImage(m_CmdBuffer, params, m_ImageIndex);
        }

        m_CmdBuffer->UnBind();

        Queue::SubmitInfo lightingSubmitInfo{};
        lightingSubmitInfo.pWaitSemaphores.push_back(ImageAvailableSemaphore);
        lightingSubmitInfo.pSignalSemaphores.push_back(RenderFinishedSemaphore);
         lightingSubmitInfo.pWaitStages.push_back(Queue::WaitStage::PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        lightingSubmitInfo.pCmdBuffers.push_back(m_CmdBuffer);

        m_GraphicsQueue->Submit(lightingSubmitInfo, m_Fence);

        Queue::PresentInfo presentInfo{};
        presentInfo.pWaitSemaphores.push_back(RenderFinishedSemaphore);
        presentInfo.pSwapchains.push_back(m_Swapchain);
        presentInfo.pImageIndex = m_ImageIndex;

        // Present
        m_GraphicsQueue->Present(presentInfo);
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

                RenderCommand::DrawIndexed(m_CmdBuffer, subMesh.index_count, 1, subMesh.first_index, 0, 0);
            }
        }
        for (auto& child : e.GetComponent<TransformComponent>().children) {
            RenderEntity(child);
        }
    }

    std::unique_ptr<Renderer> Renderer::Create()
    {
        return std::make_unique<Renderer>();
    }
}
