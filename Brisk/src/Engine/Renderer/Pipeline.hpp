#pragma once

// INCLUDES
#include "RenderPass.hpp"
#include "Descriptor.hpp"
#include "Core/Core.hpp"
#include "GpuAdapter.hpp"
//------------------------
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

        enum ShaderStage {
            VERTEX,
            GEOMETRY,
            COMPUTE,
            FRAGMENT,
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
            std::shared_ptr<RenderPass> pRenderPass;
            std::vector<std::string> pShaderPaths;
        };

        struct ComputePipelineSpecs {
            std::shared_ptr<ShaderModule> pShaderModule;
            std::vector<std::shared_ptr<DescriptorLayout>> pDescriptorLayouts;
        };
    public:
        virtual void Init(const GraphicsPipelineSpecs& specs) = 0;
        virtual void Init(const ComputePipelineSpecs& specs) = 0;

        virtual void Bind(std::shared_ptr<CommandBuffer> cmd) = 0;

        virtual void BindPushConstant(std::shared_ptr<CommandBuffer> cmd, uint32_t, void* data) = 0;

        virtual void Destroy() = 0;

        static std::shared_ptr<Pipeline> Create();

        // TODO: Shouldn't be public
        GraphicsPipelineSpecs m_GraphicsSpecs;
        ComputePipelineSpecs m_ComputeSpecs;
    protected:
        std::vector<GpuDescriptorResourceType> p_ResourceTypes;
    };
}