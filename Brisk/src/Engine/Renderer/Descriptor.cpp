// INCLUDES
#include "Descriptor.hpp"
#include "Graphics/Vulkan/DescriptorLayoutVulkan.hpp"
//---------------------------------------------------

namespace Brisk
{
	std::shared_ptr<DescriptorLayout> DescriptorLayout::Create() {
		return std::make_shared<DescriptorLayoutVulkan>();
	}

	void DescriptorLayout::SetDescriptorType(GpuDescriptorResourceType type) {
		m_Type = type;
	}

	GpuDescriptorResourceType DescriptorLayout::GetDescriptorType() {
		return m_Type;
	}

}