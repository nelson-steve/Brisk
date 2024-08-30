#pragma once

#include "Engine/Engine.hpp"
#include "Core/Log.hpp"

#include <Volk/volk.h>

#include <vector>
#include <fstream>
#include <string>

namespace Brisk {
	class ShaderManager {
    public:
        static const VkShaderModule CreateShaderModule(const std::string& path);
    private:
        static std::vector<char>* ReadShaderFile(const std::string& fileName);
	};
}