// INCLUDES
#include "Renderer.hpp"
#include "RenderPass.hpp"
#include "RenderCommand.hpp"
#include "ComputeCommand.hpp"
#include "Engine/Component.hpp"
#include "Graphics/Factories/SwapchainFactory.hpp"
//------------------------------------------------
#include <fastgltf/types.hpp>

#define NUM_CASCADES 4

namespace Brisk
{
    float farPlane = 1000.0f;
    std::vector<float> shadowCascadeLevels{ farPlane / 50.0f, farPlane / 25.0f, farPlane / 10.0f, farPlane / 2.0f };

    std::shared_ptr<Swapchain> Renderer::m_Swapchain;

    //std::vector<glm::vec4> GetFrustumCornersWorld(
    //    const glm::mat4& proj,
    //    const glm::mat4& view,
    //    float splitNear,
    //    float splitFar)
    //{
    //    std::vector<glm::vec4> corners;
    //    corners.reserve(8);

    //    // We'll override the near/far planes of the projection
    //    glm::mat4 projCopy = proj;

    //    // This assumes a perspective matrix created with glm::perspective
    //    float fovy = 2.0f * atan(1.0f / proj[1][1]);
    //    float aspect = proj[1][1] / proj[0][0];

    //    projCopy = glm::perspectiveZO(fovy, aspect, splitNear, splitFar);
    //    projCopy[1][1] *= -1.0f;

    //    // Inverse of (proj * view) gives NDC -> world
    //    glm::mat4 inv = glm::inverse(projCopy * view);

    //    // 8 corners of the clip space cube
    //    std::vector<glm::vec4> ndcCorners = {
    //        {-1, -1, -1, 1},
    //        { 1, -1, -1, 1},
    //        { 1,  1, -1, 1},
    //        {-1,  1, -1, 1},
    //        {-1, -1,  1, 1},
    //        { 1, -1,  1, 1},
    //        { 1,  1,  1, 1},
    //        {-1,  1,  1, 1}
    //    };

    //    for (auto& c : ndcCorners) {
    //        glm::vec4 world = inv * c;
    //        world /= world.w;
    //        corners.push_back(world);
    //    }

    //    return corners;
    //}

    //glm::mat4 CalculateCascadeMatrix(
    //    int cascadeIndex, int cascadeCount,
    //    float nearPlane, float farPlane,
    //    float lambda,
    //    const glm::mat4& cameraProj, const glm::mat4& cameraView,
    //    const glm::vec3& lightDir)
    //{
    //    // --- Split depth (cascade range) ---
    //    float n = nearPlane;
    //    float f = farPlane;

    //    float si = (float)cascadeIndex / cascadeCount;
    //    float si1 = (float)(cascadeIndex + 1) / cascadeCount;

    //    float logSplitNear = n * pow(f / n, si);
    //    float logSplitFar = n * pow(f / n, si1);

    //    float uniSplitNear = n + (f - n) * si;
    //    float uniSplitFar = n + (f - n) * si1;

    //    float splitNear = lambda * logSplitNear + (1.0f - lambda) * uniSplitNear;
    //    float splitFar = lambda * logSplitFar + (1.0f - lambda) * uniSplitFar;

    //    // --- Get frustum corners in world space ---
    //    std::vector<glm::vec4> frustumCorners = GetFrustumCornersWorld(cameraProj, cameraView, splitNear, splitFar);

    //    // --- Light view ---
    //    glm::vec3 center(0.0f);
    //    for (auto& c : frustumCorners) center += glm::vec3(c);
    //    center /= frustumCorners.size();

    //    float cascadeDepth = glm::length(frustumCorners[0] - frustumCorners[6]);
    //    glm::mat4 lightView = glm::lookAt(center - lightDir * cascadeDepth, center, glm::vec3(0, 1, 0));


    //    // --- Transform corners to light space ---
    //    glm::vec3 min(FLT_MAX), max(-FLT_MAX);
    //    for (auto& c : frustumCorners) {
    //        glm::vec4 tr = lightView * c;
    //        min = glm::min(min, glm::vec3(tr));
    //        max = glm::max(max, glm::vec3(tr));
    //    }

    //    glm::mat4 lightProj = glm::orthoZO(min.x, max.x, min.y, max.y, min.z, max.z);
    //    lightProj[1][1] *= -1.0f;

    //    return lightProj * lightView;
    //}

