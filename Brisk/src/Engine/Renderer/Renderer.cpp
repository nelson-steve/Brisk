// INCLUDES
#include "Renderer.hpp"
#include "RenderPass.hpp"
#include "RenderCommand.hpp"
#include "ComputeCommand.hpp"
#include "Engine/Component.hpp"
#include "Graphics/Factories/SwapchainFactory.hpp"
#include <random>
//------------------------------------------------

namespace Brisk
{
    std::shared_ptr<Swapchain> Renderer::m_Swapchain;

    std::vector<LightData> GenerateRandomLights(uint32_t count, float range = 10.0f) {
        std::vector<LightData> lights;
        lights.reserve(count);

        std::random_device rd;
        std::mt19937 rng(rd());

        std::uniform_real_distribution<float> posDist(-range, range);
        std::uniform_real_distribution<float> radiusDist(10.0f, 50.0f); // light radius
        std::uniform_real_distribution<float> colorDist(0.5f, 1.0f);  // bright colors
        std::uniform_real_distribution<float> intensityDist(1.0f, 5.0f); // intensity

        for (uint32_t i = 0; i < count; ++i) {
            glm::vec3 pos = glm::vec3(posDist(rng), posDist(rng), posDist(rng));
            float radius = radiusDist(rng);

            glm::vec3 color = glm::vec3(colorDist(rng), colorDist(rng), colorDist(rng));
            float intensity = intensityDist(rng);

            LightData light;
            light.position = glm::vec4(pos, radius);
            light.color = glm::vec4(color, intensity);

            lights.push_back(light);
        }

        return lights;
    }

