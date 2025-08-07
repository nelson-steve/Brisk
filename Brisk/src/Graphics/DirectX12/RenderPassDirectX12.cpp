#include "RenderPassDirectX12.hpp"
#include "TextureDirectX12.hpp"
#include "UtilitiesDirectX12.hpp"
#include "Engine/Engine.hpp"
#include "SwapchainDirectX12.hpp"

#include <directx/d3d12.h>

namespace Brisk
{
    void RenderPassDirectX12::Init(const std::vector<RenderPassDependency>& dependencies, const std::vector<RenderPassAttachment>& outputs) {
        m_RenderTargets.clear();
        m_HasDepth = false;

        for (const auto& attachment : outputs)
        {
            if (attachment.pAttachmentType == AttachmentType::Color)
            {
                auto texture = std::static_pointer_cast<TextureDirectX12>(attachment.pImage);

                D3D12_RENDER_PASS_RENDER_TARGET_DESC rtDesc{};
                rtDesc.cpuDescriptor = texture->GetRtvCPU();
                m_RTVHandles.push_back(texture->GetRtvCPU());

                if (attachment.pLoadOp == LoadOp::Clear)
                    rtDesc.BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
                else if (attachment.pLoadOp == LoadOp::Load)
                    rtDesc.BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
                else if (attachment.pLoadOp == LoadOp::DontCare)
                    rtDesc.BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;

                D3D12_CLEAR_VALUE clearValue = {};
                clearValue.Format = UtilitiesDirectX12::FormatToDXGIFormat(texture->GetSpecs().p_Format);
                clearValue.Color[0] = 1.0f; clearValue.Color[1] = 0.0f;
                clearValue.Color[2] = 0.0f; clearValue.Color[3] = 1.0f;
                rtDesc.BeginningAccess.Clear.ClearValue = clearValue;

                if (attachment.pStoreOp == StoreOp::Store)
                    rtDesc.EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
                else if (attachment.pStoreOp == StoreOp::DontCare)
                    rtDesc.EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;

                m_ColorAttachmentCount++;
                m_RenderTargets.push_back(rtDesc);
            }
            else if (attachment.pAttachmentType == AttachmentType::Depth)
            {
                auto texture = std::static_pointer_cast<TextureDirectX12>(attachment.pImage);

                m_DSVHandle = texture->GetDsvCPU();

                m_HasDepth = true;
                m_DepthStencil.cpuDescriptor = m_DSVHandle;
                m_DepthStencil.DepthBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;

                if (attachment.pLoadOp == LoadOp::Clear)
                    m_DepthStencil.DepthBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
                else if (attachment.pLoadOp == LoadOp::Load)
                    m_DepthStencil.DepthBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
                else if (attachment.pLoadOp == LoadOp::DontCare)
                    m_DepthStencil.DepthBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;

                D3D12_CLEAR_VALUE clearValue = {};
                clearValue.Format = UtilitiesDirectX12::FormatToDXGIFormat(texture->GetSpecs().p_Format);
                clearValue.Color[0] = 1.0f; clearValue.Color[1] = 0.0f; // Red
                clearValue.Color[2] = 0.0f; clearValue.Color[3] = 1.0f;
                m_DepthStencil.DepthBeginningAccess.Clear.ClearValue = clearValue;

                if (attachment.pStoreOp == StoreOp::Store)
                    m_DepthStencil.DepthEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
                else if (attachment.pStoreOp == StoreOp::DontCare)
                    m_DepthStencil.DepthEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;

                m_DepthStencil.StencilBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS;
                m_DepthStencil.StencilEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS;
            }
            else if (attachment.pAttachmentType == AttachmentType::Swapchain)
            {
                isSwapchain = true;
                for (int i = 0; i < std::static_pointer_cast<SwapchainDirectX12>(Engine::s_Application->GetRenderer()->GetSwapchain())->m_RtvHandles.size(); i++) {
                    D3D12_RENDER_PASS_RENDER_TARGET_DESC rtDesc{};
                    rtDesc.cpuDescriptor = std::static_pointer_cast<SwapchainDirectX12>(Engine::s_Application->GetRenderer()->GetSwapchain())->m_RtvHandles[i];
                    m_RTVHandles.push_back(std::static_pointer_cast<SwapchainDirectX12>(Engine::s_Application->GetRenderer()->GetSwapchain())->m_RtvHandles[i]);

                    if (attachment.pLoadOp == LoadOp::Clear)
                        rtDesc.BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
                    else if (attachment.pLoadOp == LoadOp::Load)
                        rtDesc.BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
                    else if (attachment.pLoadOp == LoadOp::DontCare)
                        rtDesc.BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;

                    D3D12_CLEAR_VALUE clearValue = {};
                    clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                    clearValue.Color[0] = 1.0f; clearValue.Color[1] = 0.0f;
                    clearValue.Color[2] = 0.0f; clearValue.Color[3] = 1.0f;
                    rtDesc.BeginningAccess.Clear.ClearValue = clearValue;

                    if (attachment.pStoreOp == StoreOp::Store)
                        rtDesc.EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
                    else if (attachment.pStoreOp == StoreOp::DontCare)
                        rtDesc.EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;

                    m_RenderTargets.push_back(rtDesc);
                }
            }
        }
    }

    void RenderPassDirectX12::Begin(std::shared_ptr<CommandBuffer> cmd, uint32_t imageIndex) {
        ComPtr<ID3D12GraphicsCommandList6> commandList = std::static_pointer_cast<CommandBufferDirectX12>(cmd)->Get();
        //commandList->BeginRenderPass(
        //    0,
        //    nullptr,
        //    &m_DepthStencil,
        //    D3D12_RENDER_PASS_FLAG_NONE
        //);
        if (!isSwapchain) {
            commandList->OMSetRenderTargets(
                m_RTVHandles.size(),
                m_RTVHandles.size() > 0 ? m_RTVHandles.data() : nullptr,
                FALSE,
                m_DSVHandle.ptr ? &m_DSVHandle : nullptr
            );
        }
        else {
            commandList->OMSetRenderTargets(
                1,
                &m_RTVHandles[imageIndex],
                FALSE,
                nullptr
            );
        }

        if (m_DSVHandle.ptr) {
            commandList->ClearDepthStencilView(
                m_DSVHandle,
                D3D12_CLEAR_FLAG_DEPTH,
                1.0f,
                0,
                0,
                nullptr
            );
        }
        if (m_RTVHandles.size() > 0) {
            if (isSwapchain) {
                FLOAT Color[4];
                Color[0] = 1.0f; Color[1] = 0.0f;
                Color[2] = 0.0f; Color[3] = 1.0f;
                commandList->ClearRenderTargetView(
                    m_RTVHandles[imageIndex],
                    Color,
                    0,
                    nullptr
                );
            }
            else {
                for (int i = 0; i < m_RTVHandles.size(); i++) {
                    FLOAT Color[4];
                    Color[0] = 1.0f; Color[1] = 0.0f;
                    Color[2] = 0.0f; Color[3] = 1.0f;
                    commandList->ClearRenderTargetView(
                        m_RTVHandles[i],
                        Color,
                        0,
                        nullptr
                    );
                }
            }
        }

    }

    void RenderPassDirectX12::End(std::shared_ptr<CommandBuffer> cmd) {
        ComPtr<ID3D12GraphicsCommandList6> commandList = std::static_pointer_cast<CommandBufferDirectX12>(cmd)->Get();
        //commandList->EndRenderPass();
    }
}