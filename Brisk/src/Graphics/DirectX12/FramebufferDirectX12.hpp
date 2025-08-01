#pragma once

//INCLUDES
#include "Engine/Renderer/Framebuffer.hpp"
//-----------------
#include <directx/d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
//--------------------
using Microsoft::WRL::ComPtr;

namespace Brisk
{
    class FramebufferDirectX12 : public Framebuffer {
    public:
        virtual void Init(const FramebufferSpecs& specs) override;
        virtual void Destroy() override;

        virtual void Bind() override;

    private:
        Microsoft::WRL::ComPtr<ID3D12Resource> m_RenderTarget;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RTVHeap;
        D3D12_CPU_DESCRIPTOR_HANDLE m_RTVHandle;
    };
}