    void Renderer::Init()
    {
        RenderCommand::s_RendererAPI = RendererAPI::Create();
        ComputeCommand::s_ComputeAPI = ComputeAPI::Create();

        m_Swapchain = SwapchainFactory::CreateSwapchain(Engine::s_Application->GetWindow());
        m_Swapchain->Create(Swapchain::DOUBLE_BUFFERING);

#ifdef DISABLED_CODE
        //m_LightsUBO = Buffer::Create();
        //m_LightsUBO->Init(sizeof(LightsMVP), nullptr, Core::BufferUsage::UniformBuffer,
        //    Core::MemoryProperty::HostVisible | Core::MemoryProperty::HostCoherent, true);

        //Engine::s_Application->GetGpuAdapter()->AddResource(GpuDescriptorResourceType::SceneLightsUBO, nullptr, m_LightsUBO, 0);

        //glm::vec3 lightDir = glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f));
        //float distance = 2000.0f;

        //glm::vec3 up = glm::abs(glm::dot(lightDir, glm::vec3(0, 1, 0))) > 0.99f
        //    ? glm::vec3(0, 0, 1)
        //    : glm::vec3(0, 1, 0);

        //glm::vec3 lightPos = -lightDir * distance;
        //glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), up);

        //float orthoRange = 500.0f;
        //float nearPlane = 0.1f;
        //float farPlane = 1000.0f;
        //glm::mat4 lightProj = glm::ortho(-orthoRange, orthoRange, -orthoRange, orthoRange, nearPlane, farPlane);

        //LightsMVP mvp{};    
        //mvp.ViewProjection = lightProj * lightView;
        //mvp.Model = glm::mat4(1.0f);
        //m_LightsUBO->UpdatePersistantData(sizeof(LightsMVP), &mvp);
#endif
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
                        -1,
                        0,
                        Core::AccessType::None, // src access
                        Core::AccessType::DepthStencilWrite, // dst access
                        Core::PipelineStage::BottomOfPipe, // src stage
                        Core::PipelineStage::EarlyFragmentTest // dst stage
                    },
                    RenderPassDependency {
                        0,
                        -1,
                        Core::AccessType::DepthStencilWrite, // src access
                        Core::AccessType::DepthStencilRead, // dst access
                        Core::PipelineStage::LateFragmentTest, // src stage
                        Core::PipelineStage::EarlyFragmentTest // dst stage
                    },
                },
                {   
                    RenderPassAttachment{ 0, AttachmentType::Depth, m_DepthPre, LoadOp::Clear, StoreOp::Store }
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
                        -1,
                        0,
                        Core::AccessType::None, // src access
                        Core::AccessType::DepthStencilWrite, // dst access
                        Core::PipelineStage::BottomOfPipe, // src stage
                        Core::PipelineStage::EarlyFragmentTest // dst stage
                    },
                },
                {
                    RenderPassAttachment{ 0, AttachmentType::Depth, m_ShadowMap, LoadOp::Clear, StoreOp::Store }
                }
            );

            // Geometry pass
            //----------------------------------------------------------------------------------------------------
            m_Pos = Texture::Create();
            m_Normal = Texture::Create();
            m_Albedo = Texture::Create();
            m_Material = Texture::Create();
            m_Emissive = Texture::Create();

            {
                Texture::TextureSpecification specs{};
                specs.p_Width = 1920;
                specs.p_Height = 1080;
                specs.p_Type = Texture::TextureType::TEXTURE2D;
                specs.p_DebugName = "g_Depth";
                specs.p_Usage = Core::TextureUsage::ImageUsageDepthStencilAttachment    ;
                specs.p_Format = Core::Format::FORMAT_D16_UNORM;
                specs.p_IsDepth = true;

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
                        -1,
                        0,
                        Core::AccessType::None, // src access
                        Core::AccessType::ColorAttachmentWrite, // dst access
                        Core::PipelineStage::ColorAttachment, // src stage
                        Core::PipelineStage::ColorAttachment // dst stage
                    },
                    RenderPassDependency {
                        0,
                        -1,
                        Core::AccessType::ColorAttachmentWrite, // src access
                        Core::AccessType::ShaderRead, // dst access
                        Core::PipelineStage::ColorAttachment, // src stage
                        Core::PipelineStage::FragmentShader // dst stage
                    },
                },
                {   
                    RenderPassAttachment{ 0, AttachmentType::Color, m_Pos,      LoadOp::Clear, StoreOp::Store    },
                    RenderPassAttachment{ 1, AttachmentType::Color, m_Normal,   LoadOp::Clear, StoreOp::Store    },
                    RenderPassAttachment{ 2, AttachmentType::Color, m_Albedo,   LoadOp::Clear, StoreOp::Store    },
                    RenderPassAttachment{ 3, AttachmentType::Color, m_Material, LoadOp::Clear, StoreOp::Store    },
                    RenderPassAttachment{ 4, AttachmentType::Color, m_Emissive, LoadOp::Clear, StoreOp::Store    },
                    RenderPassAttachment{ 5, AttachmentType::Depth, m_DepthPre, LoadOp::Load, StoreOp::DontCare  },
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
                        -1,
                        0,
                        Core::AccessType::None, // src access
                        Core::AccessType::ColorAttachmentWrite, // dst access
                        Core::PipelineStage::ColorAttachment, // src stage
                        Core::PipelineStage::ColorAttachment // dst stage
                    },
                    RenderPassDependency {
                        0,
                        -1,
                        Core::AccessType::ColorAttachmentWrite, // src access
                        Core::AccessType::ShaderRead, // dst access
                        Core::PipelineStage::ColorAttachment, // src stage
                        Core::PipelineStage::FragmentShader // dst stage
                    },
                },
                {
                    RenderPassAttachment{ 0, AttachmentType::Color, m_LightingOutput, LoadOp::Clear, StoreOp::Store }
                }
            );

            // UI pass
            //----------------------------------------------------------------------------------------------------
            m_UIPass = RenderPass::Create();
            m_UIPass->Init(
                {
                    RenderPassDependency {
                        -1,
                        0,
                        Core::AccessType::None, // src access
                        Core::AccessType::ColorAttachmentWrite, // dst access
                        Core::PipelineStage::ColorAttachment, // src stage
                        Core::PipelineStage::ColorAttachment // dst stage
                    },
                    RenderPassDependency {
                        -1,
                        0,
                        Core::AccessType::ColorAttachmentWrite,
                        Core::AccessType::ShaderRead, 
                        Core::PipelineStage::ColorAttachment,
                        Core::PipelineStage::FragmentShader
                    },
                },
                {
                    RenderPassAttachment{ 0, AttachmentType::Swapchain, nullptr, LoadOp::Clear, StoreOp::Store }
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

                pipelineSpecs.pShaderPathsVK.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/DepthPrePassVS.spv");
                pipelineSpecs.pShaderPathsVK.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/DepthPrePassFS.spv");
                pipelineSpecs.pShaderPathsDX.push_back("\\Shaders\\DirectX12\\DeferredRenderer\\Compiled\\DepthPrePass_vert.cso");
                pipelineSpecs.pShaderPathsDX.push_back("\\Shaders\\DirectX12\\DeferredRenderer\\Compiled\\DepthPrePass_frag.cso");

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
                pipelineSpecs.pDebugName = "DepthPrePas pipeline";

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

                pipelineSpecs.pShaderPathsVK.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/ShadowMapPassVS.spv");
                pipelineSpecs.pShaderPathsVK.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/ShadowMapPassFS.spv");
                pipelineSpecs.pShaderPathsDX.push_back("\\Shaders\\DirectX12\\DeferredRenderer\\Compiled\\DepthPrePass_vert.cso");
                pipelineSpecs.pShaderPathsDX.push_back("\\Shaders\\DirectX12\\DeferredRenderer\\Compiled\\DepthPrePass_frag.cso");

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
                pipelineSpecs.pDebugName = "ShadowMap pipeline";

                m_ShadowMapPipeline = Pipeline::Create();
                //m_ShadowMapPipeline->Init(pipelineSpecs);
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
                    {0, 0, Core::Format::FORMAT_R32G32B32_SFLOAT,    offsetof(MeshAsset::MeshData, MeshAsset::MeshData::Position)},
                    {0, 1, Core::Format::FORMAT_R32G32B32_SFLOAT,    offsetof(MeshAsset::MeshData, MeshAsset::MeshData::Normal)},
                    {0, 2, Core::Format::FORMAT_R32G32_SFLOAT,       offsetof(MeshAsset::MeshData, MeshAsset::MeshData::UV0)},
                    {0, 3, Core::Format::FORMAT_R32G32_SFLOAT,       offsetof(MeshAsset::MeshData, MeshAsset::MeshData::UV1)},
                    {0, 4, Core::Format::FORMAT_R32G32B32_SFLOAT,    offsetof(MeshAsset::MeshData, MeshAsset::MeshData::Color)},
                    {0, 5, Core::Format::FORMAT_R32G32B32A32_SFLOAT, offsetof(MeshAsset::MeshData, MeshAsset::MeshData::Tangent)},
                    {0, 6, Core::Format::FORMAT_R32G32B32A32_UINT,   offsetof(MeshAsset::MeshData, MeshAsset::MeshData::JointIndices)},
                    {0, 7, Core::Format::FORMAT_R32G32B32A32_SFLOAT, offsetof(MeshAsset::MeshData, MeshAsset::MeshData::JointWeights)},
                };
                pipelineSpecs.pLayout = vertexLayout;
                pipelineSpecs.pRenderPass = m_GeometryBufferPass;

                pipelineSpecs.pShaderPathsVK.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/GeometryPassVS.spv");
                pipelineSpecs.pShaderPathsVK.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/GeometryPassFS.spv");
                pipelineSpecs.pShaderPathsDX.push_back("\\Shaders\\DirectX12\\DeferredRenderer\\Compiled\\GeometryPass_vert.cso");
                pipelineSpecs.pShaderPathsDX.push_back("\\Shaders\\DirectX12\\DeferredRenderer\\Compiled\\GeometryPass_frag.cso");

                pipelineSpecs.pDepthClampEnable = false;
                pipelineSpecs.pRasterizationDiscardEnable = false;
                pipelineSpecs.pPolygoneMode = Pipeline::POLYGON_MODE_FILL;
                pipelineSpecs.pLineWidth = 1.0f;
                pipelineSpecs.pCullMode = Pipeline::CullMode::BACK;
                pipelineSpecs.pFrontFace = Pipeline::FrontFace::CLOCKWISE;
                pipelineSpecs.pDepthBiasEnable = false;
                pipelineSpecs.pDepthTestEnable = true;
                pipelineSpecs.pDepthWriteEnable = false;
                pipelineSpecs.pCompareOp = Pipeline::COMPARE_OP_EQUAL;
                pipelineSpecs.pDepthBoundsTestEnable = false;
                pipelineSpecs.pStencilTestEnable = false;
                pipelineSpecs.pDebugName = "GeometryPass pipeline";

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
                pipelineSpecs.pShaderPathsVK.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/LightingPassVS.spv");
                pipelineSpecs.pShaderPathsVK.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/LightingPassFS.spv");
                pipelineSpecs.pShaderPathsDX.push_back("\\Shaders\\DirectX12\\DeferredRenderer\\Compiled\\LightingPass_vert.cso");
                pipelineSpecs.pShaderPathsDX.push_back("\\Shaders\\DirectX12\\DeferredRenderer\\Compiled\\LightingPass_frag.cso");

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
                pipelineSpecs.pDebugName = "LightingPass pipeline";

                m_LightingPipeline = Pipeline::Create();
                m_LightingPipeline->Init(pipelineSpecs);
            }
            //----------------------------------------------------------------------------------------------------

            // -- Clustered lighting --
            // AAB Generator
            //----------------------------------------------------------------------------------------------------
            {
                Pipeline::ComputePipelineSpecs pipelineSpecs{};
                pipelineSpecs.pShaderPath = "Shaders/Vulkan/ClusteredLighting/Compiled/ClusterAABBGenerateCS.spv";

                m_AABBGeneratorPipeline = Pipeline::Create();
                m_AABBGeneratorPipeline->Init(pipelineSpecs);
            }
            //----------------------------------------------------------------------------------------------------

            // Assign lights to clusters
            //----------------------------------------------------------------------------------------------------
            {
                Pipeline::ComputePipelineSpecs pipelineSpecs{};
                pipelineSpecs.pShaderPath = "Shaders/Vulkan/ClusteredLighting/Compiled/AssignLightsToClustersCS.spv";

                m_AssignLightsToClustersPipeline = Pipeline::Create();
                m_AssignLightsToClustersPipeline->Init(pipelineSpecs);
            }
            //----------------------------------------------------------------------------------------------------
        }

        m_MVPBuffer = Buffer::Create();
        m_MVPBuffer->Init(sizeof(MVP), nullptr, Core::BufferUsage::UniformBuffer,
            Core::MemoryProperty::HostVisible | Core::MemoryProperty::HostCoherent, true);
        {
            m_ClusterTilesSSBO = Buffer::Create();
            m_ClusterTilesSSBO->Init(sizeof(TileAABB) * NUM_CLUSTERS, nullptr, Core::BufferUsage::StorageBuffer, Core::MemoryProperty::DeviceLocal, false);

            m_ClusterInfoUBO = Buffer::Create();
            m_ClusterInfoUBO->Init(sizeof(ClusterInfo), nullptr, Core::BufferUsage::UniformBuffer, Core::MemoryProperty::HostVisible | Core::MemoryProperty::HostCoherent, true);

            m_AABBGeneratorPipeline->UpdateResources("u_ClusterInfo", {}, m_ClusterInfoUBO);
            m_AABBGeneratorPipeline->UpdateResources("ssbo_ClusterAABB", {}, m_ClusterTilesSSBO);
        }

        {
            m_CameraData = Buffer::Create();
            m_CameraData->Init(sizeof(MVP), nullptr, Core::BufferUsage::UniformBuffer,
                Core::MemoryProperty::HostVisible | Core::MemoryProperty::HostCoherent, true);

            std::vector<LightData> lights = GenerateRandomLights(MAX_LIGHTS, 400);

            m_LightsList = Buffer::Create();
            m_LightsList->Init(sizeof(LightData) * lights.size(), lights.data(), Core::BufferUsage::StorageBuffer, Core::MemoryProperty::DeviceLocal, false);

            m_ClusterLightIndexList = Buffer::Create();
            m_ClusterLightIndexList->Init(sizeof(uint32_t) * NUM_CLUSTERS * MAX_LIGHTS_PER_CLUSTER, nullptr, Core::BufferUsage::StorageBuffer, Core::MemoryProperty::DeviceLocal, false);

            m_ClusterLightOffsetList = Buffer::Create();
            m_ClusterLightOffsetList->Init(sizeof(LightOffset) * NUM_CLUSTERS, nullptr, Core::BufferUsage::StorageBuffer, Core::MemoryProperty::DeviceLocal, false);

            m_AtomicCounters = Buffer::Create();
            m_AtomicCounters->Init(sizeof(uint32_t) * NUM_CLUSTERS, nullptr, Core::BufferUsage::StorageBuffer, Core::MemoryProperty::DeviceLocal, false);

            uint32_t globalIndex = 0;
            m_GlobalIndexCountSSBO = Buffer::Create();
            m_GlobalIndexCountSSBO->Init(sizeof(uint32_t), &globalIndex, Core::BufferUsage::StorageBuffer, Core::MemoryProperty::DeviceLocal, false);

            m_AssignLightsToClustersPipeline->UpdateResources("u_ClusterInfo", {}, m_ClusterInfoUBO);
            m_AssignLightsToClustersPipeline->UpdateResources("ssbo_LightsList", {}, m_LightsList);
            m_AssignLightsToClustersPipeline->UpdateResources("ssbo_ClusterAABB", {}, m_ClusterTilesSSBO);
            m_AssignLightsToClustersPipeline->UpdateResources("ssbo_ClusterLightIndexList", {}, m_ClusterLightIndexList);
            m_AssignLightsToClustersPipeline->UpdateResources("ssbo_ClusterLightOffsetList", {}, m_ClusterLightOffsetList);
            m_AssignLightsToClustersPipeline->UpdateResources("ssbo_AtomicCounters", {}, m_AtomicCounters);
            m_AssignLightsToClustersPipeline->UpdateResources("ssbo_GlobalIndex", {}, m_GlobalIndexCountSSBO);
        }

        m_DepthPrePassPipeline->UpdateResources("u_MVP", {}, m_MVPBuffer);

        m_LightingPipeline->UpdateResources("sampler_Position", { m_Pos      }, nullptr);
        m_LightingPipeline->UpdateResources("sampler_Normal",   { m_Normal   }, nullptr);
        m_LightingPipeline->UpdateResources("sampler_Albedo",   { m_Albedo   }, nullptr);
        m_LightingPipeline->UpdateResources("sampler_Material", { m_Material }, nullptr);
        m_LightingPipeline->UpdateResources("sampler_Emissive", { m_Emissive }, nullptr);
        m_LightingPipeline->UpdateResources("sampler_Depth",    { m_DepthPre }, nullptr);
        m_LightingPipeline->UpdateResources("ssbo_ClusterAABB", {}, m_ClusterTilesSSBO);
        m_LightingPipeline->UpdateResources("u_MVP",            {}, m_MVPBuffer);

        m_Fence = Fence::Create();
        m_Fence->Init();

        ImageAvailableSemaphore = Semaphore::Create();
        ImageAvailableSemaphore->Init();

        ClusteredTaskSemaphore = Semaphore::Create();
        ClusteredTaskSemaphore->Init();

        RenderFinishedSemaphore = Semaphore::Create();
        RenderFinishedSemaphore->Init();

        m_GraphicsQueue = Queue::Create();
        m_GraphicsQueue->Init(Queue::QueueType::Graphics);
        m_ComputeQueue = Queue::Create();
        m_ComputeQueue->Init(Queue::QueueType::Compute);

        m_CmdBuffer = CommandBuffer::Create();
        m_CmdBuffer->Allocate(CommandBuffer::PoolType::Graphics);
        m_ClusteredCmdBuffer = CommandBuffer::Create();
        m_ClusteredCmdBuffer->Allocate(CommandBuffer::PoolType::Graphics);

        m_Editor = std::make_shared<Editor>();
        m_Editor->Create(m_UIPass, m_CmdBuffer, m_LightingOutput);
    }

    void Renderer::RenderScene(float deltaTime)
    {
        if (!SceneManager::pActiveScene) return;

        MVP mvp{};
        mvp.ProjView = Engine::s_Application->GetCamera()->GetViewProjection();
        //mvp.View = glm::inverse( Engine::s_Application->GetCamera()->GetViewMatrix());
        mvp.View = Engine::s_Application->GetCamera()->GetViewMatrix();
        mvp.CamPos = Engine::s_Application->GetCamera()->GetPosition();

        m_MVPBuffer->UpdatePersistantData(sizeof(MVP), &mvp);

        ClusterInfo clusterInfo{};
        clusterInfo.View = Engine::s_Application->GetCamera()->GetViewMatrix();
        clusterInfo.InverseProj = glm::inverse(Engine::s_Application->GetCamera()->GetProjection());
        clusterInfo.TileSizes = glm::uvec4(16, 9, 24, 0);
        clusterInfo.ScreenDimensions = glm::uvec2(1920, 1080);
        clusterInfo.zNear = 0.1f;
        clusterInfo.zFar = 1000.0f;
        clusterInfo.numLights = MAX_LIGHTS;
        m_ClusterInfoUBO->UpdatePersistantData(sizeof(ClusterInfo), &clusterInfo);

        auto view = SceneManager::pActiveScene->Reg().view<MeshComponent, WorldTransformComponent>();

        for (auto e : view) {
            Entity entity = { e, SceneManager::pActiveScene.get() };
            auto& meshComp = entity.GetComponent<MeshComponent>();
            m_RenderGroups[meshComp.p_Mesh.get()].push_back(entity);
        }

        m_Fence->Wait();
        m_Fence->Reset();

        m_ClusteredCmdBuffer->Reset();

        m_ClusteredCmdBuffer->Bind();
        // --- CLUSTERS AABB GENERATOR COMPUTE TASK ---------------------------
        //------------------------------------------------------------------------------------------------------------------------------------------------
        m_AABBGeneratorPipeline->Bind(m_ClusteredCmdBuffer);
        ComputeCommand::CmdDispatch(m_ClusteredCmdBuffer, 16, 9, 24);
        m_ClusterTilesSSBO->MemoryPipelineBarrier(m_ClusteredCmdBuffer,
            {
                Core::ImageLayout::Undefined,
                Core::ImageLayout::Undefined,
                Core::AccessType::ShaderWrite,
                Core::AccessType::ShaderWrite,
                Core::PipelineStage::ComputeShader,
                Core::PipelineStage::ComputeShader,
                Core::ImageAspectFlags::Color,
                0,
                1,
                0,
                0
            });
        //------------------------------------------------------------------------------------------------------------------------------------------------
        m_ClusteredCmdBuffer->UnBind();

        Queue::SubmitInfo clusteredSubmitInfo{};
        clusteredSubmitInfo.pCmdBuffers.push_back(m_ClusteredCmdBuffer);
        m_GraphicsQueue->Submit(clusteredSubmitInfo, nullptr);
        Engine::s_Application->GetGpuAdapter()->WaitIdle();

        m_ClusteredCmdBuffer->Reset();

        m_ClusteredCmdBuffer->Bind();
        // --- ASSIGN LIGHTS TO CLUSTERS COMPUTE TASK ---------------------------
        //------------------------------------------------------------------------------------------------------------------------------------------------
        m_AssignLightsToClustersPipeline->Bind(m_ClusteredCmdBuffer);
        ComputeCommand::CmdDispatch(m_ClusteredCmdBuffer, 3456, 1, 1);
        m_ClusterTilesSSBO->MemoryPipelineBarrier(m_ClusteredCmdBuffer,
            {
                Core::ImageLayout::Undefined,
                Core::ImageLayout::Undefined,
                Core::AccessType::ShaderWrite,
                Core::AccessType::ShaderRead,
                Core::PipelineStage::ComputeShader,
                Core::PipelineStage::ComputeShader,
                Core::ImageAspectFlags::Color,
                0,
                1,
                0,
                0
            });
        m_ClusterLightOffsetList->MemoryPipelineBarrier(m_ClusteredCmdBuffer,
            {
                Core::ImageLayout::Undefined,
                Core::ImageLayout::Undefined,
                Core::AccessType::ShaderWrite,
                Core::AccessType::ShaderRead,
                Core::PipelineStage::ComputeShader,
                Core::PipelineStage::ComputeShader,
                Core::ImageAspectFlags::Color,
                0,
                1,
                0,
                0
            });
        m_ClusterLightIndexList->MemoryPipelineBarrier(m_ClusteredCmdBuffer,
            {
                Core::ImageLayout::Undefined,
                Core::ImageLayout::Undefined,
                Core::AccessType::ShaderWrite,
                Core::AccessType::ShaderRead,
                Core::PipelineStage::ComputeShader,
                Core::PipelineStage::ComputeShader,
                Core::ImageAspectFlags::Color,
                0,
                1,
                0,
                0
            });
        //------------------------------------------------------------------------------------------------------------------------------------------------
        m_ClusteredCmdBuffer->UnBind();

        Queue::SubmitInfo clusteredSubmitInfo2{};
        clusteredSubmitInfo2.pCmdBuffers.push_back(m_ClusteredCmdBuffer);
        m_GraphicsQueue->Submit(clusteredSubmitInfo2, nullptr);
        Engine::s_Application->GetGpuAdapter()->WaitIdle();

        m_Swapchain->AquireNextImage(UINT64_MAX, ImageAvailableSemaphore, nullptr, &m_ImageIndex);
        m_CmdBuffer->Reset();
        m_CmdBuffer->Bind();

        // --- SHADOW MAP PASS ---------------------------
        //------------------------------------------------------------------------------------------------------------------------------------------------
        //m_ShadowMapPass->Begin(m_CmdBuffer);
        //m_ShadowMapPipeline->Bind(m_CmdBuffer);

        //RenderCommand::SetViewport(m_CmdBuffer, 0, 0, m_ShadowMap->GetWidth(), m_ShadowMap->GetHeight(), 0, 1);
        //RenderCommand::SetScissor(m_CmdBuffer, 0, 0, m_ShadowMap->GetWidth(), m_ShadowMap->GetHeight());

        //Render(false);

        //m_ShadowMapPass->End(m_CmdBuffer);
        // 
        //------------------------------------------------------------------------------------------------------------------------------------------------

        // --- DEPTH PRE PASS ---------------------------
        //------------------------------------------------------------------------------------------------------------------------------------------------
        m_DepthPrePass->Begin(m_CmdBuffer);
        m_DepthPrePassPipeline->Bind(m_CmdBuffer);

        RenderCommand::SetViewport(m_CmdBuffer, 0, 0, m_DepthPre->GetWidth(), m_DepthPre->GetHeight(), 0, 1);
        RenderCommand::SetScissor(m_CmdBuffer, 0, 0, m_DepthPre->GetWidth(), m_DepthPre->GetHeight());

        auto meshes = SceneManager::pActiveScene->Reg().view<MeshComponent, WorldTransformComponent>();

        for (auto& [mesh, entities] : m_RenderGroups) {
            Render(mesh, entities, true, true);
        }

        m_DepthPrePass->End(m_CmdBuffer);
        //------------------------------------------------------------------------------------------------------------------------------------------------

        // --- GBUFFER PASS ---------------------------
        //------------------------------------------------------------------------------------------------------------------------------------------------
        m_GeometryBufferPass->Begin(m_CmdBuffer);
        RenderCommand::SetViewport(m_CmdBuffer, 0, 0, m_Pos->GetWidth(), m_Pos->GetHeight(), 0, 1);
        RenderCommand::SetScissor(m_CmdBuffer, 0, 0, m_Pos->GetWidth(), m_Pos->GetHeight());

        for (auto& [mesh, entities] : m_RenderGroups) {
            m_GBufferPipeline->UpdateResources("ssbo_Materials", {}, mesh->m_MaterialStorageBuffer);
            m_GBufferPipeline->Bind(m_CmdBuffer);
            Render(mesh, entities, true, true);
        }
        m_GeometryBufferPass->End(m_CmdBuffer);
        //------------------------------------------------------------------------------------------------------------------------------------------------

        m_Editor->Update();

        Texture::ImageBarrierParams params{};
        params.oldLayout = Core::ImageLayout::DepthStencilAttachmentOptimal;
        params.newLayout = Core::ImageLayout::ShaderReadOnlyOptimal;
        params.srcAccess = Core::AccessType::DepthStencilWrite;
        params.dstAccess = Core::AccessType::ShaderRead;
        params.srcStage = Core::PipelineStage::LateFragmentTest;
        params.dstStage = Core::PipelineStage::FragmentShader;

        m_DepthPre->TransitionImageLayout(m_CmdBuffer, { params });

        // --- LIGHTING PASS ---------------------------
        //------------------------------------------------------------------------------------------------------------------------------------------------
        m_LightingPass->Begin(m_CmdBuffer);
        m_LightingPipeline->Bind(m_CmdBuffer);

        RenderCommand::SetViewport(m_CmdBuffer, 0, 0, m_LightingOutput->GetWidth(), m_LightingOutput->GetHeight(), 0, 1);
        RenderCommand::SetScissor(m_CmdBuffer, 0, 0, m_LightingOutput->GetWidth(), m_LightingOutput->GetHeight());

        RenderCommand::Draw(m_CmdBuffer, 3, 0);

        m_LightingPass->End(m_CmdBuffer);
        //------------------------------------------------------------------------------------------------------------------------------------------------

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

        Engine::s_Application->GetGpuAdapter()->WaitIdle();

        m_RenderGroups.clear();
    }

    glm::mat4 GetWorldTransform(Entity entity) {
        glm::mat4 local = entity.GetComponent<WorldTransformComponent>().GetTransform();

        if (entity.HasComponent<ParentComponent>()) {
            Entity parent = entity.GetComponent<ParentComponent>().parent;
            return GetWorldTransform(parent) * local;
        }

        return local;
    }

    void Renderer::Render(MeshAsset* mesh, std::vector<Entity> entities, bool pushMaterialIndex, bool pushModelMatrix) {
        RenderCommand::BindVertexBuffer(m_CmdBuffer, { mesh->GetVertexBuffer() }, 0);
        RenderCommand::BindIndexBuffer(m_CmdBuffer, mesh->GetIndexBuffer(), 0);

        for (Entity e : entities) {
            auto& meshComp = e.GetComponent<MeshComponent>();
            auto& transform = e.GetComponent<WorldTransformComponent>();

            glm::mat4 t = GetWorldTransform(e);

            auto& submesh = mesh->m_Meshes[meshComp.p_SubMeshIndex];
            for (auto& primitive : submesh.primitives) {
                uint32_t index = primitive.materialIndex != -1 ? primitive.materialIndex : 0;

                struct pcData {
                    glm::mat4 model;
                    uint32_t index;
                } pc;

                pc.index = primitive.materialIndex;
                pc.model = t;

                if (primitive.materialIndex < 0)
                    BRISK_CORE_WARN("Invalid material index");

                if (pushMaterialIndex && pushModelMatrix)
                    m_GBufferPipeline->BindPushConstant(m_CmdBuffer, sizeof(glm::mat4) + sizeof(uint32_t), &pc, 0, true);

                if ((fastgltf::AlphaMode)mesh->m_Materials[primitive.materialIndex].alphaMode == (fastgltf::AlphaMode)0U)
                {
                    RenderCommand::DrawIndexed(m_CmdBuffer, primitive.indexCount, 1, primitive.firstIndex, 0, 0);
                }
            }
        }
    }

    void Renderer::RenderEntity(const MeshComponent& mesh, int alphaMode, bool push) {
        for (auto& subMesh : mesh.p_Mesh->m_Meshes) {
            for (auto& primitive : subMesh.primitives) {
                uint32_t index = primitive.materialIndex != -1 ? primitive.materialIndex : 0;

                if(primitive.materialIndex < 0)
                    BRISK_CORE_WARN("Invalid material index");

                //if(push)
                //    m_GBufferPipeline->BindPushConstant(m_CmdBuffer, sizeof(uint32_t), &index, false);

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
