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

        m_LightsUBO = Buffer::Create();
        m_LightsUBO->Init(sizeof(LightsMVP), nullptr, Core::BufferUsage::UniformBuffer,
            Core::MemoryProperty::HostVisible | Core::MemoryProperty::HostCoherent, true);

        Engine::s_Application->GetGpuAdapter()->AddResource(GpuDescriptorResourceType::SceneLightsUBO, nullptr, m_LightsUBO, 0);

        glm::vec3 lightDir = glm::normalize(glm::vec3(0.0f, 0.0f, -10.0f)); // example
        float range = 10.0f;
        float distance = 200.0f;
        float nearPlane = 0.1f;
        float farPlane = 1000.0f;


        glm::mat4 lightView = glm::lookAt(
            -lightDir * distance,  // light position
            glm::vec3(0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)  // up vector
        );

        glm::mat4 lightProjection = glm::ortho(-range, range, -range, range, nearPlane, farPlane);

        LightsMVP mvp{};
        mvp.ViewProjection = lightProjection * lightView;
        mvp.Model = glm::mat4(1.0f); // Or your object model matrix if needed
        m_LightsUBO->UpdatePersistantData(sizeof(LightsMVP), &mvp);

        // Renderpasses
        {
            // Depth Pre pass
            //----------------------------------------------------------------------------------------------------
            m_DepthPre = Texture::Create();

            {
                Texture::TextureSpecification specs{};
                specs.p_Width = 1920;
                specs.p_Height = 1080;
                specs.p_Type = Texture::TextureType::TEXTURE2D;
                specs.p_DebugName = "g_Depth";
                specs.p_Usage = Core::TextureUsage::ImageUsageDepthStencilAttachment | Core::TextureUsage::ImageUsageSampled;
                specs.p_Format = Core::Format::FORMAT_D16_UNORM;
                specs.p_IsDepth = true;
                m_DepthPre->Init(specs);
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
                    RenderPassAttachment{ 0, AttachmentType::Depth, m_DepthPre  }
                }
            );

            // ShadowMap pass
            //----------------------------------------------------------------------------------------------------
            m_ShadowMap = Texture::Create();

            {
                Texture::TextureSpecification specs{};
                specs.p_Width = 1920;
                specs.p_Height = 1080;
                specs.p_Type = Texture::TextureType::TEXTURE2D;
                specs.p_DebugName = "g_Depth";
                specs.p_Usage = Core::TextureUsage::ImageUsageDepthStencilAttachment | Core::TextureUsage::ImageUsageSampled;
                specs.p_Format = Core::Format::FORMAT_D16_UNORM;
                specs.p_IsDepth = true;
                m_ShadowMap->Init(specs);
            }

            m_ShadowMapPass = RenderPass::Create();
            m_ShadowMapPass->Init(
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
                    RenderPassAttachment{ 0, AttachmentType::Depth, m_ShadowMap  }
                }
            );

            // Geometry pass
            //----------------------------------------------------------------------------------------------------
            m_Pos = Texture::Create();
            m_Normal = Texture::Create();
            m_Albedo = Texture::Create();
            m_Material = Texture::Create();
            m_Emissive = Texture::Create();
            m_Depth = Texture::Create();

            {
                Texture::TextureSpecification specs{};
                specs.p_Width = 1920;
                specs.p_Height = 1080;
                specs.p_Type = Texture::TextureType::TEXTURE2D;
                specs.p_DebugName = "g_Depth";
                specs.p_Usage = Core::TextureUsage::ImageUsageDepthStencilAttachment    ;
                specs.p_Format = Core::Format::FORMAT_D16_UNORM;
                specs.p_IsDepth = true;
                m_Depth->Init(specs);

                specs.p_IsDepth = false;
                specs.p_DebugName = "m_Pos";
                specs.p_Type = Texture::TextureType::TEXTURE2D;
                specs.p_Usage = Core::TextureUsage::ImageUsageColorAttachment | Core::TextureUsage::ImageUsageSampled;
                specs.p_Format = Core::Format::FORMAT_R16G16B16A16_SFLOAT;
                m_Pos->Init(specs);

                specs.p_DebugName = "m_Normal";
                m_Normal->Init(specs);

                specs.p_DebugName = "m_Albedo";
                specs.p_Format = Core::Format::FORMAT_R8G8B8A8_UNORM;
                m_Albedo->Init(specs);

                specs.p_DebugName = "m_Material";
                specs.p_Format = Core::Format::FORMAT_R8G8B8A8_UNORM;
                m_Material->Init(specs);

                specs.p_DebugName = "m_Emissive";
                specs.p_Format = Core::Format::FORMAT_R8G8B8A8_UNORM;
                m_Emissive->Init(specs);
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
                    RenderPassAttachment{ 0, AttachmentType::Color, m_Pos    },
                    RenderPassAttachment{ 1, AttachmentType::Color, m_Normal },
                    RenderPassAttachment{ 2, AttachmentType::Color, m_Albedo },
                    RenderPassAttachment{ 3, AttachmentType::Color, m_Material },
                    RenderPassAttachment{ 4, AttachmentType::Color, m_Emissive },
                    RenderPassAttachment{ 5, AttachmentType::Depth, m_Depth },
                }
            );

            // Lighting pass
            //----------------------------------------------------------------------------------------------------
            m_LightingOutput = Texture::Create();

            {
                Texture::TextureSpecification specs{};
                specs.p_Width = 1920;
                specs.p_Height = 1080;
                specs.p_DebugName = "g_Lighting";
                specs.p_Usage = Core::TextureUsage::ImageUsageColorAttachment | Core::TextureUsage::ImageUsageSampled;
                specs.p_Format = Core::Format::FORMAT_R8G8B8A8_UNORM;
                m_LightingOutput->Init(specs);
            }

            m_LightingPass = RenderPass::Create();
            m_LightingPass->Init(
                {
                    RenderPassDependency {
                        true, // external
                        Core::AccessType::ColorAttachmentWrite,  
                        Core::AccessType::ShaderRead,            
                        Core::PipelineStage::ColorAttachment,    
                        Core::PipelineStage::FragmentShader      
                    },

                    RenderPassDependency {
                        false,
                        Core::AccessType::ColorAttachmentWrite,  
                        Core::AccessType::ShaderRead,            
                        Core::PipelineStage::ColorAttachment,    
                        Core::PipelineStage::FragmentShader      
                    }
                },
                {  
                    RenderPassAttachment{ 0, AttachmentType::Color, m_LightingOutput }
                }
            );

            // UI pass
            //----------------------------------------------------------------------------------------------------
            m_UIPass = RenderPass::Create();
            m_UIPass->Init(
                {
                    RenderPassDependency {
                        true, // external
                        Core::AccessType::None,                                  
                        Core::AccessType::ColorAttachmentWrite,                  
                        Core::PipelineStage::BottomOfPipe,                       
                        Core::PipelineStage::ColorAttachment,                    
                    },

                    RenderPassDependency {
                        false,
                        Core::AccessType::ColorAttachmentWrite,                  
                        Core::AccessType::MemoryRead,                            
                        Core::PipelineStage::ColorAttachment,                    
                        Core::PipelineStage::BottomOfPipe,                       
                    }
                },
                {
                    RenderPassAttachment{ 0, AttachmentType::Swapchain, nullptr }
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
                vertexLayout.pStride = sizeof(MeshAsset::MeshData);
                vertexLayout.pAttributes = {
                    {0, 0, Core::Format::FORMAT_R32G32B32_SFLOAT, offsetof(MeshAsset::MeshData, MeshAsset::MeshData::Position)},
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

            // Shadow map pass pipeline
            //----------------------------------------------------------------------------------------------------
            {
                Pipeline::GraphicsPipelineSpecs pipelineSpecs{};
                Pipeline::VertexDataLayout vertexLayout;
                vertexLayout.pBinding = 0;
                vertexLayout.pStride = sizeof(MeshAsset::MeshData);
                vertexLayout.pAttributes = {
                    {0, 0, Core::Format::FORMAT_R32G32B32_SFLOAT, offsetof(MeshAsset::MeshData, MeshAsset::MeshData::Position)},
                };
                pipelineSpecs.pLayout = vertexLayout;
                pipelineSpecs.pRenderPass = m_ShadowMapPass;

                pipelineSpecs.pShaderPaths.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/ShadowMapPassVS.spv");
                pipelineSpecs.pShaderPaths.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/ShadowMapPassFS.spv");

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

                m_ShadowMapPipeline = Pipeline::Create();
                m_ShadowMapPipeline->Init(pipelineSpecs);
            }
            //----------------------------------------------------------------------------------------------------

            // Geometry pass pipeline
            //----------------------------------------------------------------------------------------------------
            {
                Pipeline::GraphicsPipelineSpecs pipelineSpecs{};
                Pipeline::VertexDataLayout vertexLayout;
                vertexLayout.pBinding = 0;
                vertexLayout.pStride = sizeof(MeshAsset::MeshData);
                vertexLayout.pAttributes = {
                    {0, 0, Core::Format::FORMAT_R32G32B32_SFLOAT, offsetof(MeshAsset::MeshData, MeshAsset::MeshData::Position)},
                    {0, 1, Core::Format::FORMAT_R32G32B32_SFLOAT, offsetof(MeshAsset::MeshData, MeshAsset::MeshData::Normal)},
                    {0, 2, Core::Format::FORMAT_R32G32_SFLOAT,    offsetof(MeshAsset::MeshData, MeshAsset::MeshData::UV0)},
                    //{0, 3, Core::Format::FORMAT_R32G32_SFLOAT,    offsetof(MeshAsset::MeshData, MeshAsset::MeshData::UV1)},
                    //{0, 4, Core::Format::FORMAT_R32G32B32_SFLOAT, offsetof(MeshAsset::MeshData, MeshAsset::MeshData::Color)},
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
                pipelineSpecs.pDepthTestEnable = true;
                pipelineSpecs.pDepthWriteEnable = true;
                pipelineSpecs.pCompareOp = Pipeline::COMPARE_OP_LESS;
                pipelineSpecs.pDepthBoundsTestEnable = false;
                pipelineSpecs.pStencilTestEnable = false;

                m_GBufferPipeline = Pipeline::Create();
                m_GBufferPipeline->Init(pipelineSpecs);

                pipelineSpecs.pCullMode = Pipeline::CullMode::NONE;
                m_GBufferDoubleSidedPipeline = Pipeline::Create();
                m_GBufferDoubleSidedPipeline->Init(pipelineSpecs);

                pipelineSpecs.pTransparent = true;
                m_GBufferAlphaBlendPipeline = Pipeline::Create();
                m_GBufferAlphaBlendPipeline->Init(pipelineSpecs);
            }
            //----------------------------------------------------------------------------------------------------

            // Lighting pass pipeline
            //----------------------------------------------------------------------------------------------------
            {
                Pipeline::GraphicsPipelineSpecs pipelineSpecs{};
                pipelineSpecs.pRenderPass = m_LightingPass;
                pipelineSpecs.pShaderPaths.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/LightingPassVS.spv");
                pipelineSpecs.pShaderPaths.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/LightingPassFS.spv");

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

                m_LightingPipeline = Pipeline::Create();
                m_LightingPipeline->Init(pipelineSpecs);
            }
            //----------------------------------------------------------------------------------------------------
        }

        Engine::s_Application->GetGpuAdapter()->AddResource(GpuDescriptorResourceType::DeferredTextures, m_Pos, nullptr, 0);
        Engine::s_Application->GetGpuAdapter()->AddResource(GpuDescriptorResourceType::DeferredTextures, m_Normal, nullptr, 1);
        Engine::s_Application->GetGpuAdapter()->AddResource(GpuDescriptorResourceType::DeferredTextures, m_Albedo, nullptr, 2);
        Engine::s_Application->GetGpuAdapter()->AddResource(GpuDescriptorResourceType::DeferredTextures, m_Material, nullptr, 3);
        Engine::s_Application->GetGpuAdapter()->AddResource(GpuDescriptorResourceType::DeferredTextures, m_Emissive, nullptr, 4);

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
        m_Editor->Create(m_UIPass, m_CmdBuffer, m_LightingOutput);
    }

    void Renderer::RenderScene(float deltaTime)
    {
        if (!SceneManager::pActiveScene) return;

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

            m_DepthPre->TransitionImageLayout(m_CmdBuffer, { params });
        }

        // --- DEPTH PRE PASS ---------------------------
        //------------------------------------------------------------------------------------------------------------------------------------------------
        m_DepthPrePass->Begin(m_CmdBuffer);
        m_DepthPrePassPipeline->Bind(m_CmdBuffer);

        RenderCommand::SetViewport(m_CmdBuffer, 0, 0, m_DepthPre->GetWidth(), m_DepthPre->GetHeight(), 0, 1);
        RenderCommand::SetScissor(m_CmdBuffer, 0, 0, m_DepthPre->GetWidth(), m_DepthPre->GetHeight());

        auto meshes = SceneManager::pActiveScene->Reg().view<MeshComponent, WorldTransformComponent>();

        for (auto e : meshes) {
            Entity entity = { e, SceneManager::pActiveScene.get() };

            auto& mesh = entity.GetComponent<MeshComponent>();
            auto& transform = entity.GetComponent<WorldTransformComponent>();

            Engine::s_Application->GetCamera()->SetMeshTransform(transform);

            RenderCommand::BindVertexBuffer(m_CmdBuffer, { mesh.p_Mesh->GetVertexBuffer() }, 0);
            RenderCommand::BindIndexBuffer(m_CmdBuffer, mesh.p_Mesh->GetIndexBuffer(), 0);

            RenderEntity(mesh, (int)fastgltf::AlphaMode::Opaque);
        }

        m_DepthPrePass->End(m_CmdBuffer);
        //------------------------------------------------------------------------------------------------------------------------------------------------

        {
            Brisk::Texture::ImageBarrierParams params{};
            params.oldLayout = Core::ImageLayout::Undefined;
            params.newLayout = Core::ImageLayout::DepthStencilAttachmentOptimal;
            params.srcAccess = Core::AccessType::None;
            params.dstAccess = Core::AccessType::DepthStencilWrite;
            params.srcStage = Core::PipelineStage::TopOfPipe;
            params.dstStage = Core::PipelineStage::EarlyFragmentTest;

            m_ShadowMap->TransitionImageLayout(m_CmdBuffer, { params });
        }

        // --- SHADOW MAP PASS ---------------------------
        //------------------------------------------------------------------------------------------------------------------------------------------------
        m_ShadowMapPass->Begin(m_CmdBuffer);
        m_ShadowMapPipeline->Bind(m_CmdBuffer);

        RenderCommand::SetViewport(m_CmdBuffer, 0, 0, m_ShadowMap->GetWidth(), m_ShadowMap->GetHeight(), 0, 1);
        RenderCommand::SetScissor(m_CmdBuffer, 0, 0, m_ShadowMap->GetWidth(), m_ShadowMap->GetHeight());

        for (auto e : meshes) {
            Entity entity = { e, SceneManager::pActiveScene.get() };

            auto& mesh = entity.GetComponent<MeshComponent>();
            auto& transform = entity.GetComponent<WorldTransformComponent>();

            Engine::s_Application->GetCamera()->SetMeshTransform(transform);

            RenderCommand::BindVertexBuffer(m_CmdBuffer, { mesh.p_Mesh->GetVertexBuffer() }, 0);
            RenderCommand::BindIndexBuffer(m_CmdBuffer, mesh.p_Mesh->GetIndexBuffer(), 0);

            RenderEntity(mesh, (int)fastgltf::AlphaMode::Opaque);
        }

        m_ShadowMapPass->End(m_CmdBuffer);
        //------------------------------------------------------------------------------------------------------------------------------------------------

        {
            Brisk::Texture::ImageBarrierParams params{};
            params.oldLayout = Core::ImageLayout::Undefined;
            params.newLayout = Core::ImageLayout::ColorAttachmentOptimal;
            params.srcAccess = Core::AccessType::None;
            params.dstAccess = Core::AccessType::ColorAttachmentWrite;
            params.srcStage = Core::PipelineStage::TopOfPipe;
            params.dstStage = Core::PipelineStage::ColorAttachment;

            m_Pos->TransitionImageLayout(m_CmdBuffer, { params });
        }

        {
            Brisk::Texture::ImageBarrierParams params{};
            params.oldLayout = Core::ImageLayout::Undefined;
            params.newLayout = Core::ImageLayout::ColorAttachmentOptimal;
            params.srcAccess = Core::AccessType::None;
            params.dstAccess = Core::AccessType::ColorAttachmentWrite;
            params.srcStage = Core::PipelineStage::TopOfPipe;
            params.dstStage = Core::PipelineStage::ColorAttachment;

            m_Normal->TransitionImageLayout(m_CmdBuffer, { params });
        }

        {
            Brisk::Texture::ImageBarrierParams params{};
            params.oldLayout = Core::ImageLayout::Undefined;
            params.newLayout = Core::ImageLayout::ColorAttachmentOptimal;
            params.srcAccess = Core::AccessType::None;
            params.dstAccess = Core::AccessType::ColorAttachmentWrite;
            params.srcStage = Core::PipelineStage::TopOfPipe;
            params.dstStage = Core::PipelineStage::ColorAttachment;

            m_Albedo->TransitionImageLayout(m_CmdBuffer, { params });
        }

        // --- GBUFFER PASS ---------------------------
        //------------------------------------------------------------------------------------------------------------------------------------------------
        m_GeometryBufferPass->Begin(m_CmdBuffer);
        RenderCommand::SetViewport(m_CmdBuffer, 0, 0, m_Pos->GetWidth(), m_Pos->GetHeight(), 0, 1);
        RenderCommand::SetScissor(m_CmdBuffer, 0, 0, m_Pos->GetWidth(), m_Pos->GetHeight());

        m_GBufferPipeline->Bind(m_CmdBuffer);
        for (const auto e : meshes) {
            Entity entity = { e, SceneManager::pActiveScene.get() };

            const auto& mesh = entity.GetComponent<MeshComponent>();
            auto& transform = entity.GetComponent<WorldTransformComponent>();

            Engine::s_Application->GetCamera()->SetMeshTransform(transform);

            RenderCommand::BindVertexBuffer(m_CmdBuffer, { mesh.p_Mesh->GetVertexBuffer() }, 0);
            RenderCommand::BindIndexBuffer(m_CmdBuffer, mesh.p_Mesh->GetIndexBuffer(), 0);

            RenderEntity(mesh, (int)fastgltf::AlphaMode::Opaque, true);
        }

        m_GBufferAlphaBlendPipeline->Bind(m_CmdBuffer);
        for (const auto e : meshes) {
            Entity entity = { e, SceneManager::pActiveScene.get() };

            const auto& mesh = entity.GetComponent<MeshComponent>();

            RenderCommand::BindVertexBuffer(m_CmdBuffer, { mesh.p_Mesh->GetVertexBuffer() }, 0);
            RenderCommand::BindIndexBuffer(m_CmdBuffer, mesh.p_Mesh->GetIndexBuffer(), 0);

            RenderEntity(mesh, (int)fastgltf::AlphaMode::Blend, true);
        }

        //m_GBufferAlphaBlendPipeline->Bind(m_CmdBuffer);
        //for (const auto e : meshes) {
        //    Entity entity = { e, SceneManager::pActiveScene.get() };

        //    const auto& mesh = entity.GetComponent<MeshComponent>();

        //    RenderCommand::BindVertexBuffer(m_CmdBuffer, { mesh.p_Mesh->GetVertexBuffer() }, 0);
        //    RenderCommand::BindIndexBuffer(m_CmdBuffer, mesh.p_Mesh->GetIndexBuffer(), 0);

        //    RenderEntity(mesh, (int)fastgltf::AlphaMode::Opaque);
        //}

        m_GeometryBufferPass->End(m_CmdBuffer);
        //------------------------------------------------------------------------------------------------------------------------------------------------

        m_Editor->Update();

        // --- LIGHTING PASS ---------------------------
        //------------------------------------------------------------------------------------------------------------------------------------------------
        {
            Brisk::Texture::ImageBarrierParams params{};
            params.oldLayout = Core::ImageLayout::Undefined;
            params.newLayout = Core::ImageLayout::ColorAttachmentOptimal;
            params.srcAccess = Core::AccessType::None;
            params.dstAccess = Core::AccessType::ColorAttachmentWrite;
            params.srcStage = Core::PipelineStage::TopOfPipe;
            params.dstStage = Core::PipelineStage::ColorAttachment;

            m_LightingOutput->TransitionImageLayout(m_CmdBuffer, { params });
        }

        m_LightingPass->Begin(m_CmdBuffer);
        m_LightingPipeline->Bind(m_CmdBuffer);

        RenderCommand::SetViewport(m_CmdBuffer, 0, 0, m_LightingOutput->GetWidth(), m_LightingOutput->GetHeight(), 0, 1);
        RenderCommand::SetScissor(m_CmdBuffer, 0, 0, m_LightingOutput->GetWidth(), m_LightingOutput->GetHeight());

        RenderCommand::Draw(m_CmdBuffer, 3, 0);

        m_LightingPass->End(m_CmdBuffer);
        //------------------------------------------------------------------------------------------------------------------------------------------------

        {
            Brisk::Texture::ImageBarrierParams params{};
            params.oldLayout = Core::ImageLayout::Undefined;
            params.newLayout = Core::ImageLayout::ColorAttachmentOptimal;
            params.srcAccess = Core::AccessType::None;
            params.dstAccess = Core::AccessType::ColorAttachmentWrite;
            params.srcStage = Core::PipelineStage::ColorAttachment;
            params.dstStage = Core::PipelineStage::ColorAttachment;

            m_Swapchain->TransitionCurrentImage(m_CmdBuffer, params, m_ImageIndex);
        }

        // --- UI PASS ---------------------------
        //------------------------------------------------------------------------------------------------------------------------------------------------
        m_UIPass->Begin(m_CmdBuffer, m_ImageIndex);

        RenderCommand::SetViewport(m_CmdBuffer, 0, 0, m_Swapchain->GetExtentWidth(), m_Swapchain->GetExtentHeight(), 0, 1);
        RenderCommand::SetScissor(m_CmdBuffer, 0, 0, m_Swapchain->GetExtentWidth(), m_Swapchain->GetExtentHeight());

        m_Editor->Render(m_CmdBuffer);

        m_UIPass->End(m_CmdBuffer);
        //------------------------------------------------------------------------------------------------------------------------------------------------

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

    void Renderer::RenderEntity(const MeshComponent& mesh, int alphaMode, bool push) {
        for (auto& subMesh : mesh.p_Mesh->m_Meshes) {
            for (auto& primitive : subMesh.primitives) {
                uint32_t index = primitive.materialIndex != -1 ? primitive.materialIndex : 0;

                if(primitive.materialIndex < 0)
                    BRISK_CORE_WARN("Invalid material index");

                if(push)
                    m_GBufferPipeline->BindPushConstant(m_CmdBuffer, sizeof(uint32_t), &index);

                if ((fastgltf::AlphaMode)mesh.p_Mesh->m_Materials[primitive.materialIndex].alphaMode == (fastgltf::AlphaMode)alphaMode) 
                {
                    RenderCommand::DrawIndexed(m_CmdBuffer, primitive.indexCount, 1, primitive.firstIndex, 0, 0);
                }
            }
        }
    }

    void Renderer::Release() {
        m_Pos->Release();
        m_Normal->Release();
        m_Albedo->Release();
        m_Material->Release();
        m_Emissive->Release();
        m_DepthPre->Release();
        m_Depth->Release();
        m_ShadowMap->Release();
        m_LightingOutput->Release();

        m_DepthPrePass->Release();
        m_ShadowMapPass->Release();
        m_GeometryBufferPass->Release();
        m_LightingPass->Release();
        m_UIPass->Release();

        m_DepthPrePassPipeline->Release();
        m_ShadowMapPipeline->Release();
        m_GBufferPipeline->Release();
        m_GBufferDoubleSidedPipeline->Release();
        m_GBufferAlphaBlendPipeline->Release();
        m_LightingPipeline->Release();

        m_Editor->Release();

        m_LightsUBO->Release();

        m_Swapchain->Release();

        ImageAvailableSemaphore->Release();
        RenderFinishedSemaphore->Release();

        m_Fence->Release();

    }

    std::unique_ptr<Renderer> Renderer::Create()
    {
        return std::make_unique<Renderer>();
    }
}
