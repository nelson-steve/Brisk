#include "ShaderModuleDirectX12.hpp"
#include <stdexcept>
#include <string>
#include <locale>
#include <codecvt>

namespace Brisk
{
    void ShaderModuleDirectX12::Init(std::string path, Pipeline::ShaderStage type) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        const std::wstring& shaderPath = converter.from_bytes(path);
        const std::string& entryPoint = "main";

        UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
        compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
        HRESULT hr = D3DCompileFromFile(
            shaderPath.c_str(),
            nullptr,
            nullptr,
            entryPoint.c_str(),
            "vs_5_1",
            compileFlags,
            0,
            &m_ShaderBlob,
            &errorBlob
        );

        if (FAILED(hr)) {
            if (errorBlob) {
                OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            }
            throw std::runtime_error("Shader compilation failed!");
        }
    }

}