#include "ShaderModuleDirectX12.hpp"
#include <stdexcept>
#include <string>

namespace Brisk
{
    std::wstring StringToWString(const std::string& str)
    {
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), NULL, 0);
        std::wstring wstr(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), &wstr[0], size_needed);
        return wstr;
    }

    void ShaderModuleDirectX12::Init(std::string path, int type) {
        const std::wstring& shaderPath = StringToWString(path);
        const std::string& entryPoint = "main";

        UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
        compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
        //HRESULT hr = D3DCompileFromFile(
        //    shaderPath.c_str(),
        //    nullptr,
        //    nullptr,
        //    entryPoint.c_str(),
        //    "vs_5_1",
        //    compileFlags,
        //    0,
        //    &m_ShaderBlob,
        //    &errorBlob
        //);

        //if (FAILED(hr)) {
        //    if (errorBlob) {
        //        OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        //    }
        //    throw std::runtime_error("Shader compilation failed!");
        //}
    }
}