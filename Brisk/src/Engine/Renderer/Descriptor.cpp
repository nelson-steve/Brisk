#include "Descriptor.hpp"

#include "Graphics/Vulkan/DescriptorVulkan.hpp"

namespace Brisk
{
	std::shared_ptr<Descriptor> Descriptor::Create() {
		return std::make_shared<DescriptorVulkan>();
	}
}