#pragma once

// INCLUDES
#include "RenderPass.hpp"
#include "Descriptor.hpp"
#include "Core/Core.hpp"
#include "GpuAdapter.hpp"
#include "CSMRenderPass.hpp"
//--------------------------
#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <optional>
//---------------

namespace Brisk
{
    class Pipeline {
        DEFINE_BASE_CLASS_CONSTRUCTOR(Pipeline)
    public:
        struct Attribute {
            uint32_t pBinding;
            uint32_t pLocation;
            Core::Format pFormat;
            uint32_t pOffset;
        };

        struct VertexDataLayout {
            uint32_t pBinding;
            uint32_t pStride;
            std::vector<Attribute> pAttributes;
        };

        enum CullMode {
            NONE,
            BACK,
            FRONT,
        };

        enum FrontFace {
            COUTNER_CLOCKWISE,
            CLOCKWISE,
        };

        enum PolygonMode {
            POLYGON_MODE_FILL = 0,
            POLYGON_MODE_LINE = 1,
            POLYGON_MODE_POINT = 2,
        };

        enum CompareOp {
            COMPARE_OP_NEVER = 0,
            COMPARE_OP_LESS = 1,
            COMPARE_OP_EQUAL = 2,
            COMPARE_OP_LESS_OR_EQUAL = 3,
            COMPARE_OP_GREATER = 4,
            COMPARE_OP_NOT_EQUAL = 5,
            COMPARE_OP_GREATER_OR_EQUAL = 6,
            COMPARE_OP_ALWAYS = 7,
        };

        struct GraphicsPipelineSpecs {
            std::optional<VertexDataLayout> pLayout;
            bool pDepthClampEnable;
            bool pRasterizationDiscardEnable;
            PolygonMode pPolygoneMode;
            float pLineWidth;
            CullMode pCullMode;
            FrontFace pFrontFace;
            bool pDepthBiasEnable;
            bool pDepthTestEnable;
            bool pDepthWriteEnable;
            CompareOp pCompareOp;
            bool pDepthBoundsTestEnable;
            bool pStencilTestEnable;
            bool pTransparent;
            std::string pDebugName;
            std::shared_ptr<RenderPass> pRenderPass;
            std::shared_ptr<CSMRenderPass> pCSMRenderPass;
            std::vector<std::string> pShaderPathsVK;
            std::vector<std::string> pShaderPathsDX;
        };

        struct ComputePipelineSpecs {
            std::string pShaderPath;
        };

        //typedef struct VkRayTracingPipelineCreateInfoKHR {
        //    VkStructureType                                      sType;
        //    const void* pNext;
        //    VkPipelineCreateFlags                                flags;
        //    uint32_t                                             stageCount;
        //    const VkPipelineShaderStageCreateInfo* pStages;
        //    uint32_t                                             groupCount;
        //    const VkRayTracingShaderGroupCreateInfoKHR* pGroups;
        //    uint32_t                                             maxPipelineRayRecursionDepth;
        //    const VkPipelineLibraryCreateInfoKHR* pLibraryInfo;
        //    const VkRayTracingPipelineInterfaceCreateInfoKHR* pLibraryInterface;
        //    const VkPipelineDynamicStateCreateInfo* pDynamicState;
        //    VkPipelineLayout                                     layout;
        //    VkPipeline                                           basePipelineHandle;
        //    int32_t                                              basePipelineIndex;
        //} VkRayTracingPipelineCreateInfoKHR;

        struct RayTracingPipelineSpecs {
            std::vector<std::string> pShaderPathsVK;
            std::vector<std::string> pShaderPathsDX;
        };
    public:
        virtual void Init(const GraphicsPipelineSpecs& specs) = 0;
        virtual void Init(const ComputePipelineSpecs& specs) = 0;
        virtual void Init(const RayTracingPipelineSpecs& specs) = 0;
        virtual void Release() = 0;

        virtual void UpdateResources(const std::string& name, std::vector<std::shared_ptr<Texture>> textures, std::shared_ptr<Buffer> buffer) = 0;
        virtual void Bind(std::shared_ptr<CommandBuffer> cmd) = 0;
        virtual void BindPushConstant(std::shared_ptr<CommandBuffer> cmd, uint32_t size, void* data, uint32_t offset, Core::ShaderStageFlags stages) = 0;

        static std::shared_ptr<Pipeline> Create();

        // TODO: Shouldn't be public
        GraphicsPipelineSpecs m_GraphicsSpecs;
        ComputePipelineSpecs m_ComputeSpecs;
    };
}