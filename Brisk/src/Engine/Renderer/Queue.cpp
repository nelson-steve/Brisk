// INCLUDES
#include "Queue.hpp"
#include "Graphics/Vulkan/QueueVulkan.hpp"
//---------------------------------------

namespace Brisk
{
	std::shared_ptr<Queue> Queue::Create() {
		return std::make_shared<QueueVulkan>();
	}
}