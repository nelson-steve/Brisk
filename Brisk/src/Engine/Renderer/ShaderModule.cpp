// INCLUDES
#include "ShaderModule.hpp"
#include "Graphics/Vulkan/ShaderModuleVulkan.hpp"
//-----------------------------------------------

namespace Brisk 
{
    std::shared_ptr<ShaderModule> ShaderModule::Create() 
    { 
        return std::make_shared<ShaderModuleVulkan>(); 
    }
}