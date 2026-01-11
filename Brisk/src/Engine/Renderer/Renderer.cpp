// INCLUDES
#include "pch.hpp"
#include "Renderer.hpp"
#include "RenderPass.hpp"
#include "RenderCommand.hpp"
#include "ComputeCommand.hpp"
#include "Engine/Component.hpp"
#include "Graphics/Factories/SwapchainFactory.hpp"
//------------------------------------------------
#include <fastgltf/types.hpp>
#include <Graphics/Vulkan/BLASVulkan.hpp>
#include <Graphics/Vulkan/TLASVulkan.hpp>
#include <Engine/Mutexes.hpp>

namespace Brisk
{
#define NUM_CASCADES 4

    Swapchain::Mode swapchainMode = Swapchain::Mode::DOUBLE_BUFFERING;
    std::shared_ptr<Swapchain> Renderer::m_Swapchain;

    float cascadeSplitLambda = 0.55f;
    std::vector<float> cascadeSplits;
    std::vector<glm::mat4> cascadeMatrices;
    float farPlane = 200;
    std::vector<float> shadowCascadeLevels{ farPlane / 50.0f, farPlane / 25.0f, farPlane / 10.0f, farPlane / 2.0f };

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

    glm::mat4 proj;
    glm::mat4 view;
    glm::mat4 getLightSpaceMatrix(const float nearPlane, const float farPlane, glm::vec3 lightDir)
    {
        auto proj = glm::perspectiveZO(
            glm::radians(45.0f), (float)1920 / (float)1080, nearPlane,
            farPlane);
        proj[1][1] *= -1.0f;
        view = Application::GetEditorCamera()->GetViewMatrix();

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
        cascadeSplits.resize(NUM_CASCADES);
        cascadeMatrices.resize(NUM_CASCADES);

        m_ScratchAllocator.m_ScratchBuffer = Buffer::Create();
        BufferDesc scratchBufferDesc{};
        scratchBufferDesc.p_Name = "Scratch buffer";
        scratchBufferDesc.p_Size = SIZE_100MB * 10;
        scratchBufferDesc.p_Usage = Core::BufferUsage::TransferSrc;
        scratchBufferDesc.p_Memory = BufferDesc::MemoryUsage::CPU_To_GPU;
        m_ScratchAllocator.m_ScratchBuffer->Init(scratchBufferDesc);

        m_SunMatrices.resize(NUM_CASCADES);

        RenderCommand::s_RendererAPI = RendererAPI::Create();
        ComputeCommand::s_ComputeAPI = ComputeAPI::Create();

        m_Swapchain = SwapchainFactory::CreateSwapchain();
        m_Swapchain->Create(swapchainMode);

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
                    RenderPassAttachment{ 0, AttachmentType::Depth, LoadOp::Clear, StoreOp::Store, m_DepthPre->GetSpecs().p_Format, Core::ImageLayout::Undefined, Core::ImageLayout::DepthStencilAttachmentOptimal}
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
                    RenderPassAttachment{ 0, AttachmentType::Color, LoadOp::Clear, StoreOp::Store   , m_Pos->GetSpecs().p_Format,      Core::ImageLayout::Undefined, Core::ImageLayout::ShaderReadOnlyOptimal },
                    RenderPassAttachment{ 1, AttachmentType::Color, LoadOp::Clear, StoreOp::Store   , m_Normal->GetSpecs().p_Format,   Core::ImageLayout::Undefined, Core::ImageLayout::ShaderReadOnlyOptimal },
                    RenderPassAttachment{ 2, AttachmentType::Color, LoadOp::Clear, StoreOp::Store   , m_Albedo->GetSpecs().p_Format,   Core::ImageLayout::Undefined, Core::ImageLayout::ShaderReadOnlyOptimal },
                    RenderPassAttachment{ 3, AttachmentType::Color, LoadOp::Clear, StoreOp::Store   , m_Material->GetSpecs().p_Format, Core::ImageLayout::Undefined, Core::ImageLayout::ShaderReadOnlyOptimal },
                    RenderPassAttachment{ 4, AttachmentType::Color, LoadOp::Clear, StoreOp::Store   , m_Emissive->GetSpecs().p_Format, Core::ImageLayout::Undefined, Core::ImageLayout::ShaderReadOnlyOptimal },
                    RenderPassAttachment{ 5, AttachmentType::Depth, LoadOp::Load,  StoreOp::DontCare, m_DepthPre->GetSpecs().p_Format, Core::ImageLayout::DepthStencilAttachmentOptimal, Core::ImageLayout::DepthStencilAttachmentOptimal},
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
                specs.p_Usage = Core::TextureUsage::ImageUsageColorAttachment | Core::TextureUsage::ImageUsageSampled | Core::TextureUsage::ImageUsageStorage;
                specs.p_Format = Core::Format::FORMAT_R16G16B16A16_SFLOAT;
                m_LightingOutput->Init(specs);
            }

            m_GlowOuput = Texture::Create();

            {
                Texture::TextureSpecification specs{};
                specs.p_Width = 1920;
                specs.p_Height = 1080;
                specs.p_DebugName = "g_GlowOuput";
                specs.p_Usage = Core::TextureUsage::ImageUsageColorAttachment | Core::TextureUsage::ImageUsageSampled;
                specs.p_Format = Core::Format::FORMAT_R16G16B16A16_SFLOAT;
                m_GlowOuput->Init(specs);
            }

            m_BloomOutputH = Texture::Create();

            {
                Texture::TextureSpecification specs{};
                specs.p_Width = BLOOM_DIM;
                specs.p_Height = BLOOM_DIM;
                specs.p_DebugName = "g_BloomBlur_H";
                specs.p_Usage = Core::TextureUsage::ImageUsageColorAttachment | Core::TextureUsage::ImageUsageSampled;
                specs.p_Format = Core::Format::FORMAT_R16G16B16A16_SFLOAT;
                m_BloomOutputH->Init(specs);
            }

            m_BloomOutputV = Texture::Create();

            {
                Texture::TextureSpecification specs{};
                specs.p_Width = BLOOM_DIM;
                specs.p_Height = BLOOM_DIM;
                specs.p_DebugName = "g_BloomBlur_V";
                specs.p_Usage = Core::TextureUsage::ImageUsageColorAttachment | Core::TextureUsage::ImageUsageSampled;
                specs.p_Format = Core::Format::FORMAT_R16G16B16A16_SFLOAT;
                m_BloomOutputV->Init(specs);
            }

            m_BloomCombineOutput = Texture::Create();

            {
                Texture::TextureSpecification specs{};
                specs.p_Width = 1920;
                specs.p_Height = 1080;
                specs.p_DebugName = "g_BloomCombine";
                specs.p_Usage = Core::TextureUsage::ImageUsageColorAttachment | Core::TextureUsage::ImageUsageSampled;
                specs.p_Format = Core::Format::FORMAT_R16G16B16A16_SFLOAT;
                m_BloomCombineOutput->Init(specs);
            }

            m_TonemapOutput = Texture::Create();

            {
                Texture::TextureSpecification specs{};
                specs.p_Width = 1920;
                specs.p_Height = 1080;
                specs.p_DebugName = "g_Tonemap";
                specs.p_Usage = Core::TextureUsage::ImageUsageColorAttachment | Core::TextureUsage::ImageUsageSampled | Core::TextureUsage::ImageUsageStorage;
                specs.p_Format = Core::Format::FORMAT_R8G8B8A8_UNORM;
                m_TonemapOutput->Init(specs);
            }


            m_AccumulationImage = Texture::Create();

