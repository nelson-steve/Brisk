#pragma once

#include "Pipeline.hpp"

#include <memory>
#include <string>

namespace Brisk 
{
    class ShaderModule {
    public:
        void Init(std::pair<std::string, Pipeline::ShaderStage> shaderInfo);

        std::shared_ptr<ShaderModule> Create() {return std::make_shared<ShaderModule>();}

    private:
        VkShaderModule m_Module;
		VkPipelineShaderStageCreateInfo m_ShaderStage;
    }
}