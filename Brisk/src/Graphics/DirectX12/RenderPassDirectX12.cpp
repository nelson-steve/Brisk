#include "RenderPassDirectX12.hpp"
#include "TextureDirectX12.hpp"

namespace Brisk
{
    void RenderPassDirectX12::Init(const std::vector<RenderPassDependency>& dependencies, const std::vector<RenderPassAttachment>& outputs) {
        m_RenderTargets.clear();
        m_HasDepth = false;

        for (const auto& attachment : outputs)
        {
            auto texture = std::static_pointer_cast<TextureDirectX12>(attachment.pImage);

            if (attachment.pAttachmentType == AttachmentType::Color)
            {
                D3D12_RENDER_PASS_RENDER_TARGET_DESC rtDesc{};
                rtDesc.cpuDescriptor = texture->GetSRVCPU();
                m_RTVHandles.push_back(texture->GetSRVCPU());
                rtDesc.BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
                //rtDesc.BeginningAccess.Clear.ClearValue = texture->GetClearValue();
                rtDesc.EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;

                m_RenderTargets.push_back(rtDesc);
            }
            else if (attachment.pAttachmentType == AttachmentType::Depth)
            {
                m_HasDepth = true;
                m_DepthStencil.cpuDescriptor = texture->GetSRVCPU();
                m_DepthStencil.DepthBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
                //m_DepthStencil.DepthBeginningAccess.Clear.ClearValue = texture->GetClearValue();
                m_DepthStencil.DepthEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;

                m_DepthStencil.StencilBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS;
                m_DepthStencil.StencilEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS;
            }
        }
    }

    void RenderPassDirectX12::Begin(std::shared_ptr<CommandBuffer> cmd) {
        ComPtr<ID3D12GraphicsCommandList> commandList = std::static_pointer_cast<CommandBufferDirectX12>(cmd)->Get();

        commandList->OMSetRenderTargets(static_cast<UINT>(m_RTVHandles.size()), m_RTVHandles.data(), FALSE,
            m_HasDepth ? &m_DSVHandle : nullptr);

        for (UINT i = 0; i < m_RTVHandles.size(); ++i) {
            FLOAT clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            commandList->ClearRenderTargetView(m_RTVHandles[i], clearColor, 0, nullptr);
        }

        if (m_HasDepth) {
            commandList->ClearDepthStencilView(m_DSVHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
        }

        //commandList->RSSetViewports(1, &m_Viewport);
        //commandList->RSSetScissorRects(1, &m_ScissorRect);
    }

    void RenderPassDirectX12::End(std::shared_ptr<CommandBuffer> cmd) {

    }
}