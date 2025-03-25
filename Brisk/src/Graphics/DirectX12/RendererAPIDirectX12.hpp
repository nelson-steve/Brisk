#pragma once

// INCLUDES
#include "CommandBufferDirectX12.hpp"
#include "BufferDirectX12.hpp"
//---------------------------------------------------------------
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
//--------------------
using Microsoft::WRL::ComPtr;


namespace Brisk
{
    class RendererAPIDirectX12 : public RendererAPI {
    public:
        virtual void SetViewport(std::shared_ptr<CommandBuffer> cmd, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t minDepth, uint32_t maxDepth) override {
            D3D12_VIEWPORT viewport{};
            viewport.TopLeftX = static_cast<float>(x);
            viewport.TopLeftY = static_cast<float>(y);
            viewport.Width = static_cast<float>(width);
            viewport.Height = static_cast<float>(height);
            viewport.MinDepth = static_cast<float>(minDepth);
            viewport.MaxDepth = static_cast<float>(maxDepth);

            std::static_pointer_cast<CommandBufferDirectX12>(cmd)->Get()->RSSetViewports(1, &viewport);
        }

        virtual void SetScissor(std::shared_ptr<CommandBuffer> cmd, uint32_t offsetX, uint32_t offsetY, uint32_t extentX, uint32_t extentY) override {
            D3D12_RECT scissorRect{};
            scissorRect.left = static_cast<LONG>(offsetX);
            scissorRect.top = static_cast<LONG>(offsetY);
            scissorRect.right = static_cast<LONG>(offsetX + extentX);
            scissorRect.bottom = static_cast<LONG>(offsetY + extentY);

            std::static_pointer_cast<CommandBufferDirectX12>(cmd)->Get()->RSSetScissorRects(1, &scissorRect);
        }

        virtual void Draw(std::shared_ptr<CommandBuffer> cmd, uint32_t vertexCount, uint32_t firstVertex) override {
            std::static_pointer_cast<CommandBufferDirectX12>(cmd)->Get()->DrawInstanced(vertexCount, 1, firstVertex, 0);
        }

        virtual void DrawIndexed(std::shared_ptr<CommandBuffer> cmd, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) override {
            std::static_pointer_cast<CommandBufferDirectX12>(cmd)->Get()->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
        }

        virtual void BindIndexBuffer(std::shared_ptr<CommandBuffer> cmd, std::shared_ptr<Buffer> buffer, uint32_t firstBinding) override {
            auto dxBuffer = std::static_pointer_cast<BufferDirectX12>(buffer);
            D3D12_INDEX_BUFFER_VIEW indexBufferView{};
            //indexBufferView.BufferLocation = dxBuffer->GetGPUVirtualAddress();
            //indexBufferView.SizeInBytes = dxBuffer->GetSize();
            indexBufferView.Format = DXGI_FORMAT_R32_UINT;

            std::static_pointer_cast<CommandBufferDirectX12>(cmd)->Get()->IASetIndexBuffer(&indexBufferView);
        }

        virtual void BindVertexBuffer(std::shared_ptr<CommandBuffer> cmd, std::vector<std::shared_ptr<Buffer>> buffers, uint32_t firstBinding) override {
            std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferViews;
            for (const auto& buffer : buffers) {
                auto dxBuffer = std::static_pointer_cast<BufferDirectX12>(buffer);
                D3D12_VERTEX_BUFFER_VIEW vbv{};
                //vbv.BufferLocation = dxBuffer->GetGPUVirtualAddress();
                //vbv.SizeInBytes = dxBuffer->GetSize();
                //vbv.StrideInBytes = dxBuffer->GetStride();
                vertexBufferViews.push_back(vbv);
            }

            std::static_pointer_cast<CommandBufferDirectX12>(cmd)->Get()->IASetVertexBuffers(firstBinding, static_cast<UINT>(vertexBufferViews.size()), vertexBufferViews.data());
        }
    };
}