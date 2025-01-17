#pragma once 

// INCLUDES
#include "Engine/Renderer/Descriptor.hpp"
//---------------------------------------
#include <Volk/volk.h>
//--------------------

namespace Brisk
{
	class DescriptorLayoutVulkan : public DescriptorLayout {
	public:
		virtual void Init() override;
		VkDescriptorSetLayout GetLayout() const { return m_DescriptorLayout; }
	private:
		VkDescriptorSetLayout m_DescriptorLayout;
	};
}