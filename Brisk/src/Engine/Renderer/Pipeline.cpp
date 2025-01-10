// INCLUDES
#include "Pipeline.hpp"
#include "Graphics/Vulkan/PipelineVulkan.hpp"
//------------------------------------------

namespace Brisk 
{
    std::shared_ptr<Pipeline> Pipeline::Create() {
        return std::make_shared<PipelineVulkan>();
    }
}