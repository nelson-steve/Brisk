// INCLUDES
#include "Semaphore.hpp"
#include "Graphics/Vulkan/SemaphoreVulkan.hpp"
//-------------------------------------------

namespace Brisk 
{
    std::shared_ptr<Semaphore> Semaphore::Create(){
        return std::make_shared<SemaphoreVulkan>();
    }
}