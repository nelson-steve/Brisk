#pragma once

#include "CommandBufferDirectX12.hpp"
#include "Engine/Renderer/RenderPass.hpp"

#include <Volk/volk.h>
#include <iostream>
#include <cassert>

namespace Brisk
{
    class RenderPassDirectX12 : public RenderPass {
    public:
        RenderPassDirectX12() = default;

        virtual void Init(const std::vector<RenderPassDependency>& dependencies, const std::vector<RenderPassAttachment>& outputs) override;
        virtual void Release() override { assert(false); }

        virtual void Begin(std::shared_ptr<CommandBuffer> cmd, uint32_t imageIndex = 0) override;
        virtual void End(std::shared_ptr<CommandBuffer> cmd) override;
    private:
        std::vector<D3D12_RENDER_PASS_RENDER_TARGET_DESC> m_RenderTargets;
        D3D12_RENDER_PASS_DEPTH_STENCIL_DESC m_DepthStencil{};

        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_RTVHandles;
        D3D12_CPU_DESCRIPTOR_HANDLE m_DSVHandle;
    };
}