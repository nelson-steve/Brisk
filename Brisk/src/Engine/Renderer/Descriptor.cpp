#include "Descriptor.hpp"

#include "Graphics/Vulkan/DescriptorLayoutVulkan.hpp"

namespace Brisk
{
	std::shared_ptr<DescriptorLayout> DescriptorLayout::Create() {
		return std::make_shared<DescriptorLayoutVulkan>();
	}
}