#pragma once

// INCLUDES
#include <utility>
#include <memory>
#include <string>
//---------------

namespace Brisk
{
    class ShaderModule {
    public:
        virtual void Init(std::pair<std::string, int> shaderInfo) = 0;

        static std::shared_ptr<ShaderModule> Create();

    };
}