    std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& projview)
    {
        const auto inv = glm::inverse(projview);

        std::vector<glm::vec4> frustumCorners;
        for (unsigned int x = 0; x < 2; ++x)
        {
            for (unsigned int y = 0; y < 2; ++y)
            {
                for (unsigned int z = 0; z < 2; ++z)
                {
                    const glm::vec4 pt = inv * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, z, 1.0f);
                    frustumCorners.push_back(pt / pt.w);
                }
            }
        }

        return frustumCorners;
    }

    std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view)
    {
        return getFrustumCornersWorldSpace(proj * view);
    }

    bool onlyOnce = true;
    glm::mat4 proj;
    glm::mat4 view;
    glm::mat4 getLightSpaceMatrix(const float nearPlane, const float farPlane, glm::vec3 lightDir)
    {
        auto proj = glm::perspectiveZO(
            glm::radians(45.0f), (float)1920 / (float)1080, nearPlane,
            farPlane);
        proj[1][1] *= -1.0f;
        view = Application::GetCamera()->GetViewMatrix();
        onlyOnce = false;

        const auto corners = getFrustumCornersWorldSpace(proj, view);

        glm::vec3 center = glm::vec3(0, 0, 0);
        for (const auto& v : corners)
        {
            center += glm::vec3(v);
        }
        center /= corners.size();

        const auto lightView = glm::lookAt(center + lightDir, center, glm::vec3(0.0f, 1.0f, 0.0f));

        float minX = std::numeric_limits<float>::max();
        float maxX = std::numeric_limits<float>::lowest();
        float minY = std::numeric_limits<float>::max();
        float maxY = std::numeric_limits<float>::lowest();
        float minZ = std::numeric_limits<float>::max();
        float maxZ = std::numeric_limits<float>::lowest();
        for (const auto& v : corners)
        {
            const auto trf = lightView * v;
            minX = std::min(minX, trf.x);
            maxX = std::max(maxX, trf.x);
            minY = std::min(minY, trf.y);
            maxY = std::max(maxY, trf.y);
            minZ = std::min(minZ, trf.z);
            maxZ = std::max(maxZ, trf.z);
        }

        // Tune this parameter according to the scene
        constexpr float zMult = 30.0f;
        if (minZ < 0)
        {
            minZ *= zMult;
        }
        else
        {
            minZ /= zMult;
        }
        if (maxZ < 0)
        {
            maxZ /= zMult;
        }
        else
        {
            maxZ *= zMult;
        }

        glm::mat4 lightProjection = glm::orthoZO(minX, maxX, minY, maxY, minZ, maxZ);
        lightProjection[1][1] *= -1.0f;
        return lightProjection * lightView;
    }

    std::vector<glm::mat4> getLightSpaceMatrices(glm::vec3 lightDir)
    {
        std::vector<glm::mat4> ret;
        for (size_t i = 0; i < shadowCascadeLevels.size(); ++i)
        {
            if (i == 0)
            {
                ret.push_back(getLightSpaceMatrix(1.0f, shadowCascadeLevels[i], lightDir));
            }
            else if (i < shadowCascadeLevels.size() - 1)
            {
                ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1], shadowCascadeLevels[i], lightDir));
            }
            else
            {
                ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1], farPlane, lightDir));
            }
        }
        return ret;
    }

    void Renderer::Init()
    {
        m_SunMatrices.resize(NUM_CASCADES);

        glm::vec3 probMinBounds = glm::vec3(-20, -10, -20);
        glm::vec3 probMaxBounds = glm::vec3(-20, -10, -20);
        glm::ivec3 probeResolution = glm::vec3(16, 8, 16);
        uint32_t probeCount = probeResolution.x * probeResolution.y * probeResolution.z;

        glm::vec3 probeSpacing = (probMaxBounds - probMinBounds) / glm::vec3(probeResolution);
        glm::vec3 probeOrigin = probMinBounds;

        for (int z = 0; z < probeResolution.z; ++z) {
            for (int y = 0; y < probeResolution.z; ++y) {
                for (int x = 0; x < probeResolution.z; ++x) {
                    glm::vec3 pos = probeOrigin + glm::vec3(x + 0.5f, y + 0.5f, z + 0.5f) * probeSpacing;
                    Probe p;
                    p.Position = pos;
                    m_Probes.push_back(p);
                }
            }
        }

        m_ProbesBuffer = Buffer::Create();
        BufferDesc probesBufferDesc{};
        probesBufferDesc.p_Size = sizeof(Probe) * probeCount;
        probesBufferDesc.p_Usage = Core::BufferUsage::StorageBuffer;
        probesBufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
        probesBufferDesc.p_Persistant = true;
        m_ProbesBuffer->Init(probesBufferDesc);

        m_IrradiannceImage = Texture::Create();
        Texture::TextureSpecification specs{};
        specs.p_Width = probeResolution.x;
        specs.p_Height = probeResolution.y;
        specs.p_Depth = probeResolution.z;
        specs.p_Type = Texture::TextureType::TEXTURE3D;
        specs.p_DebugName = "Irradiance";
        specs.p_Usage = Core::TextureUsage::ImageUsageStorage | Core::TextureUsage::ImageUsageSampled | Core::TextureUsage::ImageUsageTransferDst;
        specs.p_Format = Core::Format::FORMAT_R16G16B16A16_SFLOAT;
        m_IrradiannceImage->Init(specs);

        RenderCommand::s_RendererAPI = RendererAPI::Create();
        ComputeCommand::s_ComputeAPI = ComputeAPI::Create();

        m_Swapchain = SwapchainFactory::CreateSwapchain(Application::GetWindow());
        m_Swapchain->Create(Swapchain::DOUBLE_BUFFERING);

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
                    RenderPassAttachment{ 0, AttachmentType::Depth, m_DepthPre, LoadOp::Clear, StoreOp::Store, Core::ImageLayout::Undefined, Core::ImageLayout::DepthStencilAttachmentOptimal }
                }
            );

            // ShadowMap pass
            //----------------------------------------------------------------------------------------------------
            m_ShadowMapLOD0 = Texture::Create();
            m_ShadowMapLOD1 = Texture::Create();
            m_ShadowMapLOD2 = Texture::Create();
            m_ShadowMapLOD3 = Texture::Create();

            {
                Texture::TextureSpecification specs{};
                specs.p_Width = 2048;
                specs.p_Height = 2048;
                specs.p_Type = Texture::TextureType::TEXTURE2D;
                specs.p_DebugName = "g_ShadowMapLOD0";
                specs.p_Usage = Core::TextureUsage::ImageUsageDepthStencilAttachment | Core::TextureUsage::ImageUsageSampled;
                specs.p_Format = Core::Format::FORMAT_D16_UNORM;
                specs.p_IsDepth = true;
                m_ShadowMapLOD0->Init(specs);

                specs.p_DebugName = "g_ShadowMapLOD1";
                m_ShadowMapLOD1->Init(specs);

                specs.p_DebugName = "g_ShadowMapLOD2";
                m_ShadowMapLOD2->Init(specs);

                specs.p_DebugName = "g_ShadowMapLOD3";
                m_ShadowMapLOD3->Init(specs);
            }

            m_CSMShadowMapPass = CSMRenderPass::Create();
            m_CSMShadowMapPass->Init(
                {
                    m_ShadowMapLOD0, // Cascade 0
                    m_ShadowMapLOD1, // Cascade 1
                    m_ShadowMapLOD2, // Cascade 2
                    m_ShadowMapLOD3, // Cascade 3
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
                specs.p_DebugName = "m_Pos";
                specs.p_Type = Texture::TextureType::TEXTURE2D;
                specs.p_Usage = Core::TextureUsage::ImageUsageColorAttachment | Core::TextureUsage::ImageUsageSampled;
                specs.p_Format = Core::Format::FORMAT_R16G16B16A16_SFLOAT;
                //specs.p_Format = Core::Format::FORMAT_R8G8B8A8_UNORM;
                m_Pos->Init(specs);

                specs.p_DebugName = "m_Normal";
                //specs.p_Format = Core::Format::FORMAT_R8G8B8A8_UNORM;
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
                    RenderPassAttachment{ 0, AttachmentType::Color, m_Pos,      LoadOp::Clear, StoreOp::Store   , Core::ImageLayout::Undefined, Core::ImageLayout::ShaderReadOnlyOptimal },
                    RenderPassAttachment{ 1, AttachmentType::Color, m_Normal,   LoadOp::Clear, StoreOp::Store   , Core::ImageLayout::Undefined, Core::ImageLayout::ShaderReadOnlyOptimal },
                    RenderPassAttachment{ 2, AttachmentType::Color, m_Albedo,   LoadOp::Clear, StoreOp::Store   , Core::ImageLayout::Undefined, Core::ImageLayout::ShaderReadOnlyOptimal },
                    RenderPassAttachment{ 3, AttachmentType::Color, m_Material, LoadOp::Clear, StoreOp::Store   , Core::ImageLayout::Undefined, Core::ImageLayout::ShaderReadOnlyOptimal },
                    RenderPassAttachment{ 4, AttachmentType::Color, m_Emissive, LoadOp::Clear, StoreOp::Store   , Core::ImageLayout::Undefined, Core::ImageLayout::ShaderReadOnlyOptimal },
                    RenderPassAttachment{ 5, AttachmentType::Depth, m_DepthPre, LoadOp::Load,  StoreOp::DontCare, Core::ImageLayout::DepthStencilAttachmentOptimal, Core::ImageLayout::DepthStencilAttachmentOptimal},
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
                    RenderPassAttachment{ 0, AttachmentType::Color, m_LightingOutput, LoadOp::Clear, StoreOp::Store, Core::ImageLayout::Undefined, Core::ImageLayout::ShaderReadOnlyOptimal }
                }
            );

            // Clustered debug pass
            //----------------------------------------------------------------------------------------------------
            m_ClusteredDebugPass = RenderPass::Create();
            m_ClusteredDebugPass->Init(
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
                    RenderPassAttachment{ 0, AttachmentType::Color, m_LightingOutput, LoadOp::Load, StoreOp::Store, Core::ImageLayout::ColorAttachmentOptimal, Core::ImageLayout::ShaderReadOnlyOptimal }
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
                    RenderPassAttachment{ 0, AttachmentType::Swapchain, nullptr, LoadOp::Clear, StoreOp::Store, Core::ImageLayout::Undefined, Core::ImageLayout::PresentSrc }
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
                //vertexLayout.pStride = sizeof(MeshAsset::Vertex);
                vertexLayout.pAttributes = {
                };
                pipelineSpecs.pLayout = vertexLayout;
                pipelineSpecs.pRenderPass = m_DepthPrePass;

                pipelineSpecs.pShaderPathsVK.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/DepthPrePassMS.spv");
                pipelineSpecs.pShaderPathsVK.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/DepthPrePassFS.spv");
                pipelineSpecs.pShaderPathsDX.push_back("\\Shaders\\DirectX12\\DeferredRenderer\\Compiled\\DepthPrePass_vert.cso");
                pipelineSpecs.pShaderPathsDX.push_back("\\Shaders\\DirectX12\\DeferredRenderer\\Compiled\\DepthPrePass_frag.cso");

                pipelineSpecs.pDepthClampEnable = false;
                pipelineSpecs.pRasterizationDiscardEnable = false;
                pipelineSpecs.pPolygoneMode = Pipeline::POLYGON_MODE_FILL;
                pipelineSpecs.pLineWidth = 1.0f;
                pipelineSpecs.pCullMode = Pipeline::CullMode::BACK;
                pipelineSpecs.pFrontFace = Pipeline::FrontFace::COUTNER_CLOCKWISE;
                pipelineSpecs.pDepthBiasEnable = false;
                pipelineSpecs.pDepthTestEnable = true;  
                pipelineSpecs.pDepthWriteEnable = true;
                pipelineSpecs.pCompareOp = Pipeline::COMPARE_OP_GREATER;
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
                //vertexLayout.pStride = sizeof(MeshAsset::Vertex);
                vertexLayout.pAttributes = {
                };
                pipelineSpecs.pLayout = vertexLayout;
                pipelineSpecs.pCSMRenderPass = m_CSMShadowMapPass;

                pipelineSpecs.pShaderPathsVK.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/ShadowMapPassMS.spv");
                pipelineSpecs.pShaderPathsVK.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/ShadowMapPassFS.spv");
                pipelineSpecs.pShaderPathsDX.push_back("\\Shaders\\DirectX12\\DeferredRenderer\\Compiled\\ShadowMapPass_vert.cso");
                pipelineSpecs.pShaderPathsDX.push_back("\\Shaders\\DirectX12\\DeferredRenderer\\Compiled\\ShadowMapPass_frag.cso");

                pipelineSpecs.pDepthClampEnable = false;
                pipelineSpecs.pRasterizationDiscardEnable = false;
                pipelineSpecs.pPolygoneMode = Pipeline::POLYGON_MODE_FILL;
                pipelineSpecs.pLineWidth = 1.0f;
                pipelineSpecs.pCullMode = Pipeline::CullMode::BACK;
                pipelineSpecs.pFrontFace = Pipeline::FrontFace::COUTNER_CLOCKWISE;
                pipelineSpecs.pDepthBiasEnable = false;
                pipelineSpecs.pDepthTestEnable = true;
                pipelineSpecs.pDepthWriteEnable = true;
                pipelineSpecs.pCompareOp = Pipeline::COMPARE_OP_GREATER;
                pipelineSpecs.pDepthBoundsTestEnable = false;
                pipelineSpecs.pStencilTestEnable = false;
                pipelineSpecs.pDebugName = "CSMShadowMap pipeline";

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
                //vertexLayout.pStride = sizeof(MeshAsset::Vertex);
                vertexLayout.pAttributes = {
                };
                pipelineSpecs.pLayout = vertexLayout;
                pipelineSpecs.pRenderPass = m_GeometryBufferPass;

                pipelineSpecs.pShaderPathsVK.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/GeometryPassMS.spv");
                pipelineSpecs.pShaderPathsVK.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/GeometryPassFS.spv");
                pipelineSpecs.pShaderPathsDX.push_back("\\Shaders\\DirectX12\\DeferredRenderer\\Compiled\\GeometryPass_vert.cso");
                pipelineSpecs.pShaderPathsDX.push_back("\\Shaders\\DirectX12\\DeferredRenderer\\Compiled\\GeometryPass_frag.cso");

                pipelineSpecs.pDepthClampEnable = false;
                pipelineSpecs.pRasterizationDiscardEnable = false;
                pipelineSpecs.pPolygoneMode = Pipeline::POLYGON_MODE_FILL;
                pipelineSpecs.pLineWidth = 1.0f;
                pipelineSpecs.pCullMode = Pipeline::CullMode::BACK;
                pipelineSpecs.pFrontFace = Pipeline::FrontFace::COUTNER_CLOCKWISE;
                pipelineSpecs.pDepthBiasEnable = false;
                pipelineSpecs.pDepthTestEnable = true;
                pipelineSpecs.pDepthWriteEnable = false;
                pipelineSpecs.pCompareOp = Pipeline::COMPARE_OP_GREATER;
                pipelineSpecs.pDepthBoundsTestEnable = false;
                pipelineSpecs.pStencilTestEnable = false;
                pipelineSpecs.pDebugName = "GeometryPass pipeline";

                m_GBufferPipeline = Pipeline::Create();
                m_GBufferPipeline->Init(pipelineSpecs);

                pipelineSpecs.pCullMode = Pipeline::CullMode::NONE;
                m_GBufferDoubleSidedPipeline = Pipeline::Create();
                //m_GBufferDoubleSidedPipeline->Init(pipelineSpecs);

                pipelineSpecs.pTransparent = true;
                m_GBufferAlphaBlendPipeline = Pipeline::Create();
                //m_GBufferAlphaBlendPipeline->Init(pipelineSpecs);
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
                pipelineSpecs.pCompareOp = Pipeline::COMPARE_OP_NEVER;
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
        BufferDesc mvpBufferDesc{};
        mvpBufferDesc.p_Size = sizeof(MVP);
        mvpBufferDesc.p_Usage = Core::BufferUsage::UniformBuffer;
        mvpBufferDesc.p_Memory = BufferDesc::MemoryUsage::CPU_To_GPU;
        mvpBufferDesc.p_Persistant = true;
        m_MVPBuffer->Init(mvpBufferDesc);

        m_ShadowDataBuffer = Buffer::Create();
        BufferDesc shadowBufferDesc{};
        shadowBufferDesc.p_Size = sizeof(ShadowData);
        shadowBufferDesc.p_Usage = Core::BufferUsage::UniformBuffer;
        shadowBufferDesc.p_Memory = BufferDesc::MemoryUsage::CPU_To_GPU;
        shadowBufferDesc.p_Persistant = true;
        m_ShadowDataBuffer->Init(shadowBufferDesc);

        {
            m_ClusterTilesSSBO = Buffer::Create();
            BufferDesc clusterTilesBufferDesc{};
            clusterTilesBufferDesc.p_Size = sizeof(TileAABB) * NUM_CLUSTERS;
            clusterTilesBufferDesc.p_Usage = Core::BufferUsage::StorageBuffer;
            clusterTilesBufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
            m_ClusterTilesSSBO->Init(clusterTilesBufferDesc);

            //m_ClustersVertexBuffer = Buffer::Create();
            //BufferDesc clusterVertexBufferDesc{};
            //clusterVertexBufferDesc.p_Size = sizeof(glm::vec3) * NUM_CLUSTERS;
            //clusterVertexBufferDesc.p_Usage = BufferDesc::Usage::VertexBuffer;
            //clusterVertexBufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
            //m_ClustersVertexBuffer->Init(clusterVertexBufferDesc);

            //m_ClustersVertexBuffer = Buffer::Create();
            //BufferDesc clusterIndexBufferDesc{};
            //clusterIndexBufferDesc.p_Size = sizeof(uint32_t) * ;
            //clusterIndexBufferDesc.p_Usage = BufferDesc::Usage::IndexBuffer;
            //clusterIndexBufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
            //m_ClustersVertexBuffer->Init(clusterIndexBufferDesc);

            m_ClusterInfoUBO = Buffer::Create();
            BufferDesc clusterInfoBufferDesc{};
            clusterInfoBufferDesc.p_Size = sizeof(ClusterInfo);
            clusterInfoBufferDesc.p_Usage = Core::BufferUsage::UniformBuffer;
            clusterInfoBufferDesc.p_Memory = BufferDesc::MemoryUsage::CPU_To_GPU;
            clusterInfoBufferDesc.p_Persistant = true;
            m_ClusterInfoUBO->Init(clusterInfoBufferDesc);

            m_AABBGeneratorPipeline->UpdateResources("ClusterInfo", {}, m_ClusterInfoUBO);
            m_AABBGeneratorPipeline->UpdateResources("ClusterAABB", {}, m_ClusterTilesSSBO);
        }

        {
            auto lightsView = SceneManager::pActiveScene->Reg().view<PointLightComponent>();

            std::vector<PointLight> lights;
            for (auto e : lightsView) {
                Entity entity = { e, SceneManager::pActiveScene.get() };
                auto& light = entity.GetComponent<PointLightComponent>();

                PointLight pointLight;
                pointLight.position = glm::vec4(light.Position, light.Radius);
                pointLight.color = glm::vec4(light.Color, light.Intensity);

                lights.push_back(pointLight);
            }

            m_LightsList = Buffer::Create();
            BufferDesc lightsBufferDesc{};
            lightsBufferDesc.p_Size = sizeof(lights) * lights.size();
            lightsBufferDesc.p_Data = lights.data();
            lightsBufferDesc.p_Usage = Core::BufferUsage::StorageBuffer;
            lightsBufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
            lightsBufferDesc.p_AllowSRV = true;
            m_LightsList->Init(lightsBufferDesc);

            m_ClusterLightIndexList = Buffer::Create();
            BufferDesc clusterLightsIndexBufferDesc{};
            clusterLightsIndexBufferDesc.p_Size = sizeof(uint32_t) * NUM_CLUSTERS * MAX_LIGHTS_PER_CLUSTER;
            clusterLightsIndexBufferDesc.p_Usage = Core::BufferUsage::StorageBuffer;
            clusterLightsIndexBufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
            clusterLightsIndexBufferDesc.p_AllowUAV = true;
            m_ClusterLightIndexList->Init(clusterLightsIndexBufferDesc);

            m_ClusterLightOffsetList = Buffer::Create();
            BufferDesc clusterLightsOffsetsBufferDesc{};
            clusterLightsOffsetsBufferDesc.p_Size = sizeof(LightOffset) * NUM_CLUSTERS;
            clusterLightsOffsetsBufferDesc.p_Usage = Core::BufferUsage::StorageBuffer;
            clusterLightsOffsetsBufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
            clusterLightsOffsetsBufferDesc.p_AllowUAV = true;
            m_ClusterLightOffsetList->Init(clusterLightsOffsetsBufferDesc);

            m_AtomicCounters = Buffer::Create();
            BufferDesc atomicCountersBufferDesc{};
            atomicCountersBufferDesc.p_Size = sizeof(uint32_t) * NUM_CLUSTERS;
            atomicCountersBufferDesc.p_Usage = Core::BufferUsage::StorageBuffer;
            atomicCountersBufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
            atomicCountersBufferDesc.p_AllowUAV = true;
            m_AtomicCounters->Init(atomicCountersBufferDesc);

            uint32_t globalIndex = 0;
            m_GlobalIndexCountSSBO = Buffer::Create();
            BufferDesc globalIndexBufferDesc{};
            globalIndexBufferDesc.p_Size = sizeof(uint32_t);
            globalIndexBufferDesc.p_Data = &globalIndex;
            globalIndexBufferDesc.p_Usage = Core::BufferUsage::StorageBuffer;
            globalIndexBufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
            globalIndexBufferDesc.p_AllowUAV = true;
            m_GlobalIndexCountSSBO->Init(globalIndexBufferDesc);

            m_AssignLightsToClustersPipeline->UpdateResources("ClusterInfo", {}, m_ClusterInfoUBO);
            m_AssignLightsToClustersPipeline->UpdateResources("LightsList", {}, m_LightsList);
            m_AssignLightsToClustersPipeline->UpdateResources("ClusterAABB", {}, m_ClusterTilesSSBO);
            m_AssignLightsToClustersPipeline->UpdateResources("ClusterLightIndexList", {}, m_ClusterLightIndexList);
            m_AssignLightsToClustersPipeline->UpdateResources("ClusterLightOffsetList", {}, m_ClusterLightOffsetList);
            m_AssignLightsToClustersPipeline->UpdateResources("AtomicCounters", {}, m_AtomicCounters);
            m_AssignLightsToClustersPipeline->UpdateResources("GlobalIndex", {}, m_GlobalIndexCountSSBO);
        }

        m_DepthPrePassPipeline->UpdateResources("MVP", {}, m_MVPBuffer);

        m_LightingPipeline->UpdateResources("sampler_Position", { m_Pos      }, nullptr);
        m_LightingPipeline->UpdateResources("sampler_Normal",   { m_Normal   }, nullptr);
        m_LightingPipeline->UpdateResources("sampler_Albedo",   { m_Albedo   }, nullptr);
        m_LightingPipeline->UpdateResources("sampler_Material", { m_Material }, nullptr);
        m_LightingPipeline->UpdateResources("sampler_Emissive", { m_Emissive }, nullptr);
        m_LightingPipeline->UpdateResources("sampler_Depth",    { m_DepthPre }, nullptr);
        m_LightingPipeline->UpdateResources("ClusterAABB", {}, m_ClusterTilesSSBO);
        m_LightingPipeline->UpdateResources("MVP",            {}, m_MVPBuffer);
        m_LightingPipeline->UpdateResources("ShadowMaps",       { m_ShadowMapLOD0, m_ShadowMapLOD1, m_ShadowMapLOD2, m_ShadowMapLOD3 }, nullptr);

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            // Creating Fences
            m_ClusterFence[i] = Fence::Create();
            m_ClusterFence[i]->Init();

            m_GraphicsFence[i] = Fence::Create();
            m_GraphicsFence[i]->Init();
            //

            // Creating Semaphores
            ImageAvailableSemaphore[i] = Semaphore::Create();
            ImageAvailableSemaphore[i]->Init();

            AABBGenerateSemaphore[i] = Semaphore::Create();
            AABBGenerateSemaphore[i]->Init();

            AssignLightsSemaphore[i] = Semaphore::Create();
            AssignLightsSemaphore[i]->Init();

            RenderFinishedSemaphore[i] = Semaphore::Create();
            RenderFinishedSemaphore[i]->Init();

            BufferTransferTaskSemaphore[i] = Semaphore::Create();
            BufferTransferTaskSemaphore[i]->Init();
            //

            // Creating Command Buffers
            m_CmdBuffer[i] = CommandBuffer::Create();
            m_CmdBuffer[i]->Allocate(CommandBuffer::PoolType::Graphics);

            m_ClusteredCmdBuffer[i] = CommandBuffer::Create();
            m_ClusteredCmdBuffer[i]->Allocate(CommandBuffer::PoolType::Compute);
            //
        }

        Application::GetRenderer()->m_DrawsBuffer = Buffer::Create();
        BufferDesc drawBufferDesc{};
        drawBufferDesc.p_Name = "Draws buffer";
        drawBufferDesc.p_Size = SIZE_1MB;
        drawBufferDesc.p_Usage = Core::BufferUsage::IndirectBuffer | Core::BufferUsage::StorageBuffer | Core::BufferUsage::TransferDst;
        drawBufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
        drawBufferDesc.p_AllowSRV = true;
        Application::GetRenderer()->m_DrawsBuffer->Init(drawBufferDesc);

        Application::GetRenderer()->m_IndexBuffer = Buffer::Create();
        BufferDesc indexBufferDesc{};
        indexBufferDesc.p_Name = "Index buffer";
        indexBufferDesc.p_Size = SIZE_100MB;
        indexBufferDesc.p_Usage = Core::BufferUsage::IndexBuffer | Core::BufferUsage::TransferDst;
        indexBufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
        indexBufferDesc.p_AllowSRV = true;
        Application::GetRenderer()->m_IndexBuffer->Init(indexBufferDesc);

        Application::GetRenderer()->m_VertexBuffer = Buffer::Create();
        BufferDesc vertexBufferDesc{};
        vertexBufferDesc.p_Name = "Vertices Storage buffer";
        vertexBufferDesc.p_Size = SIZE_100MB;
        vertexBufferDesc.p_Usage = Core::BufferUsage::StorageBuffer | Core::BufferUsage::TransferDst;
        vertexBufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
        vertexBufferDesc.p_AllowSRV = true;
        Application::GetRenderer()->m_VertexBuffer->Init(vertexBufferDesc);

        Application::GetRenderer()->m_MeshletDataBuffer = Buffer::Create();
        BufferDesc meshletDataBufferDesc{};
        meshletDataBufferDesc.p_Name = "Meshes Storage buffer";
        meshletDataBufferDesc.p_Size = SIZE_100MB;
        meshletDataBufferDesc.p_Usage = Core::BufferUsage::StorageBuffer | Core::BufferUsage::TransferDst;
        meshletDataBufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
        meshletDataBufferDesc.p_AllowSRV = true;
        Application::GetRenderer()->m_MeshletDataBuffer->Init(meshletDataBufferDesc);

        Application::GetRenderer()->m_MeshletsBuffer = Buffer::Create();
        BufferDesc meshletBufferDesc{};
        meshletBufferDesc.p_Name = "Meshlets Storage buffer";
        meshletBufferDesc.p_Size = SIZE_100MB;
        meshletBufferDesc.p_Usage = Core::BufferUsage::StorageBuffer | Core::BufferUsage::TransferDst;
        meshletBufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
        meshletBufferDesc.p_AllowSRV = true;
        Application::GetRenderer()->m_MeshletsBuffer->Init(meshletBufferDesc);

        Application::GetRenderer()->m_MaterialStorageBuffer = Buffer::Create();
        BufferDesc materialsBufferDesc{};
        materialsBufferDesc.p_Size = SIZE_10MB;
        materialsBufferDesc.p_Usage = Core::BufferUsage::StorageBuffer | Core::BufferUsage::TransferDst;
        materialsBufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
        materialsBufferDesc.p_AllowUAV = true;
        Application::GetRenderer()->m_MaterialStorageBuffer->Init(materialsBufferDesc);

        m_ScratchBuffer = Buffer::Create();
        BufferDesc scratchBufferDesc{};
        scratchBufferDesc.p_Size = SIZE_100MB * 10;
        scratchBufferDesc.p_Usage = Core::BufferUsage::TransferSrc;
        scratchBufferDesc.p_Memory = BufferDesc::MemoryUsage::CPU_To_GPU;
        m_ScratchBuffer->Init(scratchBufferDesc);

        m_GBufferPipeline->UpdateResources("Vertices", {}, m_VertexBuffer);
        m_GBufferPipeline->UpdateResources("MeshDraws", {}, m_DrawsBuffer);
        m_GBufferPipeline->UpdateResources("Meshlets", {}, m_MeshletsBuffer);
        m_GBufferPipeline->UpdateResources("MeshletData", {}, m_MeshletDataBuffer);
        m_LightingPipeline->UpdateResources("u_Shadow", {}, m_ShadowDataBuffer);
        m_GBufferPipeline->UpdateResources("Materials", {}, m_MaterialStorageBuffer);

        // Creating Queue
        m_GraphicsQueue0 = Queue::Create();
        m_GraphicsQueue0->Init(Queue::QueueType::Graphics);

        m_GraphicsQueue1 = Queue::Create();
        m_GraphicsQueue1->Init(Queue::QueueType::Graphics);

        m_ComputeQueue0 = Queue::Create();
        m_ComputeQueue0->Init(Queue::QueueType::Compute);

        m_ComputeQueue1 = Queue::Create();
        m_ComputeQueue1->Init(Queue::QueueType::Compute);
        //
    }

    void Renderer::QueueBufferUpdate(BufferUpdate update) {
        m_BufferUpdatesQueued.push_back(update);
    }

    void Renderer::RenderScene(float deltaTime)
    {
        if (!SceneManager::pActiveScene) return;

        glm::vec3 lightDir; 
        MVP mvp{};
        mvp.ProjView = Application::GetCamera()->GetViewProjection();
        mvp.View = Application::GetCamera()->GetViewMatrix();
        mvp.CamPos = Application::GetCamera()->GetPosition();

        m_MVPBuffer->UpdatePersistantData(sizeof(MVP), &mvp);

        ClusterInfo clusterInfo{};
        clusterInfo.View = Application::GetCamera()->GetViewMatrix();
        clusterInfo.InverseProj = glm::inverse(Application::GetCamera()->GetProjection());
        uint32_t sizePx = (unsigned int)std::ceilf(1920 / 16.0f);
        clusterInfo.TileSizes = glm::uvec4(16, 9, 24, sizePx);
        clusterInfo.ScreenDimensions = glm::uvec4(m_LightingOutput->GetWidth(), m_LightingOutput->GetHeight(), 1, 1000);
        m_ClusterInfoUBO->UpdatePersistantData(sizeof(ClusterInfo), &clusterInfo);

        float nearClip = Application::GetCamera()->GetNearClip();
        float farClip = Application::GetCamera()->GetFarClip();

        float shadowFar = 1000.0f;
        glm::mat4 finiteProj = glm::perspectiveZO(glm::radians(45.0f), 1.778f, 1.0f, shadowFar);
        finiteProj[1][1] *= -1.0f;

        glm::mat4 cameraProj = Application::GetCamera()->GetProjection();
        glm::mat4 cameraView = Application::GetCamera()->GetViewMatrix();
        lightDir = glm::normalize(glm::vec3(-0.3f, -1.0f, -0.5f));
        float lambda = 0.7f;

        auto lightView = SceneManager::pActiveScene->Reg().view<DirectionalLightComponent>();
        for (auto e : lightView) {
            Entity entity = { e, SceneManager::pActiveScene.get() };
            auto& lc = entity.GetComponent<DirectionalLightComponent>();
            lightDir = lc.Direction;
        }

        //for (int i = 0; i < NUM_CASCADES; i++) {
        //    glm::mat4 lightMatrix = CalculateCascadeMatrix(
        //        i, NUM_CASCADES,
        //        nearClip, farClip,
        //        lambda,
        //        cameraProj, cameraView,
        //        lightDir
        //    );

        //    m_SunMatrices[i] = lightMatrix;
        //}

        m_SunMatrices = getLightSpaceMatrices(glm::normalize(lightDir));

        ShadowData shadowData{};
        shadowData.lightSpaceMatrices[0] = m_SunMatrices[0];
        shadowData.lightSpaceMatrices[1] = m_SunMatrices[1];
        shadowData.lightSpaceMatrices[2] = m_SunMatrices[2];
        shadowData.lightSpaceMatrices[3] = m_SunMatrices[3];
        shadowData.cascadeSplits = glm::vec4(shadowCascadeLevels[0], shadowCascadeLevels[1], shadowCascadeLevels[2], shadowCascadeLevels[3]);
        m_ShadowDataBuffer->UpdatePersistantData(sizeof(ShadowData), &shadowData);

        m_ClusterFence[m_CurrentFrame]->Wait();
        m_ClusterFence[m_CurrentFrame]->Reset();

        m_ClusteredCmdBuffer[m_CurrentFrame]->Reset();
        m_ClusteredCmdBuffer[m_CurrentFrame]->Bind();
        //// --- CLUSTERS AABB GENERATOR COMPUTE TASK ---------------------------
        ////------------------------------------------------------------------------------------------------------------------------------------------------
        m_AABBGeneratorPipeline->Bind(m_ClusteredCmdBuffer[m_CurrentFrame]);
        ComputeCommand::CmdDispatch(m_ClusteredCmdBuffer[m_CurrentFrame], 16, 9, 24);
        m_ClusterTilesSSBO->MemoryPipelineBarrier(m_ClusteredCmdBuffer[m_CurrentFrame],
            {
                Core::AccessType::ShaderWrite,
                Core::AccessType::ShaderWrite,
                Core::PipelineStage::ComputeShader,
                Core::PipelineStage::ComputeShader,
            });
        ////------------------------------------------------------------------------------------------------------------------------------------------------
        m_ClusteredCmdBuffer[m_CurrentFrame]->UnBind();

        Queue::SubmitInfo clusteredSubmitInfo{};
        clusteredSubmitInfo.pCmdBuffers.push_back(m_ClusteredCmdBuffer[m_CurrentFrame]);
        clusteredSubmitInfo.pSignalSemaphores.push_back(AABBGenerateSemaphore[m_CurrentFrame]);
        m_ComputeQueue0->Submit(clusteredSubmitInfo, m_ClusterFence[m_CurrentFrame]);

        m_ClusterFence[m_CurrentFrame]->Wait();
        m_ClusterFence[m_CurrentFrame]->Reset();

        m_ClusteredCmdBuffer[m_CurrentFrame]->Reset();
        m_ClusteredCmdBuffer[m_CurrentFrame]->Bind();
        //// --- ASSIGN LIGHTS TO CLUSTERS COMPUTE TASK ---------------------------
        ////------------------------------------------------------------------------------------------------------------------------------------------------
        m_AssignLightsToClustersPipeline->Bind(m_ClusteredCmdBuffer[m_CurrentFrame]);
        ComputeCommand::CmdDispatch(m_ClusteredCmdBuffer[m_CurrentFrame], 16, 9, 24);
        m_ClusterTilesSSBO->MemoryPipelineBarrier(m_ClusteredCmdBuffer[m_CurrentFrame],
            {
                Core::AccessType::ShaderWrite,
                Core::AccessType::ShaderRead,
                Core::PipelineStage::ComputeShader,
                Core::PipelineStage::ComputeShader,
            });
        m_ClusterLightOffsetList->MemoryPipelineBarrier(m_ClusteredCmdBuffer[m_CurrentFrame],
            {
                Core::AccessType::ShaderWrite,
                Core::AccessType::ShaderRead,
                Core::PipelineStage::ComputeShader,
                Core::PipelineStage::ComputeShader,
            });
        m_ClusterLightIndexList->MemoryPipelineBarrier(m_ClusteredCmdBuffer[m_CurrentFrame],
            {
                Core::AccessType::ShaderWrite,
                Core::AccessType::ShaderRead,
                Core::PipelineStage::ComputeShader,
                Core::PipelineStage::ComputeShader,
            });
        ////------------------------------------------------------------------------------------------------------------------------------------------------
        m_ClusteredCmdBuffer[m_CurrentFrame]->UnBind();

        Queue::SubmitInfo clusteredSubmitInfo2{};
        clusteredSubmitInfo2.pCmdBuffers.push_back(m_ClusteredCmdBuffer[m_CurrentFrame]);
        clusteredSubmitInfo2.pWaitSemaphores.push_back(AABBGenerateSemaphore[m_CurrentFrame]);
        clusteredSubmitInfo2.pSignalSemaphores.push_back(AssignLightsSemaphore[m_CurrentFrame]);
        clusteredSubmitInfo2.pWaitStages.push_back(Core::PipelineStage::ComputeShader);
        m_ComputeQueue0->Submit(clusteredSubmitInfo2, m_ClusterFence[m_CurrentFrame]);

        m_GraphicsFence[m_CurrentFrame]->Wait();
        m_GraphicsFence[m_CurrentFrame]->Reset();

        //

        m_CmdBuffer[m_CurrentFrame]->Reset();
        m_CmdBuffer[m_CurrentFrame]->Bind();

        for (const BufferUpdate& update : m_BufferUpdatesQueued) {
            size_t sizeOffset = m_ScratchAllocator.Allocate(update.Size);
            m_ScratchBuffer->UpdatePersistantData(update.Size, update.Ptr);
            
            update.UpdateBuffer->Update(m_CmdBuffer[m_CurrentFrame], update.Size, update.Ptr, sizeOffset, m_ScratchBuffer);
        }
        m_BufferUpdatesQueued.clear();

        m_CmdBuffer[m_CurrentFrame]->UnBind();

        Queue::SubmitInfo bufferTransferSubmitInfo{};
        bufferTransferSubmitInfo.pCmdBuffers.push_back(m_CmdBuffer[m_CurrentFrame]);
        bufferTransferSubmitInfo.pSignalSemaphores.push_back(BufferTransferTaskSemaphore[m_CurrentFrame]);
        m_GraphicsQueue0->Submit(bufferTransferSubmitInfo, m_GraphicsFence[m_CurrentFrame]);

        //

        m_GraphicsFence[m_CurrentFrame]->Wait();
        m_GraphicsFence[m_CurrentFrame]->Reset();

        m_Swapchain->AcquireNextImage(UINT64_MAX, ImageAvailableSemaphore[m_CurrentFrame], nullptr, &m_ImageIndex);

        m_CmdBuffer[m_CurrentFrame]->Reset();
        m_CmdBuffer[m_CurrentFrame]->Bind();

        // --- SHADOW MAP PASS ---------------------------
        //------------------------------------------------------------------------------------------------------------------------------------------------
        uint32_t framebuffer = 0;
        m_ShadowMapPipeline->Bind(m_CmdBuffer[m_CurrentFrame]);
        for (const glm::mat4& lightMatrix : m_SunMatrices) {
            if (Scene::m_Geometry.draws.size() == 0)
                break;
            m_CSMShadowMapPass->Begin(m_CmdBuffer[m_CurrentFrame], framebuffer++);

            RenderCommand::SetViewport(m_CmdBuffer[m_CurrentFrame], 0, 0, m_ShadowMapLOD0->GetWidth(), m_ShadowMapLOD0->GetHeight(), 0, 1);
            RenderCommand::SetScissor(m_CmdBuffer[m_CurrentFrame], 0, 0, m_ShadowMapLOD0->GetWidth(), m_ShadowMapLOD0->GetHeight());

            glm::mat4 matrix = lightMatrix;
            m_ShadowMapPipeline->BindPushConstant(m_CmdBuffer[m_CurrentFrame], sizeof(glm::mat4), &matrix, 0, Core::ShaderStageFlags::Mesh);
            m_ShadowMapPipeline->Bind(m_CmdBuffer[m_CurrentFrame]);
            RenderCommand::DrawMeshTasksIndirect(m_CmdBuffer[m_CurrentFrame],
                m_DrawsBuffer,
                offsetof(MeshDraw, MeshDraw::groupCountX), Scene::m_Geometry.draws.size(), sizeof(MeshDraw));

            m_CSMShadowMapPass->End(m_CmdBuffer[m_CurrentFrame]);
        }
        //------------------------------------------------------------------------------------------------------------------------------------------------

        {
            //Texture::ImageBarrierParams params{};
            //params.oldLayout = Core::ImageLayout::Undefined;
            //params.newLayout = Core::ImageLayout::DepthStencilAttachmentOptimal;
            //params.srcAccess = Core::AccessType::DepthStencilWrite;
            //params.dstAccess = Core::AccessType::ShaderRead;
            //params.srcStage = Core::PipelineStage::LateFragmentTest;
            //params.dstStage = Core::PipelineStage::FragmentShader;
            //m_DepthPre->TransitionImageLayout(m_CmdBuffer, { params });
        }

        // --- DEPTH PRE PASS ---------------------------
        //------------------------------------------------------------------------------------------------------------------------------------------------
        m_DepthPrePass->Begin(m_CmdBuffer[m_CurrentFrame]);

        RenderCommand::SetViewport(m_CmdBuffer[m_CurrentFrame], 0, 0, m_DepthPre->GetWidth(), m_DepthPre->GetHeight(), 0, 1);
        RenderCommand::SetScissor(m_CmdBuffer[m_CurrentFrame], 0, 0, m_DepthPre->GetWidth(), m_DepthPre->GetHeight());

        m_DepthPrePassPipeline->Bind(m_CmdBuffer[m_CurrentFrame]);

        if (Scene::m_Geometry.draws.size() > 0) {
            glm::mat4 matrix{ 1.0f };
            m_DepthPrePassPipeline->BindPushConstant(m_CmdBuffer[m_CurrentFrame], sizeof(glm::mat4), &matrix, 0, Core::ShaderStageFlags::Mesh);
            RenderCommand::DrawMeshTasksIndirect(m_CmdBuffer[m_CurrentFrame],
                m_DrawsBuffer,
                offsetof(MeshDraw, MeshDraw::groupCountX), Scene::m_Geometry.draws.size(), sizeof(MeshDraw));
        }

        m_DepthPrePass->End(m_CmdBuffer[m_CurrentFrame]);
        //------------------------------------------------------------------------------------------------------------------------------------------------

        // --- GBUFFER PASS ---------------------------
        ////------------------------------------------------------------------------------------------------------------------------------------------------
        m_GeometryBufferPass->Begin(m_CmdBuffer[m_CurrentFrame]);
        RenderCommand::SetViewport(m_CmdBuffer[m_CurrentFrame], 0, 0, m_Pos->GetWidth(), m_Pos->GetHeight(), 0, 1);
        RenderCommand::SetScissor(m_CmdBuffer[m_CurrentFrame], 0, 0, m_Pos->GetWidth(), m_Pos->GetHeight());

        m_GBufferPipeline->Bind(m_CmdBuffer[m_CurrentFrame]);

        if (Scene::m_Geometry.draws.size() > 0) {
            glm::mat4 matrix{ 1.0f };
            m_GBufferPipeline->BindPushConstant(m_CmdBuffer[m_CurrentFrame], sizeof(glm::mat4), &matrix, 0, Core::ShaderStageFlags::Mesh);
            RenderCommand::DrawMeshTasksIndirect(m_CmdBuffer[m_CurrentFrame],
                m_DrawsBuffer,
                offsetof(MeshDraw, MeshDraw::groupCountX), Scene::m_Geometry.draws.size(), sizeof(MeshDraw));
        }

        m_GeometryBufferPass->End(m_CmdBuffer[m_CurrentFrame]);
        ////------------------------------------------------------------------------------------------------------------------------------------------------

        Texture::ImageBarrierParams params{};
        params.oldLayout = Core::ImageLayout::DepthStencilAttachmentOptimal;
        params.newLayout = Core::ImageLayout::ShaderReadOnlyOptimal;
        params.srcAccess = Core::AccessType::DepthStencilWrite;
        params.dstAccess = Core::AccessType::ShaderRead;
        params.srcStage = Core::PipelineStage::LateFragmentTest;
        params.dstStage = Core::PipelineStage::FragmentShader;

        m_DepthPre->TransitionImageLayout(m_CmdBuffer[m_CurrentFrame], { params });

        {
            Texture::ImageBarrierParams params{};
            params.oldLayout = Core::ImageLayout::DepthStencilAttachmentOptimal;
            params.newLayout = Core::ImageLayout::ShaderReadOnlyOptimal;
            params.srcAccess = Core::AccessType::DepthStencilWrite;
            params.dstAccess = Core::AccessType::ShaderRead;
            params.srcStage = Core::PipelineStage::LateFragmentTest;
            params.dstStage = Core::PipelineStage::FragmentShader;
            m_ShadowMapLOD0->TransitionImageLayout(m_CmdBuffer[m_CurrentFrame], { params });
            m_ShadowMapLOD1->TransitionImageLayout(m_CmdBuffer[m_CurrentFrame], { params });
            m_ShadowMapLOD2->TransitionImageLayout(m_CmdBuffer[m_CurrentFrame], { params });
            m_ShadowMapLOD3->TransitionImageLayout(m_CmdBuffer[m_CurrentFrame], { params });
        }

        //// --- LIGHTING PASS ---------------------------
        ////------------------------------------------------------------------------------------------------------------------------------------------------
        m_LightingPass->Begin(m_CmdBuffer[m_CurrentFrame]);
        m_LightingPipeline->Bind(m_CmdBuffer[m_CurrentFrame]);

        RenderCommand::SetViewport(m_CmdBuffer[m_CurrentFrame], 0, 0, m_LightingOutput->GetWidth(), m_LightingOutput->GetHeight(), 0, 1);
        RenderCommand::SetScissor(m_CmdBuffer[m_CurrentFrame], 0, 0, m_LightingOutput->GetWidth(), m_LightingOutput->GetHeight());

        m_LightingPipeline->BindPushConstant(m_CmdBuffer[m_CurrentFrame], sizeof(glm::vec3), &lightDir, 0, Core::ShaderStageFlags::Fragment);
        RenderCommand::Draw(m_CmdBuffer[m_CurrentFrame], 3, 0);

        m_LightingPass->End(m_CmdBuffer[m_CurrentFrame]);
        ////------------------------------------------------------------------------------------------------------------------------------------------------

        {
            //Texture::ImageBarrierParams params{};
            //params.oldLayout = Core::ImageLayout::PresentSrc;
            //params.newLayout = Core::ImageLayout::ColorAttachmentOptimal;
            //params.srcAccess = Core::AccessType::DepthStencilWrite;
            //params.dstAccess = Core::AccessType::ShaderRead;
            //params.srcStage = Core::PipelineStage::LateFragmentTest;
            //params.dstStage = Core::PipelineStage::FragmentShader;    
            //m_Swapchain->TransitionCurrentImage(m_CmdBuffer, params, m_ImageIndex);
        }

        //// --- UI PASS ---------------------------
        ////------------------------------------------------------------------------------------------------------------------------------------------------
        m_UIPass->Begin(m_CmdBuffer[m_CurrentFrame], m_ImageIndex);

        RenderCommand::SetViewport(m_CmdBuffer[m_CurrentFrame], 0, 0, m_Swapchain->GetExtentWidth(), m_Swapchain->GetExtentHeight(), 0, 1);
        RenderCommand::SetScissor(m_CmdBuffer[m_CurrentFrame], 0, 0, m_Swapchain->GetExtentWidth(), m_Swapchain->GetExtentHeight());

        Application::GetGuiLayer()->Render(m_CmdBuffer[m_CurrentFrame]);

        m_UIPass->End(m_CmdBuffer[m_CurrentFrame]);
        ////------------------------------------------------------------------------------------------------------------------------------------------------

        {
            //Texture::ImageBarrierParams params{};
            //params.oldLayout = Core::ImageLayout::ColorAttachmentOptimal;
            //params.newLayout = Core::ImageLayout::PresentSrc;
            //params.srcAccess = Core::AccessType::DepthStencilWrite;
            //params.dstAccess = Core::AccessType::ShaderRead;
            //params.srcStage = Core::PipelineStage::LateFragmentTest;
            //params.dstStage = Core::PipelineStage::FragmentShader;
            //m_Swapchain->TransitionCurrentImage(m_CmdBuffer, params, m_ImageIndex);
        }

        m_CmdBuffer[m_CurrentFrame]->UnBind();

        Queue::SubmitInfo renderingSubmitInfo{};
        renderingSubmitInfo.pWaitSemaphores.push_back(ImageAvailableSemaphore[m_CurrentFrame]);
        renderingSubmitInfo.pWaitSemaphores.push_back(AssignLightsSemaphore[m_CurrentFrame]);
        renderingSubmitInfo.pWaitSemaphores.push_back(BufferTransferTaskSemaphore[m_CurrentFrame]);
        renderingSubmitInfo.pSignalSemaphores.push_back(RenderFinishedSemaphore[m_CurrentFrame]);
        renderingSubmitInfo.pWaitStages.push_back(Core::PipelineStage::ColorAttachment);
        renderingSubmitInfo.pWaitStages.push_back(Core::PipelineStage::EarlyFragmentTest);
        renderingSubmitInfo.pWaitStages.push_back(Core::PipelineStage::TransferStage);
        renderingSubmitInfo.pCmdBuffers.push_back(m_CmdBuffer[m_CurrentFrame]);

        m_GraphicsQueue0->Submit(renderingSubmitInfo, m_GraphicsFence[m_CurrentFrame]);

        Queue::PresentInfo presentInfo{};
        presentInfo.pWaitSemaphores.push_back(RenderFinishedSemaphore[m_CurrentFrame]);
        presentInfo.pSwapchains.push_back(m_Swapchain);
        presentInfo.pImageIndex = m_ImageIndex;

        // Present
        m_GraphicsQueue0->Present(presentInfo);

        m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    glm::mat4 GetWorldTransform(Entity entity) {
        glm::mat4 local = entity.GetComponent<TransformComponent>().GetTransform();

        //if (entity.HasComponent<ParentComponent>()) {
        //    Entity parent = entity.GetComponent<ParentComponent>().parent;
        //    return GetWorldTransform(parent) * local;
        //}

        return local;
    }   

    void Renderer::Release() {
        m_Pos->Release();
        m_Normal->Release();
        m_Albedo->Release();
        m_Material->Release();
        m_Emissive->Release();
        m_DepthPre->Release();
        //m_ShadowMap->Release();
        m_LightingOutput->Release();

        m_DepthPrePass->Release();
        //m_ShadowMapPass->Release();
        m_GeometryBufferPass->Release();
        m_LightingPass->Release();
        m_UIPass->Release();

        m_DepthPrePassPipeline->Release();
        m_ShadowMapPipeline->Release();
        m_GBufferPipeline->Release();
        m_GBufferDoubleSidedPipeline->Release();
        m_GBufferAlphaBlendPipeline->Release();
        m_LightingPipeline->Release();

        m_Swapchain->Release();

        ImageAvailableSemaphore[m_CurrentFrame]->Release();
        RenderFinishedSemaphore[m_CurrentFrame]->Release();

        m_ClusterFence[m_CurrentFrame]->Release();
        m_GraphicsFence[m_CurrentFrame]->Release();
    }

    std::unique_ptr<Renderer> Renderer::Create()
    {
        return std::make_unique<Renderer>();
    }
}
