#pragma once

#include "RenderPass.hpp"

#include <cstdint>
#include <vector>
#include <string>
#include <memory>

namespace Brisk
{
    class Pipeline {
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

        };

        enum CompareOp {

        };

        enum ShaderStage {
            VERTEX,
            FRAGMENT,
        };

        struct PipelineSpecs {
            VertexDataLayout Layout;
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
            std::vector<std::pair<std::string, ShaderStage>> pShaders;
        };
    public:
        std::shared_ptr<Pipeline> Create(const PipelineSpecs& specs);

        virtual void Bind() = 0;
        virtual void UnBind() = 0;

        virtual void Destroy() = 0;
    };
}