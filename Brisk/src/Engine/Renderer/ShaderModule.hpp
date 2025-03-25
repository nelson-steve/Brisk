#pragma once

// INCLUDES
#include "Core/Core.hpp"
//--------------------
#include <utility>
#include <memory>
#include <string>
//---------------

namespace Brisk
{
    class ShaderModule {
        DEFINE_BASE_CLASS_CONSTRUCTOR(ShaderModule)
    public:
        virtual void Init(std::string path, int type) = 0;

        static std::shared_ptr<ShaderModule> Create();
    };
}