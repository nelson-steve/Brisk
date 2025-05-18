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
            //----------------------------------------------------------------------------------------------------
            g_Pos = Texture::Create();
            g_Normal = Texture::Create();
            g_Albedo = Texture::Create();
            g_Depth = Texture::Create();

            {
                Texture::TextureSpecification specs{};
                specs.p_Width = 1920;
                specs.p_Height = 1080;
                specs.p_IsDepth = false;
                specs.p_DebugName = "g_Pos";
                specs.p_Type = Texture::TextureType::TEXTURE2D;
                specs.p_Usage = Texture::TextureUsage::ImageUsageColorAttachment | Texture::TextureUsage::ImageUsageSampled;
                specs.p_Format = Core::Format::FORMAT_R16G16B16A16_SFLOAT;
                g_Pos->Init(specs);

                specs.p_DebugName = "g_Normal";
                g_Normal->Init(specs);

                specs.p_DebugName = "g_Albedo";
                specs.p_Format = Core::Format::FORMAT_R8G8B8A8_UNORM;
                g_Albedo->Init(specs);

                specs.p_DebugName = "g_Depth";
                specs.p_Usage = Texture::TextureUsage::ImageUsageDepthStencilAttachment | Texture::TextureUsage::ImageUsageSampled;
                specs.p_Format = Core::Format::FORMAT_D16_UNORM;
                specs.p_IsDepth = true;
                g_Depth->Init(specs);
            }

            // Geometry pass
            //----------------------------------------------------------------------------------------------------
            m_GeometryBufferPass = RenderPass::Create();
            m_GeometryBufferPass->Init(
                {},
                {   
                    RenderPassAttachment{ 0, AttachmentType::Color, g_Pos    },
                    RenderPassAttachment{ 1, AttachmentType::Color, g_Normal },
                    RenderPassAttachment{ 2, AttachmentType::Color, g_Albedo },
                    RenderPassAttachment{ 3, AttachmentType::Depth, g_Depth  } 
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
                specs.p_Usage = Texture::TextureUsage::ImageUsageColorAttachment | Texture::TextureUsage::ImageUsageTransferSrc | Texture::TextureUsage::ImageUsageSampled;
                specs.p_Format = Core::Format::FORMAT_R8G8B8A8_UNORM;
                g_lightingOutput->Init(specs);
            }

            m_LightingPass = RenderPass::Create();
            m_LightingPass->Init(
                {},
                {  
                    RenderPassAttachment{ 0, AttachmentType::Color, g_lightingOutput } 
                }
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
                    //{0, 3, Core::Format::FORMAT_R32G32_SFLOAT,    offsetof(MeshData, MeshData::UV1)},
                    //{0, 4, Core::Format::FORMAT_R32G32B32_SFLOAT, offsetof(MeshData, MeshData::Color)},
                };
                pipelineSpecs.pLayout = vertexLayout;
                pipelineSpecs.pRenderPass = m_GeometryBufferPass;

                pipelineSpecs.p_ResourceTypes.push_back(GpuDescriptorResourceType::MVPUBO);
                pipelineSpecs.p_ResourceTypes.push_back(GpuDescriptorResourceType::BindlessTextures);

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
                pipelineSpecs.pRenderPass = m_LightingPass;
                pipelineSpecs.p_ResourceTypes.push_back(GpuDescriptorResourceType::DeferredTextures);
                pipelineSpecs.pShaderModules.push_back(vertexShaderModule);
                pipelineSpecs.pShaderModules.push_back(fragmentShaderModule);
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

        m_GBufferCmdBuffer = CommandBuffer::Create();
        m_LightingCmdBuffer = CommandBuffer::Create();

        m_Fence = Fence::Create();
        m_Fence->Init();

        ImageAvailableSemaphore = Semaphore::Create();
        ImageAvailableSemaphore->Init();        
        
        DeferredRenderingFinishedSemaphore = Semaphore::Create();
        DeferredRenderingFinishedSemaphore->Init();

        RenderFinishedSemaphore = Semaphore::Create();
        RenderFinishedSemaphore->Init();

        m_Queue = Queue::Create();

        m_MainCmdBufferAllocator = CommandBufferAllocator::Create();
        m_MainCmdBufferAllocator->Init();
        m_MainCmdBufferAllocator->Allocate(m_GBufferCmdBuffer);
        m_MainCmdBufferAllocator->Allocate(m_LightingCmdBuffer);
    }

    void Renderer::SetupEntity(Entity e) {
        if (e.HasComponent<MeshComponent>()) {
            for (auto& subMesh : e.GetComponent<MeshComponent>().subMeshes) {
                uint32_t index = subMesh.material_index != -1 ? subMesh.material_index : 0;

                RenderCommand::DrawIndexed(m_GBufferCmdBuffer, subMesh.index_count, 1, subMesh.first_index, 0, 0);
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
        m_GBufferCmdBuffer->Reset();
        m_GBufferCmdBuffer->Bind();

        // --------------------------------------------
        {
            Brisk::Texture::ImageBarrierParams params{};
            params.oldLayout = Texture::ImageLayout::Undefined;
            params.newLayout = Texture::ImageLayout::ColorAttachmentOptimal;
            params.srcAccess = Texture::AccessType::None;
            params.dstAccess = Texture::AccessType::ColorAttachmentWrite;
            params.srcStage = Texture::PipelineStage::TopOfPipe;
            params.dstStage = Texture::PipelineStage::ColorAttachment;

            g_Pos->TransitionImageLayout(m_GBufferCmdBuffer, { params });
        }

        {
            Brisk::Texture::ImageBarrierParams params{};
            params.oldLayout = Texture::ImageLayout::Undefined;
            params.newLayout = Texture::ImageLayout::ColorAttachmentOptimal;
            params.srcAccess = Texture::AccessType::None;
            params.dstAccess = Texture::AccessType::ColorAttachmentWrite;
            params.srcStage = Texture::PipelineStage::TopOfPipe;
            params.dstStage = Texture::PipelineStage::ColorAttachment;

            g_Normal->TransitionImageLayout(m_GBufferCmdBuffer, { params });
        }

        {
            Brisk::Texture::ImageBarrierParams params{};
            params.oldLayout = Texture::ImageLayout::Undefined;
            params.newLayout = Texture::ImageLayout::ColorAttachmentOptimal;
            params.srcAccess = Texture::AccessType::None;
            params.dstAccess = Texture::AccessType::ColorAttachmentWrite;
            params.srcStage = Texture::PipelineStage::TopOfPipe;
            params.dstStage = Texture::PipelineStage::ColorAttachment;

            g_Albedo->TransitionImageLayout(m_GBufferCmdBuffer, { params });
        }

        {
            Brisk::Texture::ImageBarrierParams params{};
            params.oldLayout = Texture::ImageLayout::Undefined;
            params.newLayout = Texture::ImageLayout::DepthStencilAttachmentOptimal;
            params.srcAccess = Texture::AccessType::None;
            params.dstAccess = Texture::AccessType::DepthStencilWrite;
            params.srcStage = Texture::PipelineStage::TopOfPipe;
            params.dstStage = Texture::PipelineStage::EarlyFragmentTest;

            g_Depth->TransitionImageLayout(m_GBufferCmdBuffer, { params });
        }

        m_GBufferPipeline->Bind(m_GBufferCmdBuffer);

        m_GeometryBufferPass->Begin(m_GBufferCmdBuffer);

        RenderCommand::SetViewport(m_GBufferCmdBuffer, 0, 0, m_Swapchain->GetExtentWidth(), m_Swapchain->GetExtentHeight(), 0, 1);
        RenderCommand::SetScissor(m_GBufferCmdBuffer, 0, 0, m_Swapchain->GetExtentWidth(), m_Swapchain->GetExtentHeight());

        for (auto e : parent) {
            Entity entity = { e, SceneManager::pActiveScene.get() };
            
            auto& mesh = entity.GetComponent<MeshComponent>();
            auto& root = entity.GetComponent<RootComponent>();

            RenderCommand::BindVertexBuffer(m_GBufferCmdBuffer, { root.m_VertexBuffer }, 0);
            RenderCommand::BindIndexBuffer(m_GBufferCmdBuffer, root.m_IndexBuffer, 0);

            RenderEntity(entity);
        }

        m_GeometryBufferPass->End(m_GBufferCmdBuffer);
        m_GBufferCmdBuffer->UnBind();


        // --- LIGHTING PASS ---------------------------

        m_LightingCmdBuffer->Reset();
        m_LightingCmdBuffer->Bind();

        //// --------------------------------------------
        {
            Brisk::Texture::ImageBarrierParams params{};
            params.oldLayout = Texture::ImageLayout::Undefined;
            params.newLayout = Texture::ImageLayout::ColorAttachmentOptimal;
            params.srcAccess = Texture::AccessType::None;
            params.dstAccess = Texture::AccessType::ColorAttachmentWrite;
            params.srcStage = Texture::PipelineStage::TopOfPipe;
            params.dstStage = Texture::PipelineStage::ColorAttachment;

            g_lightingOutput->TransitionImageLayout(m_LightingCmdBuffer, { params });
        }

        m_LightingPass->Begin(m_LightingCmdBuffer);
        m_LightingPipeline->Bind(m_LightingCmdBuffer);

        RenderCommand::SetViewport(m_LightingCmdBuffer, 0, 0, m_Swapchain->GetExtentWidth(), m_Swapchain->GetExtentHeight(), 0, 1);
        RenderCommand::SetScissor(m_LightingCmdBuffer, 0, 0, m_Swapchain->GetExtentWidth(), m_Swapchain->GetExtentHeight());

        RenderCommand::Draw(m_LightingCmdBuffer, 3, 0);

        m_LightingPass->End(m_LightingCmdBuffer);

        if (times < 2) {
            times++;
            Brisk::Texture::ImageBarrierParams params{};
            params.oldLayout = Texture::ImageLayout::Undefined;
            params.newLayout = Texture::ImageLayout::TransferDst;
            params.srcAccess = Texture::AccessType::None;
            params.dstAccess = Texture::AccessType::TransferWrite;
            params.srcStage = Texture::PipelineStage::BottomOfPipe;
            params.dstStage = Texture::PipelineStage::TransferStage;

            m_Swapchain->TransitionCurrentImage(m_LightingCmdBuffer, params, m_ImageIndex);
        }
        else {
            // Prepare to be writeable
            {
                Brisk::Texture::ImageBarrierParams params{};
                params.oldLayout = Texture::ImageLayout::PresentSrc;
                params.newLayout = Texture::ImageLayout::TransferDst;
                params.srcAccess = Texture::AccessType::MemoryRead;
                params.dstAccess = Texture::AccessType::TransferWrite;
                params.srcStage = Texture::PipelineStage::BottomOfPipe;
                params.dstStage = Texture::PipelineStage::TransferStage;

                m_Swapchain->TransitionCurrentImage(m_LightingCmdBuffer, params, m_ImageIndex);
            }
        }

        // Blit the lighting output to swapchain image
        m_Swapchain->Blit(m_LightingCmdBuffer, g_lightingOutput, m_ImageIndex);

        // Transition lighting output back to color attachment for rendering
        {
            Brisk::Texture::ImageBarrierParams params{};
            params.oldLayout = Texture::ImageLayout::TransferSrc;
            params.newLayout = Texture::ImageLayout::ColorAttachmentOptimal;
            params.srcAccess = Texture::AccessType::TransferRead;
            params.dstAccess = Texture::AccessType::None;
            params.srcStage = Texture::PipelineStage::TransferStage;
            params.dstStage = Texture::PipelineStage::BottomOfPipe;

            g_lightingOutput->TransitionImageLayout(m_LightingCmdBuffer, { params });
        }

        // Prepare to be presentable
        {
            Brisk::Texture::ImageBarrierParams params{};
            params.oldLayout = Texture::ImageLayout::TransferDst;
            params.newLayout = Texture::ImageLayout::PresentSrc;
            params.srcAccess = Texture::AccessType::TransferWrite;
            params.dstAccess = Texture::AccessType::MemoryRead;
            params.srcStage = Texture::PipelineStage::TransferStage;
            params.dstStage = Texture::PipelineStage::BottomOfPipe;

            m_Swapchain->TransitionCurrentImage(m_LightingCmdBuffer, params, m_ImageIndex);
        }

        m_LightingCmdBuffer->UnBind();

        Queue::SubmitInfo deferredSubmitInfo{};
        deferredSubmitInfo.pSignalSemaphores.push_back(DeferredRenderingFinishedSemaphore);
        deferredSubmitInfo.pWaitStages.push_back(Queue::WaitStage::PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        deferredSubmitInfo.pCmdBuffers.push_back(m_GBufferCmdBuffer);

        m_Queue->Submit(deferredSubmitInfo, nullptr);

        Queue::SubmitInfo lightingSubmitInfo{};
        lightingSubmitInfo.pWaitSemaphores.push_back(ImageAvailableSemaphore);
        lightingSubmitInfo.pWaitSemaphores.push_back(DeferredRenderingFinishedSemaphore);
        lightingSubmitInfo.pSignalSemaphores.push_back(RenderFinishedSemaphore);
        lightingSubmitInfo.pWaitStages.push_back(Queue::WaitStage::PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        lightingSubmitInfo.pWaitStages.push_back(Queue::WaitStage::PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        lightingSubmitInfo.pCmdBuffers.push_back(m_LightingCmdBuffer);

        m_Queue->Submit(lightingSubmitInfo, m_Fence);

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

                RenderCommand::DrawIndexed(m_GBufferCmdBuffer, subMesh.index_count, 1, subMesh.first_index, 0, 0);
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
