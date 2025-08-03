#pragma once

#include "Engine/Renderer/Texture.hpp"

#include <directx/d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

#include <cassert>

namespace Brisk
{
    class TextureDirectX12 : public Texture
    {
    public:
        ID3D12Resource* GetResource() { return m_Texture.Get(); }
        ID3D12DescriptorHeap* GetSRVHeap() { return m_SRVHeap.Get(); }
        D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPU() const { return m_SRVHandleCPU; }
        D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPU() const { return m_SRVHandleGPU; }

        virtual void TransitionImageLayout(std::shared_ptr<CommandBuffer> cmd, std::vector<ImageBarrierParams> params) override;
        virtual void CopyImage(std::shared_ptr<CommandBuffer> cmd, std::shared_ptr<Texture> src, std::shared_ptr<Texture> dest, uint32_t width, uint32_t height) override;

        virtual uint32_t GetWidth() const { return m_Specs.p_Width; }
        virtual uint32_t GetHeight() const { return m_Specs.p_Height; }
        virtual void Resize() override { assert(false); }

        virtual void Init(const TextureSpecification& specs) override;
        virtual void Init(const std::string& path) override;
        virtual void Init(const fastgltf::Image& image, const fastgltf::Asset& asset) override;
        virtual void Release() { assert(false); }

    private:
        ComPtr<ID3D12Resource> m_Texture;
        ComPtr<ID3D12DescriptorHeap> m_SRVHeap;
        D3D12_CPU_DESCRIPTOR_HANDLE m_SRVHandleCPU;
        D3D12_GPU_DESCRIPTOR_HANDLE m_SRVHandleGPU;
    };
}
