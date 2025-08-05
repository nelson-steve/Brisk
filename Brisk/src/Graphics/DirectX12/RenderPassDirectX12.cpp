#include "RenderPassDirectX12.hpp"
#include "TextureDirectX12.hpp"
#include "UtilitiesDirectX12.hpp"

#include <directx/d3d12.h>

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
                D3D12_CLEAR_VALUE clearValue = {};
                clearValue.Format = UtilitiesDirectX12::FormatToDXGIFormat(texture->GetSpecs().p_Format);
                clearValue.Color[0] = 1.0f; clearValue.Color[1] = 0.0f;
                clearValue.Color[2] = 0.0f; clearValue.Color[3] = 1.0f;
                rtDesc.BeginningAccess.Clear.ClearValue = clearValue;
                rtDesc.EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;

                m_RenderTargets.push_back(rtDesc);
            }
            else if (attachment.pAttachmentType == AttachmentType::Depth)
            {
                m_DSVHandle = texture->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();

                m_HasDepth = true;
                m_DepthStencil.cpuDescriptor = m_DSVHandle;
                m_DepthStencil.DepthBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
                D3D12_CLEAR_VALUE clearValue = {};
                clearValue.Format = UtilitiesDirectX12::FormatToDXGIFormat(texture->GetSpecs().p_Format);
                clearValue.Color[0] = 1.0f; clearValue.Color[1] = 0.0f; // Red
                clearValue.Color[2] = 0.0f; clearValue.Color[3] = 1.0f;
                m_DepthStencil.DepthBeginningAccess.Clear.ClearValue = clearValue;
                m_DepthStencil.DepthEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;

                m_DepthStencil.StencilBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS;
                m_DepthStencil.StencilEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS;
            }
        }
    }

    void RenderPassDirectX12::Begin(std::shared_ptr<CommandBuffer> cmd, uint32_t imageIndex) {
        ComPtr<ID3D12GraphicsCommandList6> commandList = std::static_pointer_cast<CommandBufferDirectX12>(cmd)->Get();
        commandList->BeginRenderPass(
            0,
            nullptr,
            &m_DepthStencil,
            D3D12_RENDER_PASS_FLAG_NONE
        );

        commandList->OMSetRenderTargets(
            0,
            nullptr,
            FALSE,
            &m_DSVHandle
        );

        commandList->ClearDepthStencilView(
            m_DSVHandle,
            D3D12_CLEAR_FLAG_DEPTH,
            1.0f,
            0,
            0,
            nullptr
        );

    }

    void RenderPassDirectX12::End(std::shared_ptr<CommandBuffer> cmd) {
        ComPtr<ID3D12GraphicsCommandList6> commandList = std::static_pointer_cast<CommandBufferDirectX12>(cmd)->Get();
        commandList->EndRenderPass();
    }
}