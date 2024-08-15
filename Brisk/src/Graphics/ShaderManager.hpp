#pragma once

#include "Engine/Engine.hpp"
#include "GPUDevice.hpp"
#include "Core/Log.hpp"

#include <Volk/volk.h>

#include <vector>
#include <fstream>

namespace Brisk {
    struct ShaderModule {
        ShaderType Type;
        VkShaderModule Module;
    };

	class ShaderManager {
    public:
        static const ShaderModule& CreateShaderModule(const ShaderInfo& info);
        static const void ReleaseShaderModule(const std::vector<ShaderModule>& modules);
    private:
        static std::vector<char>& ReadShaderFile(const std::string& fileName);
    public:
        static std::vector<char>* m_ShaderFileBuffer;

	};
}