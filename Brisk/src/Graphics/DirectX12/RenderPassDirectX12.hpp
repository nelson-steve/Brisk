#pragma once

#include "CommandBufferDirectX12.hpp"
#include "Engine/Renderer/RenderPass.hpp"

namespace Brisk 
{
    class RenderPassDirectX12 : public RenderPass {
    public:
        virtual void Init(const RenderPassSpecs& specs) override;
        virtual void Begin(std::shared_ptr<CommandBuffer> cmd, uint32_t imageIndex) override;
        virtual void End(std::shared_ptr<CommandBuffer> cmd) override;

        virtual void AddRenderTarget(std::shared_ptr<Swapchain> swapchain) override;
        virtual void AddRenderTarget(std::shared_ptr<Texture> texture) override;

        ID3D12PipelineState* GetPipelineState() const { return m_PipelineState.Get(); }
        ID3D12RootSignature* GetRootSignature() const { return m_RootSignature.Get(); }

    private:
        Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineState;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
        std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_RenderTargets;
    };
}
