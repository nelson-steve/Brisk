#include "Buffer.hpp"
#include "Graphics/Vulkan/BufferVulkan.hpp"

namespace Brisk 
{
	std::shared_ptr<Buffer> Buffer::Create() {
		return std::make_shared<BufferVulkan>();
	}
}