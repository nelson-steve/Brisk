// INCLUDES
#include "Graphics/Vulkan/CommandBufferVulkan.hpp"
//-----------------------------------------------

namespace Brisk {
	std::shared_ptr<CommandBuffer> CommandBuffer::Create() {
		return std::make_shared<CommandBufferVulkan>();
	}
}