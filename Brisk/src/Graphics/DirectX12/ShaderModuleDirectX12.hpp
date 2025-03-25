#pragma once

#include "Engine/Renderer/ShaderModule.hpp"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

namespace Brisk
{
    class ShaderModuleDirectX12 : public ShaderModule {
    public:
        virtual void Init(std::string path, int type) override;

        ID3DBlob* GetShaderBlob() { return m_ShaderBlob.Get(); }
        D3D12_SHADER_BYTECODE GetShaderBytecode() { return { m_ShaderBlob->GetBufferPointer(), m_ShaderBlob->GetBufferSize() }; }
    private:
        ComPtr<ID3DBlob> m_ShaderBlob;
    };
}