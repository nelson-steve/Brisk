#pragma once

// INCLUDES
#include "Core/Core.hpp"
#include "Pipeline.hpp"
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
        virtual void Init(std::string path, Pipeline::ShaderStage type) = 0;

        static std::shared_ptr<ShaderModule> Create();

    };
}