            {
                Texture::TextureSpecification specs{};
                specs.p_Width = 1920;
                specs.p_Height = 1080;
                specs.p_DebugName = "g_Accumulation";
                specs.p_Usage = Core::TextureUsage::ImageUsageColorAttachment | Core::TextureUsage::ImageUsageSampled | Core::TextureUsage::ImageUsageStorage;
                specs.p_Format = Core::Format::FORMAT_R16G16B16A16_SFLOAT;
                m_AccumulationImage->Init(specs);
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
                    RenderPassDependency{
                        0,
                        -1,
                        Core::AccessType::ColorAttachmentWrite, // src access
                        Core::AccessType::ShaderRead, // dst access
                        Core::PipelineStage::ColorAttachment, // src stage
                        Core::PipelineStage::FragmentShader // dst stage
                    },

                },
                {
                    RenderPassAttachment{ 0, AttachmentType::Color, LoadOp::Clear, StoreOp::Store, m_LightingOutput->GetSpecs().p_Format, Core::ImageLayout::Undefined, Core::ImageLayout::ColorAttachmentOptimal },
                    RenderPassAttachment{ 1, AttachmentType::Color, LoadOp::Clear, StoreOp::Store, m_GlowOuput->GetSpecs().p_Format, Core::ImageLayout::Undefined, Core::ImageLayout::ShaderReadOnlyOptimal }
                }
                );

            m_TransparentPass = RenderPass::Create();
            m_TransparentPass->Init(
                {
                    RenderPassDependency {
                        -1,
                        0,
                        Core::AccessType::None, // src access
                            Core::AccessType::ColorAttachmentWrite | Core::AccessType::ColorAttachmentRead, // dst access
                            Core::PipelineStage::ColorAttachment, // src stage
                            Core::PipelineStage::ColorAttachment // dst stage
                    },
                    RenderPassDependency{
                        0,
                        -1,
                        Core::AccessType::ColorAttachmentWrite | Core::AccessType::ColorAttachmentRead, // src access
                        Core::AccessType::ShaderRead, // dst access
                        Core::PipelineStage::ColorAttachment, // src stage
                        Core::PipelineStage::FragmentShader // dst stage
                    },
                },
                {
                    RenderPassAttachment{ 0, AttachmentType::Color, LoadOp::Load, StoreOp::Store, m_LightingOutput->GetSpecs().p_Format, Core::ImageLayout::ColorAttachmentOptimal, Core::ImageLayout::ShaderReadOnlyOptimal },
                }
            );

                m_TonemappingPass = RenderPass::Create();
                m_TonemappingPass->Init(
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
                    RenderPassAttachment{ 0, AttachmentType::Color, LoadOp::Clear, StoreOp::Store, m_TonemapOutput->GetSpecs().p_Format, Core::ImageLayout::Undefined, Core::ImageLayout::ShaderReadOnlyOptimal }
                }
                );

                m_BloomBlurPass = RenderPass::Create();
                m_BloomBlurPass->Init(
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
                    RenderPassAttachment{ 0, AttachmentType::Color, LoadOp::Clear, StoreOp::Store, m_BloomOutputH->GetSpecs().p_Format, Core::ImageLayout::Undefined, Core::ImageLayout::ShaderReadOnlyOptimal },
                }
                );

                m_BloomCombinePass = RenderPass::Create();
                m_BloomCombinePass->Init(
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
                    RenderPassAttachment{ 0, AttachmentType::Color, LoadOp::Clear, StoreOp::Store, m_BloomCombineOutput->GetSpecs().p_Format, Core::ImageLayout::Undefined, Core::ImageLayout::ShaderReadOnlyOptimal },
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
                            0,
                            -1,
                            Core::AccessType::ColorAttachmentWrite,
                            Core::AccessType::None,
                            Core::PipelineStage::ColorAttachment,
                            Core::PipelineStage::BottomOfPipe
                        },
                    },
                {
                    RenderPassAttachment{ 0, AttachmentType::Swapchain, LoadOp::Clear, StoreOp::Store, Core::Format::FORMAT_R8G8B8A8_UNORM, Core::ImageLayout::Undefined, Core::ImageLayout::PresentSrc }
                }
                );
        }

        // Framebuffers
        {
            m_DepthPreFramebuffer = Framebuffer::Create();
            {
                Framebuffer::FramebufferSpecs specs{};
                specs.p_Width = 1920;
                specs.p_Height = 1080;
                specs.p_RenderPass = m_DepthPrePass;
                specs.p_Attachments.push_back(m_DepthPre);

                m_DepthPreFramebuffer->Init(specs);
            }

            m_GBufferFramebuffer = Framebuffer::Create();
            {
                Framebuffer::FramebufferSpecs specs{};
                specs.p_Width = 1920;
                specs.p_Height = 1080;
                specs.p_RenderPass = m_GeometryBufferPass;
                specs.p_Attachments.push_back(m_Pos);
                specs.p_Attachments.push_back(m_Normal);
                specs.p_Attachments.push_back(m_Albedo);
                specs.p_Attachments.push_back(m_Material);
                specs.p_Attachments.push_back(m_Emissive);
                specs.p_Attachments.push_back(m_DepthPre);

                m_GBufferFramebuffer->Init(specs);
            }

            m_LightingFramebuffer = Framebuffer::Create();
            {
                Framebuffer::FramebufferSpecs specs{};
                specs.p_Width = 1920;
                specs.p_Height = 1080;
                specs.p_RenderPass = m_LightingPass;
                specs.p_Attachments.push_back(m_LightingOutput);
                specs.p_Attachments.push_back(m_GlowOuput);

                m_LightingFramebuffer->Init(specs);
            }

            m_TransparentFramebuffer = Framebuffer::Create();
            {
                Framebuffer::FramebufferSpecs specs{};
                specs.p_Width = 1920;
                specs.p_Height = 1080;
                specs.p_RenderPass = m_TransparentPass;
                specs.p_Attachments.push_back(m_LightingOutput);

                m_TransparentFramebuffer->Init(specs);
            }

            m_TonemappingFramebuffer = Framebuffer::Create();
            {
                Framebuffer::FramebufferSpecs specs{};
                specs.p_Width = 1920;
                specs.p_Height = 1080;
                specs.p_RenderPass = m_TonemappingPass;
                specs.p_Attachments.push_back(m_TonemapOutput);

                m_TonemappingFramebuffer->Init(specs);
            }

            m_BloomBlurHFramebuffer = Framebuffer::Create();
            {
                Framebuffer::FramebufferSpecs specs{};
                specs.p_Width = BLOOM_DIM;
                specs.p_Height = BLOOM_DIM;
                specs.p_RenderPass = m_BloomBlurPass;
                specs.p_Attachments.push_back(m_BloomOutputH);

                m_BloomBlurHFramebuffer->Init(specs);
            }

            m_BloomBlurVFramebuffer = Framebuffer::Create();
            {
                Framebuffer::FramebufferSpecs specs{};
                specs.p_Width = BLOOM_DIM;
                specs.p_Height = BLOOM_DIM;
                specs.p_RenderPass = m_BloomBlurPass;
                specs.p_Attachments.push_back(m_BloomOutputV);

                m_BloomBlurVFramebuffer->Init(specs);
            }

            m_BloomCombineFramebuffer = Framebuffer::Create();
            {
                Framebuffer::FramebufferSpecs specs{};
                specs.p_Width = 1920;
                specs.p_Height = 1080;
                specs.p_RenderPass = m_BloomCombinePass;
                specs.p_Attachments.push_back(m_BloomCombineOutput);

                m_BloomCombineFramebuffer->Init(specs);
            }
            
            m_UIFramebuffers.resize(Application::GetRenderer()->GetSwapchain()->GetImageCount());
            for (int i = 0; i < m_UIFramebuffers.size(); i++) {
                m_UIFramebuffers[i] = Framebuffer::Create();
                {
                    Framebuffer::FramebufferSpecs specs{};
                    specs.p_Width = 1920;
                    specs.p_Height = 1080;
                    specs.p_RenderPass = m_UIPass;
                    specs.swapchainIndex = i;

                    m_UIFramebuffers[i]->Init(specs);
                }
            }
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
                pipelineSpecs.pCompareOp = Pipeline::COMPARE_OP_LESS;
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
                pipelineSpecs.pCompareOp = Pipeline::COMPARE_OP_LESS_OR_EQUAL;
                pipelineSpecs.pDepthBoundsTestEnable = false;
                pipelineSpecs.pStencilTestEnable = false;
                pipelineSpecs.pDebugName = "GeometryPass pipeline";

                m_GBufferPipeline = Pipeline::Create();
                m_GBufferPipeline->Init(pipelineSpecs);

                pipelineSpecs.pFrontFace = Pipeline::FrontFace::CLOCKWISE;
                pipelineSpecs.pCullMode = Pipeline::CullMode::NONE;
                m_GBufferDoubleSidedPipeline = Pipeline::Create();
                m_GBufferDoubleSidedPipeline->Init(pipelineSpecs);
            }
            //----------------------------------------------------------------------------------------------------

            // Lighting pass pipeline
            //----------------------------------------------------------------------------------------------------
            {
                Pipeline::GraphicsPipelineSpecs pipelineSpecs{};
                pipelineSpecs.pRenderPass = m_LightingPass;
                pipelineSpecs.pShaderPathsVK.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/QuadRenderPassVS.spv");
                pipelineSpecs.pShaderPathsVK.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/LightingPassFS.spv");
                pipelineSpecs.pShaderPathsDX.push_back("\\Shaders\\DirectX12\\DeferredRenderer\\Compiled\\LightingPass_vert.cso");
                pipelineSpecs.pShaderPathsDX.push_back("\\Shaders\\DirectX12\\DeferredRenderer\\Compiled\\LightingPass_frag.cso");

                pipelineSpecs.pDepthClampEnable = false;
                pipelineSpecs.pRasterizationDiscardEnable = false;
                pipelineSpecs.pPolygoneMode = Pipeline::POLYGON_MODE_FILL;
                pipelineSpecs.pLineWidth = 1.0f;
                pipelineSpecs.pCullMode = Pipeline::CullMode::BACK;
                pipelineSpecs.pFrontFace = Pipeline::FrontFace::COUTNER_CLOCKWISE;
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

            // Transparent pass pipeline
            //----------------------------------------------------------------------------------------------------
            {
                Pipeline::GraphicsPipelineSpecs pipelineSpecs{};
                Pipeline::VertexDataLayout vertexLayout;
                vertexLayout.pBinding = 0;
                //vertexLayout.pStride = sizeof(MeshAsset::Vertex);
                vertexLayout.pAttributes = {
                };
                pipelineSpecs.pLayout = vertexLayout;
                pipelineSpecs.pRenderPass = m_TransparentPass;

                pipelineSpecs.pShaderPathsVK.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/ForwardPassMS.spv");
                pipelineSpecs.pShaderPathsVK.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/ForwardPassFS.spv");
                pipelineSpecs.pShaderPathsDX.push_back("\\Shaders\\DirectX12\\DeferredRenderer\\Compiled\\GeometryPass_vert.cso");
                pipelineSpecs.pShaderPathsDX.push_back("\\Shaders\\DirectX12\\DeferredRenderer\\Compiled\\GeometryPass_frag.cso");

                pipelineSpecs.pDepthClampEnable = false;
                pipelineSpecs.pRasterizationDiscardEnable = false;
                pipelineSpecs.pPolygoneMode = Pipeline::POLYGON_MODE_FILL;
                pipelineSpecs.pLineWidth = 1.0f;
                pipelineSpecs.pDepthBiasEnable = false;
                pipelineSpecs.pDepthTestEnable = false;
                pipelineSpecs.pDepthWriteEnable = false;
                pipelineSpecs.pCompareOp = Pipeline::COMPARE_OP_LESS_OR_EQUAL;
                pipelineSpecs.pDepthBoundsTestEnable = false;
                pipelineSpecs.pStencilTestEnable = false;
                pipelineSpecs.pDebugName = "ForwardPass pipeline";
                pipelineSpecs.pFrontFace = Pipeline::FrontFace::CLOCKWISE;
                pipelineSpecs.pCullMode = Pipeline::CullMode::NONE;
                pipelineSpecs.pTransparent = true;

                m_TransparentPipeline = Pipeline::Create();
                m_TransparentPipeline->Init(pipelineSpecs);
            }
            //----------------------------------------------------------------------------------------------------

            // Blur pass pipeline
            //----------------------------------------------------------------------------------------------------
            {
                Pipeline::GraphicsPipelineSpecs pipelineSpecs{};
                pipelineSpecs.pRenderPass = m_BloomBlurPass;
                pipelineSpecs.pShaderPathsVK.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/QuadRenderPassVS.spv");
                pipelineSpecs.pShaderPathsVK.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/BloomBlurVPassFS.spv");
                pipelineSpecs.pShaderPathsDX.push_back("\\Shaders\\DirectX12\\DeferredRenderer\\Compiled\\BloomBlurPass_vert.cso");
                pipelineSpecs.pShaderPathsDX.push_back("\\Shaders\\DirectX12\\DeferredRenderer\\Compiled\\BloomBlurPass_frag.cso");

                pipelineSpecs.pDepthClampEnable = false;
                pipelineSpecs.pRasterizationDiscardEnable = false;
                pipelineSpecs.pPolygoneMode = Pipeline::POLYGON_MODE_FILL;
                pipelineSpecs.pLineWidth = 1.0f;
                pipelineSpecs.pCullMode = Pipeline::CullMode::BACK;
                pipelineSpecs.pFrontFace = Pipeline::FrontFace::COUTNER_CLOCKWISE;
                pipelineSpecs.pDepthBiasEnable = false;
                pipelineSpecs.pDepthTestEnable = false;
                pipelineSpecs.pDepthWriteEnable = false;
                pipelineSpecs.pCompareOp = Pipeline::COMPARE_OP_NEVER;
                pipelineSpecs.pDepthBoundsTestEnable = false;
                pipelineSpecs.pStencilTestEnable = false;
                pipelineSpecs.pDebugName = "BloomBlurVPass pipeline";

                m_BlurVPipeline = Pipeline::Create();
                m_BlurVPipeline->Init(pipelineSpecs);

                pipelineSpecs.pDebugName = "BloomBlurHPass pipeline";
                pipelineSpecs.pShaderPathsVK[1] = "Shaders/Vulkan/DeferredRenderer/Compiled/BloomBlurHPassFS.spv";
                m_BlurHPipeline = Pipeline::Create();
                m_BlurHPipeline->Init(pipelineSpecs);
            }
            //----------------------------------------------------------------------------------------------------

            // Bloom combine pass pipeline
            //----------------------------------------------------------------------------------------------------
            {
                Pipeline::GraphicsPipelineSpecs pipelineSpecs{};
                pipelineSpecs.pRenderPass = m_BloomBlurPass;
                pipelineSpecs.pShaderPathsVK.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/QuadRenderPassVS.spv");
                pipelineSpecs.pShaderPathsVK.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/BloomCombinePassFS.spv");
                pipelineSpecs.pShaderPathsDX.push_back("\\Shaders\\DirectX12\\DeferredRenderer\\Compiled\\BloomBlurPass_vert.cso");
                pipelineSpecs.pShaderPathsDX.push_back("\\Shaders\\DirectX12\\DeferredRenderer\\Compiled\\BloomBlurPass_frag.cso");

                pipelineSpecs.pDepthClampEnable = false;
                pipelineSpecs.pRasterizationDiscardEnable = false;
                pipelineSpecs.pPolygoneMode = Pipeline::POLYGON_MODE_FILL;
                pipelineSpecs.pLineWidth = 1.0f;
                pipelineSpecs.pCullMode = Pipeline::CullMode::BACK;
                pipelineSpecs.pFrontFace = Pipeline::FrontFace::COUTNER_CLOCKWISE;
                pipelineSpecs.pDepthBiasEnable = false;
                pipelineSpecs.pDepthTestEnable = false;
                pipelineSpecs.pDepthWriteEnable = false;
                pipelineSpecs.pCompareOp = Pipeline::COMPARE_OP_NEVER;
                pipelineSpecs.pDepthBoundsTestEnable = false;
                pipelineSpecs.pStencilTestEnable = false;
                pipelineSpecs.pDebugName = "BloomCombinePass pipeline";

                m_BloomCombinePipeline = Pipeline::Create();
                m_BloomCombinePipeline->Init(pipelineSpecs);
            }
            //----------------------------------------------------------------------------------------------------

            // Tonemapping pass pipeline
            //----------------------------------------------------------------------------------------------------
            {
                Pipeline::GraphicsPipelineSpecs pipelineSpecs{};
                pipelineSpecs.pRenderPass = m_TonemappingPass;
                pipelineSpecs.pShaderPathsVK.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/QuadRenderPassVS.spv");
                pipelineSpecs.pShaderPathsVK.push_back("Shaders/Vulkan/DeferredRenderer/Compiled/TonemappingPassFS.spv");
                pipelineSpecs.pShaderPathsDX.push_back("\\Shaders\\DirectX12\\DeferredRenderer\\Compiled\\Tonemapping_vert.cso");
                pipelineSpecs.pShaderPathsDX.push_back("\\Shaders\\DirectX12\\DeferredRenderer\\Compiled\\Tonemapping_frag.cso");

                pipelineSpecs.pDepthClampEnable = false;
                pipelineSpecs.pRasterizationDiscardEnable = false;
                pipelineSpecs.pPolygoneMode = Pipeline::POLYGON_MODE_FILL;
                pipelineSpecs.pLineWidth = 1.0f;
                pipelineSpecs.pCullMode = Pipeline::CullMode::BACK;
                pipelineSpecs.pFrontFace = Pipeline::FrontFace::COUTNER_CLOCKWISE;
                pipelineSpecs.pDepthBiasEnable = false;
                pipelineSpecs.pDepthTestEnable = false;
                pipelineSpecs.pDepthWriteEnable = false;
                pipelineSpecs.pCompareOp = Pipeline::COMPARE_OP_NEVER;
                pipelineSpecs.pDepthBoundsTestEnable = false;
                pipelineSpecs.pStencilTestEnable = false;
                pipelineSpecs.pDebugName = "Tonemapping pipeline";

                m_TonemappingPipeline = Pipeline::Create();
                m_TonemappingPipeline->Init(pipelineSpecs);
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

            // -- Ray Tracing --
            // Ray tracing pipeline
            {
                // The order of these paths matter
                // Raygen
                // Miss 
                // Closest Hit  
                Pipeline::RayTracingPipelineSpecs pipelineSpecs{};
                pipelineSpecs.pShaderPathsVK.push_back("Shaders/Vulkan/RayTracing/Compiled/RayGen.spv");
                pipelineSpecs.pShaderPathsVK.push_back("Shaders/Vulkan/RayTracing/Compiled/Miss.spv");
                pipelineSpecs.pShaderPathsVK.push_back("Shaders/Vulkan/RayTracing/Compiled/CHit.spv");

                m_RayTracing = Pipeline::Create();
                m_RayTracing->Init(pipelineSpecs);
            }
        }

        m_BLAS = std::make_shared<BLASVulkan>();
        m_TLAS = std::make_shared<TLASVulkan>();

        m_MVPBuffer = Buffer::Create();
        BufferDesc mvpBufferDesc{};
        mvpBufferDesc.p_Size = sizeof(MVP);
        mvpBufferDesc.p_Usage = Core::BufferUsage::UniformBuffer;
        mvpBufferDesc.p_Memory = BufferDesc::MemoryUsage::CPU_To_GPU;
        mvpBufferDesc.p_Persistant = true;
        m_MVPBuffer->Init(mvpBufferDesc);

        m_BloomSettingBuffer = Buffer::Create();
        BufferDesc bloomSettingBufferDesc{};
        bloomSettingBufferDesc.p_Size = sizeof(BloomSetting);
        bloomSettingBufferDesc.p_Usage = Core::BufferUsage::UniformBuffer;
        bloomSettingBufferDesc.p_Memory = BufferDesc::MemoryUsage::CPU_To_GPU;
        bloomSettingBufferDesc.p_Persistant = true;
        m_BloomSettingBuffer->Init(bloomSettingBufferDesc);

        m_RayTracingPropsBuffer = Buffer::Create();
        BufferDesc rayTracingBufferDesc{};
        rayTracingBufferDesc.p_Size = sizeof(RayTracingProps);
        rayTracingBufferDesc.p_Usage = Core::BufferUsage::UniformBuffer;
        rayTracingBufferDesc.p_Memory = BufferDesc::MemoryUsage::CPU_To_GPU;
        rayTracingBufferDesc.p_Persistant = true;
        m_RayTracingPropsBuffer->Init(rayTracingBufferDesc);

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

            m_ClusterInfoUBO = Buffer::Create();
            BufferDesc clusterInfoBufferDesc{};
            clusterInfoBufferDesc.p_Size = sizeof(ClusterInfo);
            clusterInfoBufferDesc.p_Usage = Core::BufferUsage::UniformBuffer;
            clusterInfoBufferDesc.p_Memory = BufferDesc::MemoryUsage::CPU_To_GPU;
            clusterInfoBufferDesc.p_Persistant = true;
            m_ClusterInfoUBO->Init(clusterInfoBufferDesc);

            m_AABBGeneratorPipeline->UpdateResources("ClusterInfo", {}, m_ClusterInfoUBO, {});
            m_AABBGeneratorPipeline->UpdateResources("ClusterAABB", {}, m_ClusterTilesSSBO, {});
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

            m_AssignLightsToClustersPipeline->UpdateResources("ClusterInfo", {}, m_ClusterInfoUBO, {});
            m_AssignLightsToClustersPipeline->UpdateResources("LightsList", {}, m_LightsList, {});
            m_AssignLightsToClustersPipeline->UpdateResources("ClusterAABB", {}, m_ClusterTilesSSBO, {});
            m_AssignLightsToClustersPipeline->UpdateResources("ClusterLightIndexList", {}, m_ClusterLightIndexList, {});
            m_AssignLightsToClustersPipeline->UpdateResources("ClusterLightOffsetList", {}, m_ClusterLightOffsetList, {});
            m_AssignLightsToClustersPipeline->UpdateResources("AtomicCounters", {}, m_AtomicCounters, {});
            m_AssignLightsToClustersPipeline->UpdateResources("GlobalIndex", {}, m_GlobalIndexCountSSBO, {});
        }

        m_DepthPrePassPipeline->UpdateResources("MVP", {}, m_MVPBuffer, {});
        m_LightingPipeline->UpdateResources("BloomSetting", {}, m_BloomSettingBuffer, {});

        m_LightingPipeline->UpdateResources("sampler_Position", { m_Pos      }, nullptr, {});
        m_LightingPipeline->UpdateResources("sampler_Normal",   { m_Normal   }, nullptr, {});
        m_LightingPipeline->UpdateResources("sampler_Albedo",   { m_Albedo   }, nullptr, {});
        m_LightingPipeline->UpdateResources("sampler_Material", { m_Material }, nullptr, {});
        m_LightingPipeline->UpdateResources("sampler_Emissive", { m_Emissive }, nullptr, {});
        m_LightingPipeline->UpdateResources("sampler_Depth",    { m_DepthPre }, nullptr, {});
        m_LightingPipeline->UpdateResources("MVP",            {}, m_MVPBuffer, {});
        m_LightingPipeline->UpdateResources("ShadowMaps",       { m_ShadowMapLOD0, m_ShadowMapLOD1, m_ShadowMapLOD2, m_ShadowMapLOD3 }, nullptr, {});

        m_TransferCmdBuffer = CommandBuffer::Create();
        m_TransferCmdBuffer->Allocate(CommandBuffer::PoolType::Transfer);

        for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
            // Creating Fences
            m_ClusterFence[i] = Fence::Create();
            m_ClusterFence[i]->Init();

            m_GraphicsFence[i] = Fence::Create();
            m_GraphicsFence[i]->Init();

            m_RayTracingFence[i] = Fence::Create();
            m_RayTracingFence[i]->Init();

            // Creating Semaphores
            ImageAvailableSemaphore[i] = Semaphore::Create();
            ImageAvailableSemaphore[i]->Init();

            AABBGenerateSemaphore[i] = Semaphore::Create();
            AABBGenerateSemaphore[i]->Init();

            AssignLightsSemaphore[i] = Semaphore::Create();
            AssignLightsSemaphore[i]->Init();

            RenderFinishedSemaphore[i] = Semaphore::Create();
            RenderFinishedSemaphore[i]->Init();

            TransferFinishedSemaphore[i] = Semaphore::Create();
            TransferFinishedSemaphore[i]->Init();

            RayTracingFinishedSemaphore[i] = Semaphore::Create();
            RayTracingFinishedSemaphore[i]->Init();
            //

            // Creating Command Buffers
            m_CmdBuffer[i] = CommandBuffer::Create();
            m_CmdBuffer[i]->Allocate(CommandBuffer::PoolType::Graphics);

            m_RayTracingCmdBuffer[i] = CommandBuffer::Create();
            m_RayTracingCmdBuffer[i]->Allocate(CommandBuffer::PoolType::Graphics);

            m_ClusteredCmdBuffer[i] = CommandBuffer::Create();
            m_ClusteredCmdBuffer[i]->Allocate(CommandBuffer::PoolType::Compute);
            //
        }
        //

        // Creating Queue
        m_GraphicsQueue0 = Queue::Create();
        m_GraphicsQueue0->Init(Queue::QueueType::Graphics);

        m_GraphicsQueue1 = Queue::Create();
        m_GraphicsQueue1->Init(Queue::QueueType::Graphics);

        m_TransferQueue0 = Queue::Create();
        m_TransferQueue0->Init(Queue::QueueType::Transfer);

        m_TransferQueue1 = Queue::Create();
        m_TransferQueue1->Init(Queue::QueueType::Transfer);

        m_ComputeQueue0 = Queue::Create();
        m_ComputeQueue0->Init(Queue::QueueType::Compute);

        m_ComputeQueue1 = Queue::Create();
        m_ComputeQueue1->Init(Queue::QueueType::Compute);
        //

        m_DrawsOpaqueBuffer = Buffer::Create();
        BufferDesc drawOpBufferDesc{};
        drawOpBufferDesc.p_Name = "Draws Opaque buffer";
        drawOpBufferDesc.p_Size = SIZE_1MB * 8; // 8 MB
        drawOpBufferDesc.p_Usage = Core::BufferUsage::IndirectBuffer | Core::BufferUsage::StorageBuffer | Core::BufferUsage::TransferDst;
        drawOpBufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
        drawOpBufferDesc.p_AllowCopyDst = true;
        m_DrawsOpaqueBuffer->Init(drawOpBufferDesc);

        m_DrawsDoubleSidedBuffer = Buffer::Create();
        BufferDesc drawDSBufferDesc{};
        drawDSBufferDesc.p_Name = "Draws Double sided buffer";
        drawDSBufferDesc.p_Size = SIZE_1MB * 8; // 8 MB
        drawDSBufferDesc.p_Usage = Core::BufferUsage::IndirectBuffer | Core::BufferUsage::StorageBuffer | Core::BufferUsage::TransferDst;
        drawDSBufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
        drawDSBufferDesc.p_AllowCopyDst = true;
        m_DrawsDoubleSidedBuffer->Init(drawDSBufferDesc);

        m_DrawsBlendBuffer = Buffer::Create();
        BufferDesc drawBlendBufferDesc{};
        drawBlendBufferDesc.p_Name = "Draws blend buffer";
        drawBlendBufferDesc.p_Size = SIZE_1MB * 8; // 8 MB
        drawBlendBufferDesc.p_Usage = Core::BufferUsage::IndirectBuffer | Core::BufferUsage::StorageBuffer | Core::BufferUsage::TransferDst;
        drawBlendBufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
        drawBlendBufferDesc.p_AllowCopyDst = true;
        m_DrawsBlendBuffer->Init(drawBlendBufferDesc);

        m_DrawsBlendDoubleSidedBuffer = Buffer::Create();
        BufferDesc drawBlendDSBufferDesc{};
        drawBlendDSBufferDesc.p_Name = "Draws blend buffer";
        drawBlendDSBufferDesc.p_Size = SIZE_1MB * 8; // 8 MB
        drawBlendDSBufferDesc.p_Usage = Core::BufferUsage::IndirectBuffer | Core::BufferUsage::StorageBuffer | Core::BufferUsage::TransferDst;
        drawBlendDSBufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
        drawBlendDSBufferDesc.p_AllowCopyDst = true;
        m_DrawsBlendDoubleSidedBuffer->Init(drawBlendDSBufferDesc);

        m_MeshesBuffer = Buffer::Create();
        BufferDesc meshesBufferDesc{};
        meshesBufferDesc.p_Name = "Meshes buffer";
        meshesBufferDesc.p_Size = SIZE_1MB; // 1 MB
        meshesBufferDesc.p_Usage = Core::BufferUsage::IndirectBuffer | Core::BufferUsage::StorageBuffer | Core::BufferUsage::TransferDst;
        meshesBufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
        meshesBufferDesc.p_AllowCopyDst = true;
        m_MeshesBuffer->Init(meshesBufferDesc);

        m_IndexBuffer = Buffer::Create();
        BufferDesc indexBufferDesc{};
        indexBufferDesc.p_Name = "Index buffer";
        indexBufferDesc.p_Size = SIZE_1MB * 256; // 256 MB
        indexBufferDesc.p_Usage = Core::BufferUsage::IndexBuffer | Core::BufferUsage::TransferDst | Core::BufferUsage::StorageBuffer | Core::BufferUsage::ShaderDeviceAddress | Core::BufferUsage::AccelerationStructureBuildInputReadOnly;
        indexBufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
        indexBufferDesc.p_AllowCopyDst = true;
        m_IndexBuffer->Init(indexBufferDesc);

        m_VertexBuffer = Buffer::Create();
        BufferDesc vertexBufferDesc{};
        vertexBufferDesc.p_Name = "Vertices buffer";
        vertexBufferDesc.p_Size = SIZE_1MB * 256; // 256 MB
        vertexBufferDesc.p_Usage = Core::BufferUsage::StorageBuffer | Core::BufferUsage::TransferDst | Core::BufferUsage::ShaderDeviceAddress | Core::BufferUsage::AccelerationStructureBuildInputReadOnly;
        vertexBufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
        vertexBufferDesc.p_AllowCopyDst = true;
        m_VertexBuffer->Init(vertexBufferDesc);

        m_MeshletDataBuffer = Buffer::Create();
        BufferDesc meshletDataBufferDesc{};
        meshletDataBufferDesc.p_Name = "Meshlets buffer";
        meshletDataBufferDesc.p_Size = SIZE_1MB * 128; // 128 MB;
        meshletDataBufferDesc.p_Usage = Core::BufferUsage::StorageBuffer | Core::BufferUsage::TransferDst;
        meshletDataBufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
        meshletDataBufferDesc.p_AllowCopyDst = true;
        m_MeshletDataBuffer->Init(meshletDataBufferDesc);

        m_MeshletsBuffer = Buffer::Create();
        BufferDesc meshletBufferDesc{};
        meshletBufferDesc.p_Name = "Meshlets buffer";
        meshletBufferDesc.p_Size = SIZE_1MB * 32; // 32 MB
        meshletBufferDesc.p_Usage = Core::BufferUsage::StorageBuffer | Core::BufferUsage::TransferDst;
        meshletBufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
        meshletBufferDesc.p_AllowCopyDst = true;
        m_MeshletsBuffer->Init(meshletBufferDesc);

        m_MaterialStorageBuffer = Buffer::Create();
        BufferDesc materialsBufferDesc{};
        materialsBufferDesc.p_Name = "Materials buffer";
        materialsBufferDesc.p_Size = SIZE_1MB * 32; // 32 MB;
        materialsBufferDesc.p_Usage = Core::BufferUsage::StorageBuffer | Core::BufferUsage::TransferDst;
        materialsBufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
        materialsBufferDesc.p_AllowCopyDst = true;
        m_MaterialStorageBuffer->Init(materialsBufferDesc);

        m_TransformsBuffer = Buffer::Create();
        BufferDesc transformsBufferDesc{};
        transformsBufferDesc.p_Name = "Transforms buffer";
        transformsBufferDesc.p_Size = SIZE_100KB; // 100 KB;
        transformsBufferDesc.p_Usage = Core::BufferUsage::StorageBuffer | Core::BufferUsage::TransferDst;
        transformsBufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
        transformsBufferDesc.p_AllowCopyDst = true;
        m_TransformsBuffer->Init(transformsBufferDesc);

        m_SBT = SBT::Create();
        m_SBT->Init(m_RayTracing);

        m_GBufferPipeline->UpdateResources("Vertices", {}, m_VertexBuffer, {});
        m_GBufferPipeline->UpdateResources("MeshDraws", {}, m_DrawsOpaqueBuffer, {});
        m_GBufferPipeline->UpdateResources("MeshDrawsDS", {}, m_DrawsDoubleSidedBuffer, {});
        m_TransparentPipeline->UpdateResources("MeshDrawsBlended", {}, m_DrawsBlendDoubleSidedBuffer, {});
        m_GBufferPipeline->UpdateResources("Meshlets", {}, m_MeshletsBuffer, {});
        m_GBufferPipeline->UpdateResources("MeshletData", {}, m_MeshletDataBuffer, {});
        m_LightingPipeline->UpdateResources("u_Shadow", {}, m_ShadowDataBuffer, {});
        m_GBufferPipeline->UpdateResources("Materials", {}, m_MaterialStorageBuffer, {});
        m_GBufferPipeline->UpdateResources("Transforms", {}, m_TransformsBuffer, {});

        m_TonemappingPipeline->UpdateResources("BloomOuput", { m_BloomCombineOutput }, {}, {});

        //SceneManager::pActiveScene->LoadGltfScene("../Assets/Sponza/glTF/Sponza.gltf");
        //SceneManager::pActiveScene->LoadGltfScene("../Data/Models/gltf_models/Sponza/glTF/Sponza.gltf");
        //SceneManager::pActiveScene->LoadGltfScene("../Data/Models/mixed_workflow/scene.gltf");
        //SceneManager::pActiveScene->LoadGltfScene("../Data/Models/lancia_fulvia_rallye/scene.gltf");
        //SceneManager::pActiveScene->LoadGltfScene("../Data/Models/gltf_models/DamagedHelmet/glTF/DamagedHelmet.gltf");
        SceneManager::pActiveScene->LoadGltfScene("../Data/gltfModels/2.0/AlphaBlendModeTest/glTF/AlphaBlendModeTest.gltf");

        m_BlurVPipeline->UpdateResources("Glow", 
            { 
                m_GlowOuput,
            }, {}, {});

        m_BlurHPipeline->UpdateResources("BlurVert",
            {
                m_BloomOutputV,
            }, {}, {});

        m_BloomCombinePipeline->UpdateResources("bloom", { m_BloomOutputH, }, {}, {});
        m_BloomCombinePipeline->UpdateResources("scene", { m_LightingOutput, }, {}, {});
    }

    void Renderer::RebuildAccelerationStructures() {
        m_BLAS->Build(m_VertexBuffer, m_IndexBuffer);
        m_TLAS->Build(m_BLAS);

        //m_RayTracing->UpdateResources("Meshes", {}, m_MeshesBuffer, {});
        //m_RayTracing->UpdateResources("Indices", {}, m_IndexBuffer, {});
        //m_RayTracing->UpdateResources("topLevelAS", {}, {}, { m_TLAS });
        // TODO: Create separate output texture for ray tracing
        //m_RayTracing->UpdateResources("resultImage", { m_TonemapOutput }, {}, {});
        //m_RayTracing->UpdateResources("accumulation", { m_AccumulationImage }, {}, {});
        //m_RayTracing->UpdateResources("camera", {}, { m_RayTracingPropsBuffer }, {});
    }

    void Renderer::RenderScene(float deltaTime)
    {
        if (!SceneManager::pActiveScene) return;

        Application::GetJobSystem().ExecuteMainThreadCallbacks();
        glm::vec3 lightDir{ 4.0f, -32.0f, 12.0f };
        MVP mvp{};
        mvp.ProjView = Application::GetEditorCamera()->GetViewProjection();
        mvp.View = Application::GetEditorCamera()->GetViewMatrix();
        mvp.CamPos = Application::GetEditorCamera()->GetPosition();
        m_MVPBuffer->UpdatePersistantData(sizeof(MVP), &mvp);

        BloomSetting bloomSetting{};
        bloomSetting.knee = Application::GetRendererSettings().knee;
        bloomSetting.threshold = Application::GetRendererSettings().threshold;
        bloomSetting.intensity = Application::GetRendererSettings().intensity;
        m_BloomSettingBuffer->UpdatePersistantData(sizeof(BloomSetting), &bloomSetting);

        ClusterInfo clusterInfo{};
        clusterInfo.View = Application::GetEditorCamera()->GetViewMatrix();
        clusterInfo.InverseProj = glm::inverse(Application::GetEditorCamera()->GetProjection());
        uint32_t sizePx = (unsigned int)std::ceilf(1920 / 16.0f);
        clusterInfo.TileSizes = glm::uvec4(16, 9, 24, sizePx);
        clusterInfo.ScreenDimensions = glm::uvec4(m_LightingOutput->GetWidth(), m_LightingOutput->GetHeight(), 1, 1000);
        m_ClusterInfoUBO->UpdatePersistantData(sizeof(ClusterInfo), &clusterInfo);

        auto lightView = SceneManager::pActiveScene->Reg().view<DirectionalLightComponent>();
        for (auto e : lightView) {
            Entity entity = { e, SceneManager::pActiveScene.get() };
            auto& lc = entity.GetComponent<DirectionalLightComponent>();
            lightDir = lc.Direction;
        }

        bool cascadedShadows = true;

        if (!cascadedShadows) { // No cascades
            glm::mat4 lightProjectionMatrix, lightViewMatrix;
            glm::mat4 lightSpaceMatrix;
            float near_plane = 1.0f, far_plane = 200.0f;
            float lightSize = 10;
            lightProjectionMatrix = glm::orthoZO(-lightSize, lightSize, -lightSize, lightSize, near_plane, far_plane);
            lightProjectionMatrix[1][1] *= -1.0f;

            glm::vec3 lightDirection = normalize(-lightDir);
            glm::vec3 target = glm::vec3(0.0f);
            glm::vec3 lightPos = target - lightDirection * 20.0f;

            lightViewMatrix = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
            lightSpaceMatrix = lightProjectionMatrix * lightViewMatrix;

            m_SunMatrices[0] = lightSpaceMatrix;
            m_SunMatrices[1] = lightSpaceMatrix;
            m_SunMatrices[2] = lightSpaceMatrix;
            m_SunMatrices[3] = lightSpaceMatrix;
        }
        else { // CSM
            m_SunMatrices = getLightSpaceMatrices(glm::normalize(-lightDir));
        }

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

        if (!m_Swapchain->AcquireNextImage(UINT64_MAX, ImageAvailableSemaphore[m_CurrentFrame], nullptr, &m_ImageIndex)) {
            RecreateSwapchain();
            return;
        }

        m_GraphicsFence[m_CurrentFrame]->Reset();

        if (m_SubmitTransferWork) {
            Queue::SubmitInfo transferSubmitInfo{};
            transferSubmitInfo.pSignalSemaphores.push_back(TransferFinishedSemaphore[m_CurrentFrame]);
            transferSubmitInfo.pCmdBuffers.push_back(m_TransferCmdBuffer);

            m_TransferQueue0->Submit(transferSubmitInfo, nullptr);

            m_ScratchAllocator.Reset();
        }

        m_CmdBuffer[m_CurrentFrame]->Reset();
        m_CmdBuffer[m_CurrentFrame]->Bind();

        // --- SHADOW MAP PASS ---------------------------
        //------------------------------------------------------------------------------------------------------------------------------------------------
        uint32_t framebuffer = 0;
        m_ShadowMapPipeline->Bind(m_CmdBuffer[m_CurrentFrame]);
        for (const glm::mat4& lightMatrix : m_SunMatrices) {
            m_CSMShadowMapPass->Begin(m_CmdBuffer[m_CurrentFrame], framebuffer++);

            RenderCommand::SetViewport(m_CmdBuffer[m_CurrentFrame], 0, 0, m_ShadowMapLOD0->GetWidth(), m_ShadowMapLOD0->GetHeight(), 0, 1);
            RenderCommand::SetScissor(m_CmdBuffer[m_CurrentFrame], 0, 0, m_ShadowMapLOD0->GetWidth(), m_ShadowMapLOD0->GetHeight());

            struct PushConstant {
                glm::mat4 matrix;
                uint32_t OpaquePass;
            } pc;

            pc.matrix = lightMatrix;
            pc.OpaquePass = 1;
            m_ShadowMapPipeline->BindPushConstant(m_CmdBuffer[m_CurrentFrame], sizeof(PushConstant), &pc, 0, Core::ShaderStageFlags::Mesh);
            m_ShadowMapPipeline->Bind(m_CmdBuffer[m_CurrentFrame]);
            if (SceneManager::pActiveScene->GetDrawsOpaqueCount() != 0) {
                RenderCommand::DrawMeshTasksIndirect(m_CmdBuffer[m_CurrentFrame],
                    m_DrawsOpaqueBuffer,
                    offsetof(MeshDraw, MeshDraw::groupCountX), SceneManager::pActiveScene->GetDrawsOpaqueCount(), sizeof(MeshDraw));
            }
            pc.OpaquePass = 0;
            m_ShadowMapPipeline->BindPushConstant(m_CmdBuffer[m_CurrentFrame], sizeof(PushConstant), &pc, 0, Core::ShaderStageFlags::Mesh);
            if (SceneManager::pActiveScene->GetDrawsDoubleSidedCount() != 0) {
                RenderCommand::DrawMeshTasksIndirect(m_CmdBuffer[m_CurrentFrame],
                    m_DrawsDoubleSidedBuffer,
                    offsetof(MeshDraw, MeshDraw::groupCountX), SceneManager::pActiveScene->GetDrawsDoubleSidedCount(), sizeof(MeshDraw));
            }

            m_CSMShadowMapPass->End(m_CmdBuffer[m_CurrentFrame]);
        }
        //------------------------------------------------------------------------------------------------------------------------------------------------

        // --- DEPTH PRE PASS ---------------------------
        //------------------------------------------------------------------------------------------------------------------------------------------------
        m_DepthPrePass->Begin(m_CmdBuffer[m_CurrentFrame], m_DepthPreFramebuffer);

        RenderCommand::SetViewport(m_CmdBuffer[m_CurrentFrame], 0, 0, m_DepthPre->GetWidth(), m_DepthPre->GetHeight(), 0, 1);
        RenderCommand::SetScissor(m_CmdBuffer[m_CurrentFrame], 0, 0, m_DepthPre->GetWidth(), m_DepthPre->GetHeight());

        m_DepthPrePassPipeline->Bind(m_CmdBuffer[m_CurrentFrame]);
        glm::mat4 matrix{ 1.0f };
        m_DepthPrePassPipeline->BindPushConstant(m_CmdBuffer[m_CurrentFrame], sizeof(glm::mat4), &matrix, 0, Core::ShaderStageFlags::Mesh);
        if (SceneManager::pActiveScene->GetDrawsOpaqueCount() != 0) {
            RenderCommand::DrawMeshTasksIndirect(m_CmdBuffer[m_CurrentFrame],
                m_DrawsOpaqueBuffer,
                offsetof(MeshDraw, MeshDraw::groupCountX), SceneManager::pActiveScene->GetDrawsOpaqueCount(), sizeof(MeshDraw));
        }

        m_DepthPrePass->End(m_CmdBuffer[m_CurrentFrame]);
        //------------------------------------------------------------------------------------------------------------------------------------------------

        // --- GBUFFER PASS ---------------------------
        ////------------------------------------------------------------------------------------------------------------------------------------------------
        m_GeometryBufferPass->Begin(m_CmdBuffer[m_CurrentFrame], m_GBufferFramebuffer);
        RenderCommand::SetViewport(m_CmdBuffer[m_CurrentFrame], 0, 0, m_Pos->GetWidth(), m_Pos->GetHeight(), 0, 1);
        RenderCommand::SetScissor(m_CmdBuffer[m_CurrentFrame], 0, 0, m_Pos->GetWidth(), m_Pos->GetHeight());

        m_GBufferPipeline->Bind(m_CmdBuffer[m_CurrentFrame]);
        uint32_t OpaquePass = 1;
        m_GBufferPipeline->BindPushConstant(m_CmdBuffer[m_CurrentFrame], sizeof(uint32_t), &OpaquePass, 0, Core::ShaderStageFlags::Mesh | Core::ShaderStageFlags::Fragment);
        if (SceneManager::pActiveScene->GetDrawsOpaqueCount() != 0) {
            RenderCommand::DrawMeshTasksIndirect(m_CmdBuffer[m_CurrentFrame],
                m_DrawsOpaqueBuffer,
                offsetof(MeshDraw, MeshDraw::groupCountX), SceneManager::pActiveScene->GetDrawsOpaqueCount(), sizeof(MeshDraw));
        }

        m_GBufferDoubleSidedPipeline->Bind(m_CmdBuffer[m_CurrentFrame]);
        OpaquePass = 0;
        m_GBufferPipeline->BindPushConstant(m_CmdBuffer[m_CurrentFrame], sizeof(uint32_t), &OpaquePass, 0, Core::ShaderStageFlags::Mesh | Core::ShaderStageFlags::Fragment);
        if (SceneManager::pActiveScene->GetDrawsDoubleSidedCount() != 0) {
            RenderCommand::DrawMeshTasksIndirect(m_CmdBuffer[m_CurrentFrame],
                m_DrawsDoubleSidedBuffer,
                offsetof(MeshDraw, MeshDraw::groupCountX), SceneManager::pActiveScene->GetDrawsDoubleSidedCount(), sizeof(MeshDraw));
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
        m_LightingPass->Begin(m_CmdBuffer[m_CurrentFrame], m_LightingFramebuffer);
        m_LightingPipeline->Bind(m_CmdBuffer[m_CurrentFrame]);

        RenderCommand::SetViewport(m_CmdBuffer[m_CurrentFrame], 0, 0, m_LightingOutput->GetWidth(), m_LightingOutput->GetHeight(), 0, 1);
        RenderCommand::SetScissor(m_CmdBuffer[m_CurrentFrame], 0, 0, m_LightingOutput->GetWidth(), m_LightingOutput->GetHeight());

        m_LightingPipeline->BindPushConstant(m_CmdBuffer[m_CurrentFrame], sizeof(glm::vec3), &lightDir, 0, Core::ShaderStageFlags::Fragment);
        RenderCommand::Draw(m_CmdBuffer[m_CurrentFrame], 3, 0);

        m_LightingPass->End(m_CmdBuffer[m_CurrentFrame]);
        ////------------------------------------------------------------------------------------------------------------------------------------------------

        //// --- FORWARD TRANSPARENT PASS ---------------------------
        ////------------------------------------------------------------------------------------------------------------------------------------------------
        m_TransparentPass->Begin(m_CmdBuffer[m_CurrentFrame], m_TransparentFramebuffer);
        m_TransparentPipeline->Bind(m_CmdBuffer[m_CurrentFrame]);

        RenderCommand::SetViewport(m_CmdBuffer[m_CurrentFrame], 0, 0, m_LightingOutput->GetWidth(), m_LightingOutput->GetHeight(), 0, 1);
        RenderCommand::SetScissor(m_CmdBuffer[m_CurrentFrame], 0, 0, m_LightingOutput->GetWidth(), m_LightingOutput->GetHeight());

        m_TransparentPipeline->BindPushConstant(m_CmdBuffer[m_CurrentFrame], sizeof(glm::vec3), &lightDir, 0, Core::ShaderStageFlags::Fragment);
        if (SceneManager::pActiveScene->GetDrawsBlendDSCount() != 0) {
            RenderCommand::DrawMeshTasksIndirect(m_CmdBuffer[m_CurrentFrame],
                m_DrawsBlendDoubleSidedBuffer,
                offsetof(MeshDraw, MeshDraw::groupCountX), SceneManager::pActiveScene->GetDrawsBlendDSCount(), sizeof(MeshDraw));
        }

        m_TransparentPass->End(m_CmdBuffer[m_CurrentFrame]);
        ////------------------------------------------------------------------------------------------------------------------------------------------------

        //// --- BLOOM BLUR V PASS ---------------------------
        ////------------------------------------------------------------------------------------------------------------------------------------------------
        m_BloomBlurPass->Begin(m_CmdBuffer[m_CurrentFrame], m_BloomBlurVFramebuffer);
        m_BlurVPipeline->Bind(m_CmdBuffer[m_CurrentFrame]);
        RenderCommand::SetViewport(m_CmdBuffer[m_CurrentFrame], 0, 0, m_BloomOutputV->GetWidth(), m_BloomOutputV->GetHeight(), 0, 1);
        RenderCommand::SetScissor(m_CmdBuffer[m_CurrentFrame], 0, 0, m_BloomOutputV->GetWidth(), m_BloomOutputV->GetHeight());

        RenderCommand::Draw(m_CmdBuffer[m_CurrentFrame], 3, 0);

        m_BloomBlurPass->End(m_CmdBuffer[m_CurrentFrame]);
        ////------------------------------------------------------------------------------------------------------------------------------------------------

        //// --- BLOOM BLUR H PASS ---------------------------
        ////------------------------------------------------------------------------------------------------------------------------------------------------
        m_BloomBlurPass->Begin(m_CmdBuffer[m_CurrentFrame], m_BloomBlurHFramebuffer);
        m_BlurHPipeline->Bind(m_CmdBuffer[m_CurrentFrame]);
        RenderCommand::SetViewport(m_CmdBuffer[m_CurrentFrame], 0, 0, m_BloomOutputH->GetWidth(), m_BloomOutputH->GetHeight(), 0, 1);
        RenderCommand::SetScissor(m_CmdBuffer[m_CurrentFrame], 0, 0, m_BloomOutputH->GetWidth(), m_BloomOutputH->GetHeight());

        RenderCommand::Draw(m_CmdBuffer[m_CurrentFrame], 3, 0);

        m_BloomBlurPass->End(m_CmdBuffer[m_CurrentFrame]);
        ////------------------------------------------------------------------------------------------------------------------------------------------------

        //// --- BLOOM COMBINE PASS ---------------------------
        ////------------------------------------------------------------------------------------------------------------------------------------------------
        m_BloomCombinePass->Begin(m_CmdBuffer[m_CurrentFrame], m_BloomCombineFramebuffer);
        m_BloomCombinePipeline->Bind(m_CmdBuffer[m_CurrentFrame]);
        RenderCommand::SetViewport(m_CmdBuffer[m_CurrentFrame], 0, 0, m_BloomCombineOutput->GetWidth(), m_BloomCombineOutput->GetHeight(), 0, 1);
        RenderCommand::SetScissor(m_CmdBuffer[m_CurrentFrame], 0, 0, m_BloomCombineOutput->GetWidth(), m_BloomCombineOutput->GetHeight());

        RenderCommand::Draw(m_CmdBuffer[m_CurrentFrame], 3, 0);

        m_BloomCombinePass->End(m_CmdBuffer[m_CurrentFrame]);
        ////------------------------------------------------------------------------------------------------------------------------------------------------

        //{
        //    Texture::ImageBarrierParams params{};
        //    params.oldLayout = Core::ImageLayout::ColorAttachmentOptimal;
        //    params.newLayout = Core::ImageLayout::ShaderReadOnlyOptimal;
        //    params.srcAccess = Core::AccessType::ColorAttachmentWrite;
        //    params.dstAccess = Core::AccessType::ShaderRead;
        //    params.srcStage = Core::PipelineStage::ColorAttachment;
        //    params.dstStage = Core::PipelineStage::FragmentShader;
        //    m_BloomCombineOutput->TransitionImageLayout(m_CmdBuffer[m_CurrentFrame], { params });
        //}

        //// --- TONEMAP PASS ---------------------------
        ////------------------------------------------------------------------------------------------------------------------------------------------------
        m_TonemappingPass->Begin(m_CmdBuffer[m_CurrentFrame], m_TonemappingFramebuffer);
        m_TonemappingPipeline->Bind(m_CmdBuffer[m_CurrentFrame]);

        RenderCommand::SetViewport(m_CmdBuffer[m_CurrentFrame], 0, 0, m_TonemapOutput->GetWidth(), m_TonemapOutput->GetHeight(), 0, 1);
        RenderCommand::SetScissor(m_CmdBuffer[m_CurrentFrame], 0, 0, m_TonemapOutput->GetWidth(), m_TonemapOutput->GetHeight());

        RenderCommand::Draw(m_CmdBuffer[m_CurrentFrame], 3, 0);

        m_TonemappingPass->End(m_CmdBuffer[m_CurrentFrame]);
        ////------------------------------------------------------------------------------------------------------------------------------------------------

        {
            Texture::ImageBarrierParams params{};
            params.oldLayout = Core::ImageLayout::ShaderReadOnlyOptimal;
            params.newLayout = Core::ImageLayout::ColorAttachmentOptimal;
            params.srcAccess = Core::AccessType::ShaderRead;
            params.dstAccess = Core::AccessType::ColorAttachmentWrite;
            params.srcStage = Core::PipelineStage::FragmentShader;
            params.dstStage = Core::PipelineStage::ColorAttachment;
            m_LightingOutput->TransitionImageLayout(m_CmdBuffer[m_CurrentFrame], { params });
        }

        //// --- UI PASS ---------------------------
        ////------------------------------------------------------------------------------------------------------------------------------------------------
        m_UIPass->Begin(m_CmdBuffer[m_CurrentFrame], m_UIFramebuffers[m_ImageIndex]);

        RenderCommand::SetViewport(m_CmdBuffer[m_CurrentFrame], 0, 0, m_Swapchain->GetExtentWidth(), m_Swapchain->GetExtentHeight(), 0, 1);
        RenderCommand::SetScissor(m_CmdBuffer[m_CurrentFrame], 0, 0, m_Swapchain->GetExtentWidth(), m_Swapchain->GetExtentHeight());

        Application::GetGuiLayer()->Render(m_CmdBuffer[m_CurrentFrame]);

        m_UIPass->End(m_CmdBuffer[m_CurrentFrame]);
        ////------------------------------------------------------------------------------------------------------------------------------------------------

        m_CmdBuffer[m_CurrentFrame]->UnBind();

        Queue::SubmitInfo lightingSubmitInfo{};
        if (m_SubmitTransferWork) {
            lightingSubmitInfo.pWaitSemaphores.push_back(TransferFinishedSemaphore[m_CurrentFrame]);
            lightingSubmitInfo.pWaitStages.push_back(Core::PipelineStage::TransferStage);
            m_SubmitTransferWork = false;
        }
        lightingSubmitInfo.pWaitSemaphores.push_back(ImageAvailableSemaphore[m_CurrentFrame]);
        lightingSubmitInfo.pWaitSemaphores.push_back(AssignLightsSemaphore[m_CurrentFrame]);
        lightingSubmitInfo.pSignalSemaphores.push_back(RenderFinishedSemaphore[m_CurrentFrame]);
        lightingSubmitInfo.pWaitStages.push_back(Core::PipelineStage::ColorAttachment);
        lightingSubmitInfo.pWaitStages.push_back(Core::PipelineStage::EarlyFragmentTest);
        lightingSubmitInfo.pCmdBuffers.push_back(m_CmdBuffer[m_CurrentFrame]);

        {
            std::lock_guard<std::mutex> lock(g_GraphicsQueueMutex);
            m_GraphicsQueue0->Submit(lightingSubmitInfo, m_GraphicsFence[m_CurrentFrame]);
        }

        Queue::PresentInfo presentInfo{};
        presentInfo.pWaitSemaphores.push_back(RenderFinishedSemaphore[m_CurrentFrame]);
        presentInfo.pSwapchains.push_back(m_Swapchain);
        presentInfo.pImageIndex = m_ImageIndex;

        // Present
        {
            std::lock_guard<std::mutex> lock(g_GraphicsQueueMutex);
            m_GraphicsQueue0->Present(presentInfo);
        }

        if (m_WindowResized) {
            RecreateSwapchain();
            m_WindowResized = false;
        }

        m_CurrentFrame = (m_CurrentFrame + 1) % FRAMES_IN_FLIGHT;
    }

    void Renderer::RenderRT(float deltaTime)
    {
        if (!SceneManager::pActiveScene) return;

        Application::GetJobSystem().ExecuteMainThreadCallbacks();
        glm::vec3 lightDir{ 4.0f, -32.0f, 12.0f };

        auto lightView = SceneManager::pActiveScene->Reg().view<DirectionalLightComponent>();
        for (auto e : lightView) {
            Entity entity = { e, SceneManager::pActiveScene.get() };
            auto& lc = entity.GetComponent<DirectionalLightComponent>();
            lightDir = lc.Direction;
        }

        RayTracingProps rayProps{};
        rayProps.ProjInv = glm::inverse(Application::GetEditorCamera()->GetProjection());
        rayProps.ViewInv = glm::inverse(Application::GetEditorCamera()->GetViewMatrix());
        rayProps.LightPos = lightDir;
        rayProps.CamPos = Application::GetEditorCamera()->GetPosition();
        rayProps.dimension = glm::vec2(1920, 1080);
        rayProps.frame = Application::GetEditorCamera()->GetFrameIndex();
        m_RayTracingPropsBuffer->UpdatePersistantData(sizeof(RayTracingProps), &rayProps);

        if (!m_Swapchain->AcquireNextImage(UINT64_MAX, ImageAvailableSemaphore[m_CurrentFrame], nullptr, &m_ImageIndex)) {
            RecreateSwapchain();
            return;
        }

        if (m_SubmitTransferWork) {
            Queue::SubmitInfo transferSubmitInfo{};
            transferSubmitInfo.pSignalSemaphores.push_back(TransferFinishedSemaphore[m_CurrentFrame]);
            transferSubmitInfo.pCmdBuffers.push_back(m_TransferCmdBuffer);

            m_TransferQueue0->Submit(transferSubmitInfo, nullptr);

            m_ScratchAllocator.Reset();
        }

        m_RayTracingCmdBuffer[m_CurrentFrame]->Reset();
        m_RayTracingCmdBuffer[m_CurrentFrame]->Bind();

        //// --- RAY TRACING PASS ---------------------------
        ////------------------------------------------------------------------------------------------------------------------------------------------------
        {
            Texture::ImageBarrierParams params{};
            params.oldLayout = Core::ImageLayout::Undefined;
            params.newLayout = Core::ImageLayout::General;
            params.srcAccess = Core::AccessType::ShaderWrite;
            params.dstAccess = Core::AccessType::ShaderWrite;
            params.srcStage = Core::PipelineStage::RayTracing;
            params.dstStage = Core::PipelineStage::RayTracing;

            m_LightingOutput->TransitionImageLayout(m_RayTracingCmdBuffer[m_CurrentFrame], { params });
        }

        m_RayTracingFence[m_CurrentFrame]->Wait();
        m_RayTracingFence[m_CurrentFrame]->Reset();

        m_RayTracing->Bind(m_RayTracingCmdBuffer[m_CurrentFrame]);

        RenderCommand::TraceRays(m_RayTracingCmdBuffer[m_CurrentFrame], m_SBT, 1920, 1080);
        m_RayTracingCmdBuffer[m_CurrentFrame]->UnBind();
        Queue::SubmitInfo rayTracingSubmitInfo{};
        rayTracingSubmitInfo.pCmdBuffers.push_back(m_RayTracingCmdBuffer[m_CurrentFrame]);
        rayTracingSubmitInfo.pSignalSemaphores.push_back(RayTracingFinishedSemaphore[m_CurrentFrame]);

        {
            std::lock_guard<std::mutex> lock(g_GraphicsQueueMutex);
            m_GraphicsQueue1->Submit(rayTracingSubmitInfo, m_RayTracingFence[m_CurrentFrame]);
        }

        m_RayTracingFence[m_CurrentFrame]->Wait();
        m_RayTracingFence[m_CurrentFrame]->Reset();

        m_RayTracingCmdBuffer[m_CurrentFrame]->Reset();
        m_RayTracingCmdBuffer[m_CurrentFrame]->Bind();

        ////------------------------------------------------------------------------------------------------------------------------------------------------

        {
            Texture::ImageBarrierParams params{};
            params.oldLayout = Core::ImageLayout::General;
            params.newLayout = Core::ImageLayout::ShaderReadOnlyOptimal;
            params.srcAccess = Core::AccessType::ShaderWrite;
            params.dstAccess = Core::AccessType::ShaderRead;
            params.srcStage = Core::PipelineStage::RayTracing;
            params.dstStage = Core::PipelineStage::FragmentShader;    
            m_LightingOutput->TransitionImageLayout(m_RayTracingCmdBuffer[m_CurrentFrame], { params });
        }

        //// --- UI PASS ---------------------------
        ////------------------------------------------------------------------------------------------------------------------------------------------------
        m_UIPass->Begin(m_RayTracingCmdBuffer[m_CurrentFrame], m_UIFramebuffers[m_ImageIndex]);

        RenderCommand::SetViewport(m_RayTracingCmdBuffer[m_CurrentFrame], 0, 0, m_Swapchain->GetExtentWidth(), m_Swapchain->GetExtentHeight(), 0, 1);
        RenderCommand::SetScissor(m_RayTracingCmdBuffer[m_CurrentFrame], 0, 0, m_Swapchain->GetExtentWidth(), m_Swapchain->GetExtentHeight());

        Application::GetGuiLayer()->Render(m_RayTracingCmdBuffer[m_CurrentFrame]);

        m_UIPass->End(m_RayTracingCmdBuffer[m_CurrentFrame]);
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

        m_RayTracingCmdBuffer[m_CurrentFrame]->UnBind();

        Queue::SubmitInfo submitInfo{};
        if (m_SubmitTransferWork) {
            submitInfo.pWaitSemaphores.push_back(TransferFinishedSemaphore[m_CurrentFrame]);
            submitInfo.pWaitStages.push_back(Core::PipelineStage::TransferStage);
            m_SubmitTransferWork = false;
        }
        submitInfo.pWaitSemaphores.push_back(RayTracingFinishedSemaphore[m_CurrentFrame]);
        submitInfo.pWaitStages.push_back(Core::PipelineStage::RayTracing);
        submitInfo.pWaitSemaphores.push_back(ImageAvailableSemaphore[m_CurrentFrame]);
        submitInfo.pSignalSemaphores.push_back(RenderFinishedSemaphore[m_CurrentFrame]);
        submitInfo.pWaitStages.push_back(Core::PipelineStage::ColorAttachment);
        submitInfo.pCmdBuffers.push_back(m_RayTracingCmdBuffer[m_CurrentFrame]);

        {
            std::lock_guard<std::mutex> lock(g_GraphicsQueueMutex);
            m_GraphicsQueue0->Submit(submitInfo, m_RayTracingFence[m_CurrentFrame]);
        }

        Queue::PresentInfo presentInfo{};
        presentInfo.pWaitSemaphores.push_back(RenderFinishedSemaphore[m_CurrentFrame]);
        presentInfo.pSwapchains.push_back(m_Swapchain);
        presentInfo.pImageIndex = m_ImageIndex;

        // Present
        {
            std::lock_guard<std::mutex> lock(g_GraphicsQueueMutex);
            m_GraphicsQueue0->Present(presentInfo);
        }

        if (m_WindowResized) {
            RecreateSwapchain();
            m_WindowResized = false;
        }

        m_CurrentFrame = (m_CurrentFrame + 1) % FRAMES_IN_FLIGHT;
    }

    void Renderer::RecreateSwapchain() {
        Application::GetGpuAdapter()->WaitIdle();
        m_UIPass->Release();
        m_Swapchain->Release();

        m_Swapchain->Create(swapchainMode);

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
                    RenderPassAttachment{ 0, AttachmentType::Swapchain, LoadOp::Clear, StoreOp::Store, Core::Format::FORMAT_R8G8B8A8_UNORM, Core::ImageLayout::Undefined, Core::ImageLayout::PresentSrc }
                }
        );
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
        m_TransparentPipeline->Release();
        m_LightingPipeline->Release();

        m_Swapchain->Release();

        //ImageAvailableSemaphore->Release();
        //RenderFinishedSemaphore->Release();

        //m_ClusterFence->Release();
        //m_GraphicsFence->Release();
    }

    std::unique_ptr<Renderer> Renderer::Create()
    {
        return std::make_unique<Renderer>();
    }
}
