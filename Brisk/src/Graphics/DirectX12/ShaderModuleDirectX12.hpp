#pragma once

#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

#include <string>

namespace Brisk
{
    class ShaderModuleDirectX12 {
    public:
        void Init(std::string path, int type);

        ID3DBlob* GetShaderBlob() { return m_ShaderBlob.Get(); }
        D3D12_SHADER_BYTECODE GetShaderBytecode() { return { m_ShaderBlob->GetBufferPointer(), m_ShaderBlob->GetBufferSize() }; }
    private:
        ComPtr<ID3DBlob> m_ShaderBlob;
    };
}