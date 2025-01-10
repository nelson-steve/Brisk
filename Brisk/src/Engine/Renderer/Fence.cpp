// INCLUDES
#include "Fence.hpp"
#include <Graphics/Vulkan/FenceVulkan.hpp>
//----------------------------------------

namespace Brisk 
{
    std::shared_ptr<Fence> Fence::Create(){
        return std::make_shared<FenceVulkan>();
    }
}